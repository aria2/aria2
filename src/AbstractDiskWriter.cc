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
#include "AbstractDiskWriter.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "DlAbortEx.h"
#include "File.h"
#include "Util.h"

AbstractDiskWriter::AbstractDiskWriter():fd(0) {
#ifdef HAVE_LIBSSL
  EVP_MD_CTX_init(&ctx);
#endif // HAVE_LIBSSL
}

AbstractDiskWriter::~AbstractDiskWriter() {
  if(fd != 0) {
    close(fd);
  }
#ifdef HAVE_LIBSSL
  EVP_MD_CTX_cleanup(&ctx);
#endif // HAVE_LIBSSL
}

void AbstractDiskWriter::closeFile() {
  if(fd != 0) {
    close(fd);
    fd = 0;
  }
}

void AbstractDiskWriter::openExistingFile(string filename) {
  File f(filename);
  if(!f.isFile()) {
    throw new DlAbortEx(strerror(errno));
  }

  if((fd = open(filename.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}

void AbstractDiskWriter::createFile(string filename, int addFlags) {
  // TODO proper filename handling needed
  assert(filename.size());
//   if(filename.empty()) {
//     filename = "index.html";
//   }
  if((fd = open(filename.c_str(), O_CREAT|O_RDWR|O_TRUNC|addFlags, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }  
}

void AbstractDiskWriter::writeDataInternal(const char* data, int len) {
  if(write(fd, data, len) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
}

int AbstractDiskWriter::readDataInternal(char* data, int len) {
  int ret;
  if((ret = read(fd, data, len)) < 0) {
    throw new DlAbortEx(strerror(errno));
  }
  return ret;
}

string AbstractDiskWriter::sha1Sum(long long int offset, long long int length) {
#ifdef HAVE_LIBSSL
  EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL);
  try {
    int BUFSIZE = 16*1024;
    char buf[BUFSIZE];
    for(int i = 0; i < length/BUFSIZE; i++) {
      if(BUFSIZE != readData(buf, BUFSIZE, offset)) {
	throw "error";
      }
      EVP_DigestUpdate(&ctx, buf, BUFSIZE);
      offset += BUFSIZE;
    }
    int r = length%BUFSIZE;
    if(r > 0) {
      if(r != readData(buf, r, offset)) {
	throw "error";
      }
      EVP_DigestUpdate(&ctx, buf, r);
    }
    unsigned char hashValue[20];
    int len;
    EVP_DigestFinal_ex(&ctx, hashValue, (unsigned int*)&len);
    return Util::toHex(hashValue, 20);
  } catch(string ex) {
    throw new DlAbortEx(strerror(errno));
  }
#else
  return "";
#endif // HASHVALUE
}

void AbstractDiskWriter::seek(long long int offset) {
  if(offset != lseek(fd, offset, SEEK_SET)) {
    throw new DlAbortEx(strerror(errno));
  }
}

void AbstractDiskWriter::writeData(const char* data, int len, long long int offset) {
  seek(offset);
  writeDataInternal(data, len);
}

int AbstractDiskWriter::readData(char* data, int len, long long int offset) {
  seek(offset);
  return readDataInternal(data, len);
}
