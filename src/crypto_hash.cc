/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */
// Written in 2014 by Nils Maier

#include "crypto_hash.h"
#include "crypto_endian.h"
#include "a2functional.h"

#include <cstring>
#include <stdexcept>
#include <unordered_map>

// Compiler hints
#if defined(__GNUG__)
#  define likely(x) __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else // ! __GNUG_
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif // ! __GNUG__

// Basic operations
static forceinline uint32_t rol(uint32_t x, uint32_t n)
{
  return x << n | x >> (32 - n);
}

static forceinline uint32_t ror(uint32_t x, uint32_t n)
{
  return x >> n | x << (32 - n);
}

static forceinline uint64_t ror(uint64_t x, uint64_t n)
{
  return x >> n | x << (64 - n);
}

// SHA functions
template <typename T> static forceinline T ch(T b, T c, T d)
{
  return d ^ (b & (c ^ d));
}

template <typename T> static forceinline T maj(T b, T c, T d)
{
  return (b & c) | (d & (b | c));
}

template <typename T> static forceinline T par(T b, T c, T d)
{
  return b ^ c ^ d;
}

#ifdef __GNUG__
#  define __hash_maybe_memfence __asm__("" ::: "memory")
#else // __GNUG__
#  define __hash_maybe_memfence
#endif // __GNUG__

// Template for the |::transform|s
#define __hash_assign_words(endian)                                            \
  word_t w00, w01, w02, w03, w04, w05, w06, w07, w08, w09, w10, w11, w12, w13, \
      w14, w15;                                                                \
  w15 = endian(buffer[15]), w14 = endian(buffer[14]);                          \
  w13 = endian(buffer[13]), w12 = endian(buffer[12]);                          \
  w11 = endian(buffer[11]), w10 = endian(buffer[10]);                          \
  w09 = endian(buffer[9]), w08 = endian(buffer[8]);                            \
  w07 = endian(buffer[7]), w06 = endian(buffer[6]);                            \
  w05 = endian(buffer[5]), w04 = endian(buffer[4]);                            \
  w03 = endian(buffer[3]), w02 = endian(buffer[2]);                            \
  w01 = endian(buffer[1]), w00 = endian(buffer[0])

using namespace crypto;
using namespace crypto::hash;

// Our base implementation, doing most of the work, short of |transform|,
// |digest| and initialization.
template <typename word_, uint_fast8_t bsize, uint_fast8_t ssize>
class AlgorithmImpl : public Algorithm {
public:
  typedef word_ word_t;

protected:
  template <uint_fast8_t size> union buffer_t {
    uint8_t bytes[size * sizeof(word_t)];
    word_t words[size];
  };

  uint_fast64_t count_;
  buffer_t<bsize> buffer_;
  buffer_t<ssize> state_;
  uint_fast8_t offset_;

  virtual void transform(const word_t* buffer) = 0;

  virtual std::string digest()
  {
    return std::string((const char*)state_.bytes, sizeof(state_.bytes));
  }

public:
  AlgorithmImpl() = default;

  virtual void update(const void* data, uint64_t len)
  {
    if (unlikely(!len)) {
      return;
    }

    auto bytes = reinterpret_cast<const uint8_t*>(data);
    count_ += len;

    // We have data buffered...
    if (unlikely(offset_)) {
      const uint32_t rem = sizeof(buffer_) - offset_;
      const uint32_t turn = (uint32_t)(len + ((rem - len) & (rem - len) >> 31));
      memcpy(buffer_.bytes + offset_, bytes, turn);
      len -= turn;
      bytes += turn;
      offset_ += turn;
      if (likely(offset_ == sizeof(buffer_))) {
        transform(buffer_.words);
        offset_ = 0;
      }
    }

    // |transform| as many blocks as possible.
    while (len >= sizeof(buffer_)) {
      // |offset_| has to be 0 at this point!
      // Which is guaranteed by the block above.

      transform(reinterpret_cast<const word_t*>(bytes));
      bytes += sizeof(buffer_);
      len -= sizeof(buffer_);
    }

    // Buffer remaining bytes, if any.
    if (unlikely(len)) {
      const uint32_t rem = sizeof(buffer_) - offset_;
      const uint32_t turn = (uint32_t)(len + ((rem - len) & (rem - len) >> 31));
      memcpy(buffer_.bytes + offset_, bytes, turn);
      offset_ += turn;
    }
  }

  virtual std::string finalize()
  {
    // Pad the message according to spec.
    const uint_fast16_t cutoff = sizeof(buffer_) - sizeof(word_t) * 2;
    buffer_.bytes[offset_] = 0x80;
    if (unlikely(++offset_ == sizeof(buffer_))) {
      transform(buffer_.words);
      memset(buffer_.bytes, 0x00, cutoff);
    }
    else if (offset_ > cutoff) {
      memset(buffer_.bytes + offset_, 0x00, sizeof(buffer_) - offset_);
      transform(buffer_.words);
      memset(buffer_.bytes, 0x00, cutoff);
    }
    else if (likely(offset_ != cutoff)) {
      memset(buffer_.bytes + offset_, 0x00, cutoff - offset_);
    }

    // Append length, multiplied by 8 (because bits!)
    const uint_fast64_t bits = __crypto_be(count_ << 3);
    if (sizeof(word_t) == 4) {
#if LITTLE_ENDIAN == BYTE_ORDER
      buffer_.words[bsize - 2] = bits;
      buffer_.words[bsize - 1] = bits >> 32;
#else  // LITTLE_ENDIAN != BYTE_ORDER
      buffer_.words[bsize - 2] = bits >> 32;
      buffer_.words[bsize - 1] = bits;
#endif // LITTLE_ENDIAN != BYTE_ORDER
    }
    else {
      buffer_.words[bsize - 2] = 0;
      buffer_.words[bsize - 1] = bits;
    }

    // Last transform:
    transform(buffer_.words);

#if LITTLE_ENDIAN == BYTE_ORDER
    // On little endian, we still need to swap the bytes.
    for (auto i = 0; i < ssize; ++i) {
      state_.words[i] = __crypto_bswap(state_.words[i]);
    }
#endif // LITTLE_ENDIAN == BYTE_ORDER

    auto rv = digest();
    reset();
    return rv;
  }

  virtual uint_fast16_t blocksize() const { return sizeof(buffer_); }
};

// Important! Other than the SHA family, MD5 is actually LE.
class MD5 : public AlgorithmImpl<uint32_t, 16, 4> {
private:
  static const word_t initvec[];

protected:
  virtual void transform(const word_t* buffer)
  {
    __hash_assign_words(__crypto_le);
    __hash_maybe_memfence;

    word_t a, b, c, d;
    a = state_.words[0];
    b = state_.words[1];
    c = state_.words[2];
    d = state_.words[3];
    __hash_maybe_memfence;

#define f1(x, y, z) (z ^ (x & (y ^ z)))
#define f2(x, y, z) (y ^ (z & (x ^ y)))
#define f3(x, y, z) (x ^ y ^ z)
#define f4(x, y, z) (y ^ (x | ~z))

#define r(f, a1, a2, a3, a4, w, k, n)                                          \
  a1 += f(a2, a3, a4) + w + k;                                                 \
  a1 = rol(a1, n);                                                             \
  a1 += a2;

    // Yeah, unrolled loops.
    // This stuff is generated, but accidentally deleted the generator script.

    r(f1, a, b, c, d, w00, 0xd76aa478, 7);
    r(f1, d, a, b, c, w01, 0xe8c7b756, 12);
    r(f1, c, d, a, b, w02, 0x242070db, 17);
    r(f1, b, c, d, a, w03, 0xc1bdceee, 22);
    r(f1, a, b, c, d, w04, 0xf57c0faf, 7);
    r(f1, d, a, b, c, w05, 0x4787c62a, 12);
    r(f1, c, d, a, b, w06, 0xa8304613, 17);
    r(f1, b, c, d, a, w07, 0xfd469501, 22);
    r(f1, a, b, c, d, w08, 0x698098d8, 7);
    r(f1, d, a, b, c, w09, 0x8b44f7af, 12);
    r(f1, c, d, a, b, w10, 0xffff5bb1, 17);
    r(f1, b, c, d, a, w11, 0x895cd7be, 22);
    r(f1, a, b, c, d, w12, 0x6b901122, 7);
    r(f1, d, a, b, c, w13, 0xfd987193, 12);
    r(f1, c, d, a, b, w14, 0xa679438e, 17);
    r(f1, b, c, d, a, w15, 0x49b40821, 22);

    r(f2, a, b, c, d, w01, 0xf61e2562, 5);
    r(f2, d, a, b, c, w06, 0xc040b340, 9);
    r(f2, c, d, a, b, w11, 0x265e5a51, 14);
    r(f2, b, c, d, a, w00, 0xe9b6c7aa, 20);
    r(f2, a, b, c, d, w05, 0xd62f105d, 5);
    r(f2, d, a, b, c, w10, 0x02441453, 9);
    r(f2, c, d, a, b, w15, 0xd8a1e681, 14);
    r(f2, b, c, d, a, w04, 0xe7d3fbc8, 20);
    r(f2, a, b, c, d, w09, 0x21e1cde6, 5);
    r(f2, d, a, b, c, w14, 0xc33707d6, 9);
    r(f2, c, d, a, b, w03, 0xf4d50d87, 14);
    r(f2, b, c, d, a, w08, 0x455a14ed, 20);
    r(f2, a, b, c, d, w13, 0xa9e3e905, 5);
    r(f2, d, a, b, c, w02, 0xfcefa3f8, 9);
    r(f2, c, d, a, b, w07, 0x676f02d9, 14);
    r(f2, b, c, d, a, w12, 0x8d2a4c8a, 20);

    r(f3, a, b, c, d, w05, 0xfffa3942, 4);
    r(f3, d, a, b, c, w08, 0x8771f681, 11);
    r(f3, c, d, a, b, w11, 0x6d9d6122, 16);
    r(f3, b, c, d, a, w14, 0xfde5380c, 23);
    r(f3, a, b, c, d, w01, 0xa4beea44, 4);
    r(f3, d, a, b, c, w04, 0x4bdecfa9, 11);
    r(f3, c, d, a, b, w07, 0xf6bb4b60, 16);
    r(f3, b, c, d, a, w10, 0xbebfbc70, 23);
    r(f3, a, b, c, d, w13, 0x289b7ec6, 4);
    r(f3, d, a, b, c, w00, 0xeaa127fa, 11);
    r(f3, c, d, a, b, w03, 0xd4ef3085, 16);
    r(f3, b, c, d, a, w06, 0x04881d05, 23);
    r(f3, a, b, c, d, w09, 0xd9d4d039, 4);
    r(f3, d, a, b, c, w12, 0xe6db99e5, 11);
    r(f3, c, d, a, b, w15, 0x1fa27cf8, 16);
    r(f3, b, c, d, a, w02, 0xc4ac5665, 23);

    r(f4, a, b, c, d, w00, 0xf4292244, 6);
    r(f4, d, a, b, c, w07, 0x432aff97, 10);
    r(f4, c, d, a, b, w14, 0xab9423a7, 15);
    r(f4, b, c, d, a, w05, 0xfc93a039, 21);
    r(f4, a, b, c, d, w12, 0x655b59c3, 6);
    r(f4, d, a, b, c, w03, 0x8f0ccc92, 10);
    r(f4, c, d, a, b, w10, 0xffeff47d, 15);
    r(f4, b, c, d, a, w01, 0x85845dd1, 21);
    r(f4, a, b, c, d, w08, 0x6fa87e4f, 6);
    r(f4, d, a, b, c, w15, 0xfe2ce6e0, 10);
    r(f4, c, d, a, b, w06, 0xa3014314, 15);
    r(f4, b, c, d, a, w13, 0x4e0811a1, 21);
    r(f4, a, b, c, d, w04, 0xf7537e82, 6);
    r(f4, d, a, b, c, w11, 0xbd3af235, 10);
    r(f4, c, d, a, b, w02, 0x2ad7d2bb, 15);
    r(f4, b, c, d, a, w09, 0xeb86d391, 21);

#undef r
#undef f4
#undef f3
#undef f2
#undef f1

    __hash_maybe_memfence;
    state_.words[0] += a;
    state_.words[1] += b;
    state_.words[2] += c;
    state_.words[3] += d;
  }

public:
  MD5() { reset(); }

  virtual ~MD5() { reset(); }

  // Since this is LE, and I am lazy, copy-paste overwriting with BE -> LE.
  virtual std::string finalize()
  {
    // Pad
    buffer_.bytes[offset_] = 0x80;
    if (unlikely(++offset_ == 64)) {
      transform(buffer_.words);
      memset(buffer_.bytes, 0x00, 56);
    }
    else if (offset_ > 56) {
      memset(buffer_.bytes + offset_, 0x00, 64 - offset_);
      transform(buffer_.words);
      memset(buffer_.bytes, 0x00, 56);
    }
    else if (likely(offset_ != 56)) {
      memset(buffer_.bytes + offset_, 0x00, 56 - offset_);
    }

    // Append length, multiplied by 8 (because bits!)
    const uint_fast64_t bits = __crypto_le(count_ << 3);
#if LITTLE_ENDIAN == BYTE_ORDER
    buffer_.words[14] = bits;
    buffer_.words[15] = bits >> 32;
#else  // LITTLE_ENDIAN != BYTE_ORDER
    buffer_.words[14] = bits >> 32;
    buffer_.words[15] = bits;
#endif // LITTLE_ENDIAN != BYTE_ORDER
    transform(buffer_.words);

#if BIG_ENDIAN == BYTE_ORDER
    state_.words[0] = __crypto_bswap(state_.words[0]);
    state_.words[1] = __crypto_bswap(state_.words[1]);
    state_.words[2] = __crypto_bswap(state_.words[2]);
    state_.words[3] = __crypto_bswap(state_.words[3]);
#endif // LITTLE_ENDIAN == BYTE_ORDER

    auto rv = std::string((const char*)state_.bytes, sizeof(state_.bytes));
    reset();
    return rv;
  }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 16; }
};

const MD5::word_t MD5::initvec[] = {0x67452301, 0xefcdab89, 0x98badcfe,
                                    0x10325476};

class SHA1 : public AlgorithmImpl<uint32_t, 16, 5> {
private:
  static const word_t initvec[];

protected:
  virtual void transform(const word_t* buffer)
  {
    __hash_assign_words(__crypto_be);
    __hash_maybe_memfence;

    word_t a, b, c, d, e;
    a = state_.words[0];
    b = state_.words[1];
    c = state_.words[2];
    d = state_.words[3];
    e = state_.words[4];
    __hash_maybe_memfence;

#define c1 0x5a827999UL
#define c2 0x6ed9eba1UL
#define c3 0x8f1bbcdcUL
#define c4 0xca62c1d6UL

#define r(f, k, a1, a2, a3, a4, a5, w)                                         \
  a1 = rol(a2, 5) + f(a3, a4, a5) + a1 + w + k;                                \
  a3 = rol(a3, 30);

#define r1(a1, a2, a3, a4, a5, w) r(ch, c1, a1, a2, a3, a4, a5, w)
#define r2(a1, a2, a3, a4, a5, w) r(par, c2, a1, a2, a3, a4, a5, w)
#define r3(a1, a2, a3, a4, a5, w) r(maj, c3, a1, a2, a3, a4, a5, w)
#define r4(a1, a2, a3, a4, a5, w) r(par, c4, a1, a2, a3, a4, a5, w)

    // Generated code. See sha1-transgen.py
    // Round 0
    r1(e, a, b, c, d, w00);
    r1(d, e, a, b, c, w01);
    r1(c, d, e, a, b, w02);
    r1(b, c, d, e, a, w03);
    r1(a, b, c, d, e, w04);
    r1(e, a, b, c, d, w05);
    r1(d, e, a, b, c, w06);
    r1(c, d, e, a, b, w07);
    r1(b, c, d, e, a, w08);
    r1(a, b, c, d, e, w09);

    // Round 10
    r1(e, a, b, c, d, w10);
    r1(d, e, a, b, c, w11);
    r1(c, d, e, a, b, w12);
    r1(b, c, d, e, a, w13);
    r1(a, b, c, d, e, w14);
    r1(e, a, b, c, d, w15);
    w00 = rol((w13 ^ w08 ^ w02 ^ w00), 1), r1(d, e, a, b, c, w00);
    w01 = rol((w14 ^ w09 ^ w03 ^ w01), 1), r1(c, d, e, a, b, w01);
    w02 = rol((w15 ^ w10 ^ w04 ^ w02), 1), r1(b, c, d, e, a, w02);
    w03 = rol((w00 ^ w11 ^ w05 ^ w03), 1), r1(a, b, c, d, e, w03);

    // Round 20
    w04 = rol((w01 ^ w12 ^ w06 ^ w04), 1), r2(e, a, b, c, d, w04);
    w05 = rol((w02 ^ w13 ^ w07 ^ w05), 1), r2(d, e, a, b, c, w05);
    w06 = rol((w03 ^ w14 ^ w08 ^ w06), 1), r2(c, d, e, a, b, w06);
    w07 = rol((w04 ^ w15 ^ w09 ^ w07), 1), r2(b, c, d, e, a, w07);
    w08 = rol((w05 ^ w00 ^ w10 ^ w08), 1), r2(a, b, c, d, e, w08);
    w09 = rol((w06 ^ w01 ^ w11 ^ w09), 1), r2(e, a, b, c, d, w09);
    w10 = rol((w07 ^ w02 ^ w12 ^ w10), 1), r2(d, e, a, b, c, w10);
    w11 = rol((w08 ^ w03 ^ w13 ^ w11), 1), r2(c, d, e, a, b, w11);
    w12 = rol((w09 ^ w04 ^ w14 ^ w12), 1), r2(b, c, d, e, a, w12);
    w13 = rol((w10 ^ w05 ^ w15 ^ w13), 1), r2(a, b, c, d, e, w13);

    // Round 30
    w14 = rol((w11 ^ w06 ^ w00 ^ w14), 1), r2(e, a, b, c, d, w14);
    w15 = rol((w12 ^ w07 ^ w01 ^ w15), 1), r2(d, e, a, b, c, w15);
    w00 = rol((w13 ^ w08 ^ w02 ^ w00), 1), r2(c, d, e, a, b, w00);
    w01 = rol((w14 ^ w09 ^ w03 ^ w01), 1), r2(b, c, d, e, a, w01);
    w02 = rol((w15 ^ w10 ^ w04 ^ w02), 1), r2(a, b, c, d, e, w02);
    w03 = rol((w00 ^ w11 ^ w05 ^ w03), 1), r2(e, a, b, c, d, w03);
    w04 = rol((w01 ^ w12 ^ w06 ^ w04), 1), r2(d, e, a, b, c, w04);
    w05 = rol((w02 ^ w13 ^ w07 ^ w05), 1), r2(c, d, e, a, b, w05);
    w06 = rol((w03 ^ w14 ^ w08 ^ w06), 1), r2(b, c, d, e, a, w06);
    w07 = rol((w04 ^ w15 ^ w09 ^ w07), 1), r2(a, b, c, d, e, w07);

    // Round 40
    w08 = rol((w05 ^ w00 ^ w10 ^ w08), 1), r3(e, a, b, c, d, w08);
    w09 = rol((w06 ^ w01 ^ w11 ^ w09), 1), r3(d, e, a, b, c, w09);
    w10 = rol((w07 ^ w02 ^ w12 ^ w10), 1), r3(c, d, e, a, b, w10);
    w11 = rol((w08 ^ w03 ^ w13 ^ w11), 1), r3(b, c, d, e, a, w11);
    w12 = rol((w09 ^ w04 ^ w14 ^ w12), 1), r3(a, b, c, d, e, w12);
    w13 = rol((w10 ^ w05 ^ w15 ^ w13), 1), r3(e, a, b, c, d, w13);
    w14 = rol((w11 ^ w06 ^ w00 ^ w14), 1), r3(d, e, a, b, c, w14);
    w15 = rol((w12 ^ w07 ^ w01 ^ w15), 1), r3(c, d, e, a, b, w15);
    w00 = rol((w13 ^ w08 ^ w02 ^ w00), 1), r3(b, c, d, e, a, w00);
    w01 = rol((w14 ^ w09 ^ w03 ^ w01), 1), r3(a, b, c, d, e, w01);

    // Round 50
    w02 = rol((w15 ^ w10 ^ w04 ^ w02), 1), r3(e, a, b, c, d, w02);
    w03 = rol((w00 ^ w11 ^ w05 ^ w03), 1), r3(d, e, a, b, c, w03);
    w04 = rol((w01 ^ w12 ^ w06 ^ w04), 1), r3(c, d, e, a, b, w04);
    w05 = rol((w02 ^ w13 ^ w07 ^ w05), 1), r3(b, c, d, e, a, w05);
    w06 = rol((w03 ^ w14 ^ w08 ^ w06), 1), r3(a, b, c, d, e, w06);
    w07 = rol((w04 ^ w15 ^ w09 ^ w07), 1), r3(e, a, b, c, d, w07);
    w08 = rol((w05 ^ w00 ^ w10 ^ w08), 1), r3(d, e, a, b, c, w08);
    w09 = rol((w06 ^ w01 ^ w11 ^ w09), 1), r3(c, d, e, a, b, w09);
    w10 = rol((w07 ^ w02 ^ w12 ^ w10), 1), r3(b, c, d, e, a, w10);
    w11 = rol((w08 ^ w03 ^ w13 ^ w11), 1), r3(a, b, c, d, e, w11);

    // Round 60
    w12 = rol((w09 ^ w04 ^ w14 ^ w12), 1), r4(e, a, b, c, d, w12);
    w13 = rol((w10 ^ w05 ^ w15 ^ w13), 1), r4(d, e, a, b, c, w13);
    w14 = rol((w11 ^ w06 ^ w00 ^ w14), 1), r4(c, d, e, a, b, w14);
    w15 = rol((w12 ^ w07 ^ w01 ^ w15), 1), r4(b, c, d, e, a, w15);
    w00 = rol((w13 ^ w08 ^ w02 ^ w00), 1), r4(a, b, c, d, e, w00);
    w01 = rol((w14 ^ w09 ^ w03 ^ w01), 1), r4(e, a, b, c, d, w01);
    w02 = rol((w15 ^ w10 ^ w04 ^ w02), 1), r4(d, e, a, b, c, w02);
    w03 = rol((w00 ^ w11 ^ w05 ^ w03), 1), r4(c, d, e, a, b, w03);
    w04 = rol((w01 ^ w12 ^ w06 ^ w04), 1), r4(b, c, d, e, a, w04);
    w05 = rol((w02 ^ w13 ^ w07 ^ w05), 1), r4(a, b, c, d, e, w05);

    // Round 70
    w06 = rol((w03 ^ w14 ^ w08 ^ w06), 1), r4(e, a, b, c, d, w06);
    w07 = rol((w04 ^ w15 ^ w09 ^ w07), 1), r4(d, e, a, b, c, w07);
    w08 = rol((w05 ^ w00 ^ w10 ^ w08), 1), r4(c, d, e, a, b, w08);
    w09 = rol((w06 ^ w01 ^ w11 ^ w09), 1), r4(b, c, d, e, a, w09);
    w10 = rol((w07 ^ w02 ^ w12 ^ w10), 1), r4(a, b, c, d, e, w10);
    w11 = rol((w08 ^ w03 ^ w13 ^ w11), 1), r4(e, a, b, c, d, w11);
    w12 = rol((w09 ^ w04 ^ w14 ^ w12), 1), r4(d, e, a, b, c, w12);
    w13 = rol((w10 ^ w05 ^ w15 ^ w13), 1), r4(c, d, e, a, b, w13);
    w14 = rol((w11 ^ w06 ^ w00 ^ w14), 1), r4(b, c, d, e, a, w14);
    w15 = rol((w12 ^ w07 ^ w01 ^ w15), 1), r4(a, b, c, d, e, w15);

#undef r4
#undef r3
#undef r2
#undef r1
#undef r
#undef c4
#undef c3
#undef c2
#undef c1

    __hash_maybe_memfence;
    state_.words[0] += a;
    state_.words[1] += b;
    state_.words[2] += c;
    state_.words[3] += d;
    state_.words[4] += e;
  }

public:
  SHA1() { reset(); }

  virtual ~SHA1() { reset(); }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 20; }
};

const SHA1::word_t SHA1::initvec[] = {0x67452301, 0xefcdab89, 0x98badcfe,
                                      0x10325476, 0xc3d2e1f0};

class SHA256 : public AlgorithmImpl<uint32_t, 16, 8> {
private:
  static const word_t initvec[];

protected:
  virtual void transform(const word_t* buffer)
  {
    __hash_assign_words(__crypto_be);
    __hash_maybe_memfence;

    word_t a, b, c, d, e, f, g, h;
    word_t t;
    a = state_.words[0];
    b = state_.words[1];
    c = state_.words[2];
    d = state_.words[3];
    e = state_.words[4];
    f = state_.words[5];
    g = state_.words[6];
    h = state_.words[7];
    __hash_maybe_memfence;

#define b0(w) (ror(w, 2) ^ ror(w, 13) ^ ror(w, 22))
#define b1(w) (ror(w, 6) ^ ror(w, 11) ^ ror(w, 25))

#define s0(w) (ror(w, 7) ^ ror(w, 18) ^ (w >> 3))
#define s1(w) (ror(w, 17) ^ ror(w, 19) ^ (w >> 10))

#define r(a1, a2, a3, a4, a5, a6, a7, a8, k, w)                                \
  a4 += t = a8 + b1(a5) + ch(a5, a6, a7) + k + w;                              \
  a8 = t + b0(a1) + maj(a1, a2, a3);

    // Generated code. See sha256-transgen.py
    // Round 0
    r(a, b, c, d, e, f, g, h, 0x428a2f98, w00);
    r(h, a, b, c, d, e, f, g, 0x71374491, w01);
    r(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w02);
    r(f, g, h, a, b, c, d, e, 0xe9b5dba5, w03);
    r(e, f, g, h, a, b, c, d, 0x3956c25b, w04);
    r(d, e, f, g, h, a, b, c, 0x59f111f1, w05);
    r(c, d, e, f, g, h, a, b, 0x923f82a4, w06);
    r(b, c, d, e, f, g, h, a, 0xab1c5ed5, w07);

    // Round 8
    r(a, b, c, d, e, f, g, h, 0xd807aa98, w08);
    r(h, a, b, c, d, e, f, g, 0x12835b01, w09);
    r(g, h, a, b, c, d, e, f, 0x243185be, w10);
    r(f, g, h, a, b, c, d, e, 0x550c7dc3, w11);
    r(e, f, g, h, a, b, c, d, 0x72be5d74, w12);
    r(d, e, f, g, h, a, b, c, 0x80deb1fe, w13);
    r(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14);
    r(b, c, d, e, f, g, h, a, 0xc19bf174, w15);

    // Round 16
    w00 = s1(w14) + w09 + s0(w01) + w00,
    r(a, b, c, d, e, f, g, h, 0xe49b69c1, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01,
    r(h, a, b, c, d, e, f, g, 0xefbe4786, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02,
    r(g, h, a, b, c, d, e, f, 0x0fc19dc6, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03,
    r(f, g, h, a, b, c, d, e, 0x240ca1cc, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04,
    r(e, f, g, h, a, b, c, d, 0x2de92c6f, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05,
    r(d, e, f, g, h, a, b, c, 0x4a7484aa, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06,
    r(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07,
    r(b, c, d, e, f, g, h, a, 0x76f988da, w07);

    // Round 24
    w08 = s1(w06) + w01 + s0(w09) + w08,
    r(a, b, c, d, e, f, g, h, 0x983e5152, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09,
    r(h, a, b, c, d, e, f, g, 0xa831c66d, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10,
    r(g, h, a, b, c, d, e, f, 0xb00327c8, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11,
    r(f, g, h, a, b, c, d, e, 0xbf597fc7, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12,
    r(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13,
    r(d, e, f, g, h, a, b, c, 0xd5a79147, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14,
    r(c, d, e, f, g, h, a, b, 0x06ca6351, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15,
    r(b, c, d, e, f, g, h, a, 0x14292967, w15);

    // Round 32
    w00 = s1(w14) + w09 + s0(w01) + w00,
    r(a, b, c, d, e, f, g, h, 0x27b70a85, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01,
    r(h, a, b, c, d, e, f, g, 0x2e1b2138, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02,
    r(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03,
    r(f, g, h, a, b, c, d, e, 0x53380d13, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04,
    r(e, f, g, h, a, b, c, d, 0x650a7354, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05,
    r(d, e, f, g, h, a, b, c, 0x766a0abb, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06,
    r(c, d, e, f, g, h, a, b, 0x81c2c92e, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07,
    r(b, c, d, e, f, g, h, a, 0x92722c85, w07);

    // Round 40
    w08 = s1(w06) + w01 + s0(w09) + w08,
    r(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09,
    r(h, a, b, c, d, e, f, g, 0xa81a664b, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10,
    r(g, h, a, b, c, d, e, f, 0xc24b8b70, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11,
    r(f, g, h, a, b, c, d, e, 0xc76c51a3, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12,
    r(e, f, g, h, a, b, c, d, 0xd192e819, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13,
    r(d, e, f, g, h, a, b, c, 0xd6990624, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14,
    r(c, d, e, f, g, h, a, b, 0xf40e3585, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15,
    r(b, c, d, e, f, g, h, a, 0x106aa070, w15);

    // Round 48
    w00 = s1(w14) + w09 + s0(w01) + w00,
    r(a, b, c, d, e, f, g, h, 0x19a4c116, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01,
    r(h, a, b, c, d, e, f, g, 0x1e376c08, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02,
    r(g, h, a, b, c, d, e, f, 0x2748774c, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03,
    r(f, g, h, a, b, c, d, e, 0x34b0bcb5, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04,
    r(e, f, g, h, a, b, c, d, 0x391c0cb3, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05,
    r(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06,
    r(c, d, e, f, g, h, a, b, 0x5b9cca4f, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07,
    r(b, c, d, e, f, g, h, a, 0x682e6ff3, w07);

    // Round 56
    w08 = s1(w06) + w01 + s0(w09) + w08,
    r(a, b, c, d, e, f, g, h, 0x748f82ee, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09,
    r(h, a, b, c, d, e, f, g, 0x78a5636f, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10,
    r(g, h, a, b, c, d, e, f, 0x84c87814, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11,
    r(f, g, h, a, b, c, d, e, 0x8cc70208, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12,
    r(e, f, g, h, a, b, c, d, 0x90befffa, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13,
    r(d, e, f, g, h, a, b, c, 0xa4506ceb, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14,
    r(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15,
    r(b, c, d, e, f, g, h, a, 0xc67178f2, w15);

#undef b0
#undef b1
#undef s0
#undef s1
#undef r

    __hash_maybe_memfence;
    state_.words[0] += a;
    state_.words[1] += b;
    state_.words[2] += c;
    state_.words[3] += d;
    state_.words[4] += e;
    state_.words[5] += f;
    state_.words[6] += g;
    state_.words[7] += h;
  }

public:
  SHA256() { reset(); }

  virtual ~SHA256() { reset(); }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 32; }
};

const SHA256::word_t SHA256::initvec[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372,
                                          0xa54ff53a, 0x510e527f, 0x9b05688c,
                                          0x1f83d9ab, 0x5be0cd19};

class SHA224 : public SHA256 {
private:
  static const word_t initvec[];

protected:
  virtual std::string digest()
  {
    return std::string((const char*)state_.bytes,
                       sizeof(state_.bytes) - sizeof(word_t));
  }

public:
  SHA224() { reset(); }

  virtual ~SHA224() { reset(); }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 28; }
};

const SHA224::word_t SHA224::initvec[] = {0xc1059ed8, 0x367cd507, 0x3070dd17,
                                          0xf70e5939, 0xffc00b31, 0x68581511,
                                          0x64f98fa7, 0xbefa4fa4};

class SHA512 : public AlgorithmImpl<uint64_t, 16, 8> {
private:
  static const word_t initvec[];

protected:
  virtual void transform(const word_t* buffer)
  {
    __hash_assign_words(__crypto_be);
    __hash_maybe_memfence;

    word_t a, b, c, d, e, f, g, h;
    word_t t;
    a = state_.words[0];
    b = state_.words[1];
    c = state_.words[2];
    d = state_.words[3];
    e = state_.words[4];
    f = state_.words[5];
    g = state_.words[6];
    h = state_.words[7];
    __hash_maybe_memfence;

#define b0(w) (ror(w, 28) ^ ror(w, 34) ^ ror(w, 39))
#define b1(w) (ror(w, 14) ^ ror(w, 18) ^ ror(w, 41))

#define s0(w) (ror(w, 1) ^ ror(w, 8) ^ (w >> 7))
#define s1(w) (ror(w, 19) ^ ror(w, 61) ^ (w >> 6))

#define r(a1, a2, a3, a4, a5, a6, a7, a8, k, w)                                \
  a4 += t = a8 + b1(a5) + ch(a5, a6, a7) + k + w;                              \
  a8 = t + b0(a1) + maj(a1, a2, a3);

    // Generated code. See sha512-transgen.py
    // Round 0
    r(a, b, c, d, e, f, g, h, 0x428a2f98d728ae22, w00);
    r(h, a, b, c, d, e, f, g, 0x7137449123ef65cd, w01);
    r(g, h, a, b, c, d, e, f, 0xb5c0fbcfec4d3b2f, w02);
    r(f, g, h, a, b, c, d, e, 0xe9b5dba58189dbbc, w03);
    r(e, f, g, h, a, b, c, d, 0x3956c25bf348b538, w04);
    r(d, e, f, g, h, a, b, c, 0x59f111f1b605d019, w05);
    r(c, d, e, f, g, h, a, b, 0x923f82a4af194f9b, w06);
    r(b, c, d, e, f, g, h, a, 0xab1c5ed5da6d8118, w07);

    // Round 8
    r(a, b, c, d, e, f, g, h, 0xd807aa98a3030242, w08);
    r(h, a, b, c, d, e, f, g, 0x12835b0145706fbe, w09);
    r(g, h, a, b, c, d, e, f, 0x243185be4ee4b28c, w10);
    r(f, g, h, a, b, c, d, e, 0x550c7dc3d5ffb4e2, w11);
    r(e, f, g, h, a, b, c, d, 0x72be5d74f27b896f, w12);
    r(d, e, f, g, h, a, b, c, 0x80deb1fe3b1696b1, w13);
    r(c, d, e, f, g, h, a, b, 0x9bdc06a725c71235, w14);
    r(b, c, d, e, f, g, h, a, 0xc19bf174cf692694, w15);

    // Round 16
    w00 = s1(w14) + w09 + s0(w01) + w00;
    r(a, b, c, d, e, f, g, h, 0xe49b69c19ef14ad2, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01;
    r(h, a, b, c, d, e, f, g, 0xefbe4786384f25e3, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02;
    r(g, h, a, b, c, d, e, f, 0x0fc19dc68b8cd5b5, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03;
    r(f, g, h, a, b, c, d, e, 0x240ca1cc77ac9c65, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04;
    r(e, f, g, h, a, b, c, d, 0x2de92c6f592b0275, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05;
    r(d, e, f, g, h, a, b, c, 0x4a7484aa6ea6e483, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06;
    r(c, d, e, f, g, h, a, b, 0x5cb0a9dcbd41fbd4, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07;
    r(b, c, d, e, f, g, h, a, 0x76f988da831153b5, w07);

    // Round 24
    w08 = s1(w06) + w01 + s0(w09) + w08;
    r(a, b, c, d, e, f, g, h, 0x983e5152ee66dfab, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09;
    r(h, a, b, c, d, e, f, g, 0xa831c66d2db43210, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10;
    r(g, h, a, b, c, d, e, f, 0xb00327c898fb213f, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11;
    r(f, g, h, a, b, c, d, e, 0xbf597fc7beef0ee4, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12;
    r(e, f, g, h, a, b, c, d, 0xc6e00bf33da88fc2, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13;
    r(d, e, f, g, h, a, b, c, 0xd5a79147930aa725, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14;
    r(c, d, e, f, g, h, a, b, 0x06ca6351e003826f, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15;
    r(b, c, d, e, f, g, h, a, 0x142929670a0e6e70, w15);

    // Round 32
    w00 = s1(w14) + w09 + s0(w01) + w00;
    r(a, b, c, d, e, f, g, h, 0x27b70a8546d22ffc, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01;
    r(h, a, b, c, d, e, f, g, 0x2e1b21385c26c926, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02;
    r(g, h, a, b, c, d, e, f, 0x4d2c6dfc5ac42aed, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03;
    r(f, g, h, a, b, c, d, e, 0x53380d139d95b3df, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04;
    r(e, f, g, h, a, b, c, d, 0x650a73548baf63de, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05;
    r(d, e, f, g, h, a, b, c, 0x766a0abb3c77b2a8, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06;
    r(c, d, e, f, g, h, a, b, 0x81c2c92e47edaee6, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07;
    r(b, c, d, e, f, g, h, a, 0x92722c851482353b, w07);

    // Round 40
    w08 = s1(w06) + w01 + s0(w09) + w08;
    r(a, b, c, d, e, f, g, h, 0xa2bfe8a14cf10364, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09;
    r(h, a, b, c, d, e, f, g, 0xa81a664bbc423001, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10;
    r(g, h, a, b, c, d, e, f, 0xc24b8b70d0f89791, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11;
    r(f, g, h, a, b, c, d, e, 0xc76c51a30654be30, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12;
    r(e, f, g, h, a, b, c, d, 0xd192e819d6ef5218, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13;
    r(d, e, f, g, h, a, b, c, 0xd69906245565a910, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14;
    r(c, d, e, f, g, h, a, b, 0xf40e35855771202a, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15;
    r(b, c, d, e, f, g, h, a, 0x106aa07032bbd1b8, w15);

    // Round 48
    w00 = s1(w14) + w09 + s0(w01) + w00;
    r(a, b, c, d, e, f, g, h, 0x19a4c116b8d2d0c8, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01;
    r(h, a, b, c, d, e, f, g, 0x1e376c085141ab53, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02;
    r(g, h, a, b, c, d, e, f, 0x2748774cdf8eeb99, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03;
    r(f, g, h, a, b, c, d, e, 0x34b0bcb5e19b48a8, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04;
    r(e, f, g, h, a, b, c, d, 0x391c0cb3c5c95a63, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05;
    r(d, e, f, g, h, a, b, c, 0x4ed8aa4ae3418acb, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06;
    r(c, d, e, f, g, h, a, b, 0x5b9cca4f7763e373, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07;
    r(b, c, d, e, f, g, h, a, 0x682e6ff3d6b2b8a3, w07);

    // Round 56
    w08 = s1(w06) + w01 + s0(w09) + w08;
    r(a, b, c, d, e, f, g, h, 0x748f82ee5defb2fc, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09;
    r(h, a, b, c, d, e, f, g, 0x78a5636f43172f60, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10;
    r(g, h, a, b, c, d, e, f, 0x84c87814a1f0ab72, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11;
    r(f, g, h, a, b, c, d, e, 0x8cc702081a6439ec, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12;
    r(e, f, g, h, a, b, c, d, 0x90befffa23631e28, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13;
    r(d, e, f, g, h, a, b, c, 0xa4506cebde82bde9, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14;
    r(c, d, e, f, g, h, a, b, 0xbef9a3f7b2c67915, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15;
    r(b, c, d, e, f, g, h, a, 0xc67178f2e372532b, w15);

    // Round 64
    w00 = s1(w14) + w09 + s0(w01) + w00;
    r(a, b, c, d, e, f, g, h, 0xca273eceea26619c, w00);
    w01 = s1(w15) + w10 + s0(w02) + w01;
    r(h, a, b, c, d, e, f, g, 0xd186b8c721c0c207, w01);
    w02 = s1(w00) + w11 + s0(w03) + w02;
    r(g, h, a, b, c, d, e, f, 0xeada7dd6cde0eb1e, w02);
    w03 = s1(w01) + w12 + s0(w04) + w03;
    r(f, g, h, a, b, c, d, e, 0xf57d4f7fee6ed178, w03);
    w04 = s1(w02) + w13 + s0(w05) + w04;
    r(e, f, g, h, a, b, c, d, 0x06f067aa72176fba, w04);
    w05 = s1(w03) + w14 + s0(w06) + w05;
    r(d, e, f, g, h, a, b, c, 0x0a637dc5a2c898a6, w05);
    w06 = s1(w04) + w15 + s0(w07) + w06;
    r(c, d, e, f, g, h, a, b, 0x113f9804bef90dae, w06);
    w07 = s1(w05) + w00 + s0(w08) + w07;
    r(b, c, d, e, f, g, h, a, 0x1b710b35131c471b, w07);

    // Round 72
    w08 = s1(w06) + w01 + s0(w09) + w08;
    r(a, b, c, d, e, f, g, h, 0x28db77f523047d84, w08);
    w09 = s1(w07) + w02 + s0(w10) + w09;
    r(h, a, b, c, d, e, f, g, 0x32caab7b40c72493, w09);
    w10 = s1(w08) + w03 + s0(w11) + w10;
    r(g, h, a, b, c, d, e, f, 0x3c9ebe0a15c9bebc, w10);
    w11 = s1(w09) + w04 + s0(w12) + w11;
    r(f, g, h, a, b, c, d, e, 0x431d67c49c100d4c, w11);
    w12 = s1(w10) + w05 + s0(w13) + w12;
    r(e, f, g, h, a, b, c, d, 0x4cc5d4becb3e42b6, w12);
    w13 = s1(w11) + w06 + s0(w14) + w13;
    r(d, e, f, g, h, a, b, c, 0x597f299cfc657e2a, w13);
    w14 = s1(w12) + w07 + s0(w15) + w14;
    r(c, d, e, f, g, h, a, b, 0x5fcb6fab3ad6faec, w14);
    w15 = s1(w13) + w08 + s0(w00) + w15;
    r(b, c, d, e, f, g, h, a, 0x6c44198c4a475817, w15);

#undef b0
#undef b1
#undef s0
#undef s1
#undef r

    __hash_maybe_memfence;
    state_.words[0] += a;
    state_.words[1] += b;
    state_.words[2] += c;
    state_.words[3] += d;
    state_.words[4] += e;
    state_.words[5] += f;
    state_.words[6] += g;
    state_.words[7] += h;
  }

public:
  SHA512() { reset(); }

  virtual ~SHA512() { reset(); }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 64; }
};

const SHA512::word_t SHA512::initvec[] = {
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b,
    0xa54ff53a5f1d36f1, 0x510e527fade682d1, 0x9b05688c2b3e6c1f,
    0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
};

class SHA384 : public SHA512 {
private:
  static const word_t initvec[];

protected:
  virtual std::string digest()
  {
    return std::string((const char*)state_.bytes,
                       sizeof(state_.bytes) - sizeof(word_t) * 2);
  }

public:
  SHA384() { reset(); }

  virtual ~SHA384() { reset(); }

  virtual void reset()
  {
    memset(buffer_.bytes, 0, sizeof(buffer_));
    memcpy(state_.bytes, initvec, sizeof(state_));
    count_ = offset_ = 0;
  }

  virtual inline uint_fast16_t length() const { return 48; }
};

const SHA384::word_t SHA384::initvec[] = {
    0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17,
    0x152fecd8f70e5939, 0x67332667ffc00b31, 0x8eb44a8768581511,
    0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4};

namespace {
// For |all|
static const std::set<std::string> names{
    "md5",    "sha",     "sha1",   "sha-1",   "sha224", "sha-224",
    "sha256", "sha-256", "sha384", "sha-384", "sha512", "sha-512",
};

// For fast name |lookup|
static const std::unordered_map<std::string, Algorithms> mapped{
    {"md5", algoMD5},        {"sha", algoSHA1},       {"sha1", algoSHA1},
    {"sha-1", algoSHA1},     {"sha224", algoSHA224},  {"sha-224", algoSHA224},
    {"sha256", algoSHA256},  {"sha-256", algoSHA256}, {"sha384", algoSHA384},
    {"sha-384", algoSHA384}, {"sha512", algoSHA512},  {"sha-512", algoSHA512},
};
static const auto mapped_end = mapped.end();
} // namespace

const std::set<std::string>& crypto::hash::all() { return names; }

Algorithms crypto::hash::lookup(const std::string& name)
{
  auto i = mapped.find(name);
  if (unlikely(i == mapped_end)) {
    return algoNone;
  }
  return i->second;
}

std::unique_ptr<Algorithm> crypto::hash::create(Algorithms algo)
{
  switch (algo) {
  case algoMD5:
    return aria2::make_unique<MD5>();

  case algoSHA1:
    return aria2::make_unique<SHA1>();

  case algoSHA224:
    return aria2::make_unique<SHA224>();

  case algoSHA256:
    return aria2::make_unique<SHA256>();

  case algoSHA384:
    return aria2::make_unique<SHA384>();

  case algoSHA512:
    return aria2::make_unique<SHA512>();

  default:
    throw std::domain_error("Invalid hash algorithm");
  }
}
