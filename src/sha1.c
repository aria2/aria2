/* This code is public-domain - it is based on libcrypt 
 * placed in the public domain by Wei Dai and other contributors.
 */

#include "sha1.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_LENGTH 64

union _buffer {
  uint8_t b[BLOCK_LENGTH];
  uint32_t w[BLOCK_LENGTH/4];
};

union _state {
  uint8_t b[SHA1_LENGTH];
  uint32_t w[SHA1_LENGTH/4];
};

struct SHA1_CTX {
  union _buffer buffer;
  uint8_t bufferOffset;
  union _state state;
  uint32_t byteCount;
  uint8_t keyBuffer[BLOCK_LENGTH];
  uint8_t innerHash[SHA1_LENGTH];
};

#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

const uint8_t sha1InitState[] = {
  0x01,0x23,0x45,0x67, // H0
  0x89,0xab,0xcd,0xef, // H1
  0xfe,0xdc,0xba,0x98, // H2
  0x76,0x54,0x32,0x10, // H3
  0xf0,0xe1,0xd2,0xc3  // H4
};

static uint32_t sha1_rol32(uint32_t number, uint8_t bits) {
  return ((number << bits) | (number >> (32-bits)));
}

static void sha1_hashBlock(struct SHA1_CTX *s) {
  uint8_t i;
  uint32_t a,b,c,d,e,t;

  a=s->state.w[0];
  b=s->state.w[1];
  c=s->state.w[2];
  d=s->state.w[3];
  e=s->state.w[4];
  for (i=0; i<80; i++) {
    if (i>=16) {
      t = s->buffer.w[(i+13)&15] ^ s->buffer.w[(i+8)&15] ^ s->buffer.w[(i+2)&15] ^ s->buffer.w[i&15];
      s->buffer.w[i&15] = sha1_rol32(t,1);
    }
    if (i<20) {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    } else if (i<40) {
      t = (b ^ c ^ d) + SHA1_K20;
    } else if (i<60) {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    } else {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t+=sha1_rol32(a,5) + e + s->buffer.w[i&15];
    e=d;
    d=c;
    c=sha1_rol32(b,30);
    b=a;
    a=t;
  }
  s->state.w[0] += a;
  s->state.w[1] += b;
  s->state.w[2] += c;
  s->state.w[3] += d;
  s->state.w[4] += e;
}

static void sha1_addUncounted(struct SHA1_CTX *s, uint8_t data) {
  s->buffer.b[s->bufferOffset ^ 3] = data;
  s->bufferOffset++;
  if (s->bufferOffset == BLOCK_LENGTH) {
    sha1_hashBlock(s);
    s->bufferOffset = 0;
  }
}

static void sha1_writebyte(struct SHA1_CTX *s, uint8_t data) {
  ++s->byteCount;
  sha1_addUncounted(s, data);
}

static void sha1_pad(struct SHA1_CTX *s) {
  // Implement SHA-1 padding (fips180-2 §5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  sha1_addUncounted(s, 0x80);
  while (s->bufferOffset != 56) sha1_addUncounted(s, 0x00);

  // Append length in the last 8 bytes
  sha1_addUncounted(s, 0); // We're only using 32 bit lengths
  sha1_addUncounted(s, 0); // But SHA-1 supports 64 bit lengths
  sha1_addUncounted(s, 0); // So zero pad the top bits
  sha1_addUncounted(s, s->byteCount >> 29); // Shifting to multiply by 8
  sha1_addUncounted(s, s->byteCount >> 21); // as SHA-1 supports bitstreams as well as
  sha1_addUncounted(s, s->byteCount >> 13); // byte.
  sha1_addUncounted(s, s->byteCount >> 5);
  sha1_addUncounted(s, s->byteCount << 3);
}

int SHA1_Init(struct SHA1_CTX **s) {
  *s = malloc(sizeof(struct SHA1_CTX));
  if (!*s) {
    return 0;
  }
  memcpy((*s)->state.b, sha1InitState, SHA1_LENGTH);
  (*s)->byteCount = 0;
  (*s)->bufferOffset = 0;
  return 1;
}

void SHA1_Update(struct SHA1_CTX *s, const void *data, size_t len) {
  const uint8_t *bytes = data;
  for (;len--;) sha1_writebyte(s, *bytes++);
}

void SHA1_Final(struct SHA1_CTX *s, unsigned char* result) {
  int i;
  // Pad to complete the last block
  sha1_pad(s);

  // Swap byte order back
  for (i=0; i<5; i++) {
    uint32_t a,b;
    a=s->state.w[i];
    b=a<<24;
    b|=(a<<8) & 0x00ff0000;
    b|=(a>>8) & 0x0000ff00;
    b|=a>>24;
    s->state.w[i]=b;
  }

  // Return pointer to hash (20 characters)
  memcpy(result, s->state.b, sizeof(s->state.b));
}

void SHA1_Free(struct SHA1_CTX** s) {
  if (!s || !*s) {
    return;
  }
  free(*s);
  *s = NULL;
}
