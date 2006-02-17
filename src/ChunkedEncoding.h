/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_CHUNKED_ENCODING_H_
#define _D_CHUNKED_ENCODING_H_

#include "TransferEncoding.h"
#include "common.h"

class ChunkedEncoding:public TransferEncoding {
private:
  enum STATE {
    READ_SIZE,
    READ_DATA,
    FINISH
  };
  long int chunkSize;
  int state;
  char* strbuf;
  int strbufSize;

  int readChunkSize(char** pp);
  int readData(char** pp, char* buf, int& len, int maxlen);
  void addBuffer(const char* inbuf, int inlen);
  int readDataEOL(char** pp);


public:
  ChunkedEncoding();
  ~ChunkedEncoding();

  void init();
  void inflate(char* outbuf, int& outlen, const char* inbuf, int inlen);
  bool finished();
  void end();
};

#endif // _D_CHUNKED_ENCODING_H_
