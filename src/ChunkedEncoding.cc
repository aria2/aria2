/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "ChunkedEncoding.h"
#include "DlAbortEx.h"
#include "message.h"
#include <string.h>
#include <strings.h>
#include <errno.h>

#define MAX_BUFSIZE (1024*1024)

ChunkedEncoding::ChunkedEncoding() {
  strbufSize = 4096;
  strbuf = new char[strbufSize];
  strbufTail = strbuf;
  state = READ_SIZE;
  chunkSize = 0;
}

ChunkedEncoding::~ChunkedEncoding() {
  if(strbuf != NULL) {
    delete [] strbuf;
  }
}

void ChunkedEncoding::init() {
}

bool ChunkedEncoding::finished() {
  return state == FINISH ? true : false;
}

void ChunkedEncoding::end() {}

void ChunkedEncoding::inflate(char* outbuf, int& outlen, const char* inbuf, int inlen) {
  addBuffer(inbuf, inlen);
  char* p = strbuf;
  int clen = 0;
  while(1) {
    if(state == READ_SIZE) {
      if(readChunkSize(&p) == 0) {
	if(chunkSize == 0) {
	  state = FINISH;
	} else {
	  state = READ_DATA;
	}
      } else {
	// chunk size is not fully received.
	break;
      }
    } else if(state == READ_DATA) {
      if(readData(&p, outbuf, clen, outlen) == 0) {
	state = READ_SIZE;
      } else {
	break;
      }
    } else {
      break;
    }
    // all bytes in strbuf were examined?
    if(strbufTail <= p) {
      break;
    }
  }
  if(strbufTail <= p) {
    strbufTail = strbuf;
  } else {
    // copy string between [p, strbufTail]
    int unreadSize = strbufTail-p;
    char* temp = new char[strbufSize];
    memcpy(temp, p, unreadSize);
    delete [] strbuf;
    strbuf = temp;
    strbufTail = strbuf+unreadSize;
  }
  outlen = clen;
}

int ChunkedEncoding::readData(char** pp, char* buf, int& len, int maxlen) {
  if(buf+len == buf+maxlen) {
    return -1;
  }
  if(chunkSize == 0) {
    return readDataEOL(pp);
  }
  int wsize;
  if(strbufTail-*pp < chunkSize) {
    wsize = strbufTail-*pp <= maxlen-len ? strbufTail-*pp : maxlen-len;
  } else {
    wsize = chunkSize <= maxlen-len ? chunkSize : maxlen-len;
  }
  memcpy(buf+len, *pp, wsize);
  chunkSize -= wsize;
  len += wsize;
  *pp += wsize;
  if(chunkSize == 0) {
    return readDataEOL(pp);
  } else {
    return -1;
  }
}

int ChunkedEncoding::readDataEOL(char** pp) {
  char* np = (char*)memchr(*pp, '\n', strbufTail-*pp);
  char* rp = (char*)memchr(*pp, '\r', strbufTail-*pp);
  if(np != NULL && rp != NULL && np-rp == 1 && *pp == rp) {
    *pp += 2;
    return 0;
  } else if(strbufTail-*pp < 2) {
    return -1;
  } else {
    throw new DlAbortEx(EX_INVALID_CHUNK_SIZE);
  }  
}

int ChunkedEncoding::readChunkSize(char** pp) {
  // we read chunk-size from *pp
  char* p;
  char* np = (char*)memchr(*pp, '\n', strbufTail-*pp);
  char* rp = (char*)memchr(*pp, '\r', strbufTail-*pp);
  if(np == NULL || rp == NULL ||  np-rp != 1) {
    // \r\n is not found. Return -1
    return -1;
  }
  p = rp;

  // We ignore chunk-extension
  char* exsp = (char*)memchr(*pp, ';', strbufTail-*pp);
  if(exsp == NULL) {
    exsp = p;
  }
  // TODO check invalid characters in buffer
  char* temp = new char[exsp-*pp+1];
  memcpy(temp, *pp, exsp-*pp);
  temp[exsp-*pp] = '\0';

  chunkSize = strtol(temp, NULL, 16);
  delete [] temp;
  if(chunkSize < 0) {
    throw new DlAbortEx(EX_INVALID_CHUNK_SIZE);
  } else if(errno == ERANGE && (chunkSize == LONG_MAX || chunkSize == LONG_MIN)) {
    throw new DlAbortEx(strerror(errno));
  }
  *pp = p+2;
  return 0;
}

void ChunkedEncoding::addBuffer(const char* inbuf, int inlen) {
  int realbufSize = strbufTail-strbuf;
  if(realbufSize+inlen >= strbufSize) {
    if(realbufSize+inlen > MAX_BUFSIZE) {
      throw new DlAbortEx(EX_TOO_LARGE_CHUNK, realbufSize+inlen);
    }
    strbufSize = realbufSize+inlen;
    char* temp = new char[strbufSize];
    memcpy(temp, strbuf, realbufSize);
    delete [] strbuf;
    strbuf = temp;
    strbufTail = strbuf+realbufSize;
  }
  memcpy(strbufTail, inbuf, inlen);
  strbufTail += inlen;
}
