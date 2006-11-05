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
#include "Util.h"
#include "DlAbortEx.h"
#include "File.h"
#include "message.h"
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

string Util::itos(int value, bool comma) {
  string str = llitos(value, comma);
  return str;
}

string Util::llitos(long long int value, bool comma)
{
  string str;
  bool flag = false;
  if(value < 0) {
    flag = true;
    value = -value;
  } else if(value == 0) {
    str = "0";
    return str;
  }
  int count = 0;
  while(value) {
    ++count;
    char digit = value%10+'0';
    str.insert(str.begin(), digit);
    value /= 10;
    if(comma && count > 3 && count%3 == 1) {
      str.insert(str.begin()+1, ',');
    }
  }
  if(flag) {
    str.insert(str.begin(), '-');
  }
  return str;
}

string Util::trim(const string& src) {
  string::size_type sp = src.find_first_not_of("\r\n\t ");
  string::size_type ep = src.find_last_not_of("\r\n\t ");
  if(sp == string::npos || ep == string::npos) {
    return "";
  } else {
    return src.substr(sp, ep-sp+1);
  }
}

void Util::split(pair<string, string>& hp, const string& src, char delim) {
  hp.first = "";
  hp.second = "";
  string::size_type p = src.find(delim);
  if(p == string::npos) {
    hp.first = src;
    hp.second = "";
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
}

long long int Util::difftv(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec || tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec) {
    return 0;
  }
  return ((long long int)(tv1.tv_sec-tv2.tv_sec)*1000000+
	  tv1.tv_usec-tv2.tv_usec);
}

int Util::difftvsec(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec) {
    return 0;
  }
  return tv1.tv_sec-tv2.tv_sec;
}

void Util::slice(Strings& result, const string& src, char delim, bool doTrim) {
  string::size_type p = 0;
  while(1) {
    string::size_type np = src.find(delim, p);
    if(np == string::npos) {
      string term = src.substr(p);
      if(doTrim) {
	term = trim(term);
      }
      if(term.size()) {
	result.push_back(term);
      }
      break;
    }
    string term = src.substr(p, np-p);
    if(doTrim) {
      term = trim(term);
    }
    p = np+1;
    if(term.size()) {
      result.push_back(term);
    }
  } 
}

bool Util::startsWith(const string& target, const string& part) {
  if(target.size() < part.size()) {
    return false;
  }
  if(part == "") {
    return true;
  }
  if(target.find(part) == 0) {
    return true;
  } else {
    return false;
  }
}

bool Util::endsWith(const string& target, const string& part) {
  if(target.size() < part.size()) {
    return false;
  }
  if(part == "") {
    return true;
  }
  if(target.rfind(part) == target.size()-part.size()) {
    return true;
  } else {
    return false;
  }
}

string Util::replace(const string& target, const string& oldstr, const string& newstr) {
  if(target == "" || oldstr == "" ) {
    return target;
  }
  string result;
  string::size_type p = 0;
  string::size_type np = target.find(oldstr);
  while(np != string::npos) {
    result += target.substr(p, np-p)+newstr;
    p = np+oldstr.size();
    np = target.find(oldstr, p);
  }
  result += target.substr(p);

  return result;
}

string Util::urlencode(const unsigned char* target, int len) {
  string dest;
  for(int i = 0; i < len; i++) {
    if(!('0' <= target[i] && target[i] <= '9' ||
	 'A' <= target[i] && target[i] <= 'Z' ||
	 'a' <= target[i] && target[i] <= 'z' ||
	 '$' == target[i] || '-' == target[i] ||
	 '_' == target[i] || '.' == target[i] ||
	 '+' == target[i] || '!' == target[i] ||
	 '*' == target[i] || '\'' == target[i] ||
	 '(' == target[i] || ')' == target[i] ||
	 ',' == target[i])) {
      char temp[4];
      sprintf(temp, "%%%02x", target[i]);
      temp[sizeof(temp)-1] = '\0';
      dest.append(temp);
    } else {
      dest += target[i];
    }
  }
  return dest;
}

string Util::torrentUrlencode(const unsigned char* target, int len) {
  string dest;
  for(int i = 0; i < len; i++) {
    if(isalpha(target[i]) || isdigit(target[i])) {
      dest += target[i];
    } else {
      char temp[4];
      sprintf(temp, "%%%02x", target[i]);
      temp[sizeof(temp)-1] = '\0';
      dest.append(temp);
    }
  }
  return dest;
}

string Util::urldecode(const string& target) {
  string result;
  for(string::const_iterator itr = target.begin();
      itr != target.end(); itr++) {
    if(*itr == '%') {
      if(itr+1 != target.end() && itr+2 != target.end() &&
	 isxdigit(*(itr+1)) && isxdigit(*(itr+2))) {
	char temp[3];
	temp[0] = *(itr+1);
	temp[1] = *(itr+2);
	temp[2] = '\0';
	result += strtol(temp, 0, 16);
	itr += 2;
      } else {
	result += *itr;
      }
    } else {
      result += *itr;
    }
  }
  return result;
}

string Util::toHex(const unsigned char* src, int len) {
  char* temp = new char[len*2+1];
  for(int i = 0; i < len; i++) {
    sprintf(temp+i*2, "%02x", src[i]);
  }
  temp[len*2] = '\0';
  string hex = temp;
  delete [] temp;
  return hex;
}

FILE* Util::openFile(const string& filename, const string& mode) {
  FILE* file = fopen(filename.c_str(), mode.c_str());
  return file;
}

void Util::fileCopy(const string& dest, const string& src) {
  File file(src);
  rangedFileCopy(dest, src, 0, file.size());
}

void Util::rangedFileCopy(const string& dest, const string& src, long long int srcOffset, long long int length) {
  int bufSize = 4096;
  char buf[bufSize];
  int destFd = -1;
  int srcFd = -1;
  try {
    if((destFd = open(dest.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
      throw new DlAbortEx(EX_FILE_OPEN, dest.c_str(), strerror(errno));
    }
    if((srcFd = open(src.c_str(), O_RDONLY, S_IRUSR|S_IWUSR)) == -1) {
      throw new DlAbortEx(EX_FILE_OPEN, src.c_str(), strerror(errno));
    }
    if(lseek(srcFd, srcOffset, SEEK_SET) != srcOffset) {
      throw new DlAbortEx(EX_FILE_SEEK, src.c_str(), strerror(errno));
    }
    int x = length/bufSize;
    int r = length%bufSize;
    for(int i = 0; i < x; i++) {
      int readLength;
      if((readLength = read(srcFd, buf, bufSize)) == -1 || readLength != bufSize) {
	throw new DlAbortEx(EX_FILE_READ, src.c_str(), strerror(errno));
      }
      if(write(destFd, buf, readLength) == -1) {
	throw new DlAbortEx(EX_FILE_WRITE, dest.c_str(), strerror(errno));
      }
    }
    if(r > 0) {
      int readLength;
      if((readLength = read(srcFd, buf, r)) == -1 || readLength != r) {
	throw new DlAbortEx(EX_FILE_READ, src.c_str(), strerror(errno));
      }
      if(write(destFd, buf, r) == -1) {
	throw new DlAbortEx(EX_FILE_WRITE, dest.c_str(), strerror(errno));
      }
    }
    close(srcFd);
    close(destFd);
    srcFd = -1;
    destFd = -1;
  } catch(Exception* e) {
    if(srcFd != -1) {
      close(srcFd);
    }
    if(destFd != -1) {
      close(destFd);
    }
    throw;
  }
}

bool Util::isPowerOf(int num, int base) {
  if(base <= 0) { return false; }
  if(base == 1) { return true; }

  while(num%base == 0) {
    num /= base;
    if(num == 1) {
      return true;
    }
  }
  return false;
}

string Util::secfmt(int sec) {
  string str;
  if(sec >= 3600) {
    str = itos(sec/3600)+"h";
    sec %= 3600;
  }
  if(sec >= 60) {
    int min = sec/60;
    if(min < 10) {
      str += "0";
    }
    str += itos(min)+"m";
    sec %= 60;
  }
  if(sec < 10) {
    str += "0";
  }
  str += itos(sec)+"s";
  return str;
}

int Util::expandBuffer(char** pbuf, int curLength, int newLength) {
  char* newbuf = new char[newLength];
  memcpy(newbuf, *pbuf, curLength);
  delete [] *pbuf;
  *pbuf = newbuf;
  return newLength;
}

int getNum(const char* buf, int offset, int length) {
  char* temp = new char[length+1];
  memcpy(temp, buf+offset, length);
  temp[length] = '\0';
  int x = strtol(temp, NULL, 10);
  delete [] temp;
  return x;
}

void unfoldSubRange(const string& src, Integers& range) {
  if(src.empty()) {
    return;
  }
  string::size_type p = src.find_first_of(",-");
  if(p == 0) {
    return;
  } else if(p == string::npos) {
    range.push_back(atoi(src.c_str()));
  } else {
    if(src.at(p) == ',') {
      int num = getNum(src.c_str(), 0, p);
      range.push_back(num);
      unfoldSubRange(src.substr(p+1), range);
    } else if(src.at(p) == '-') {
      int rightNumBegin = p+1;
      string::size_type nextDelim = src.find_first_of(",", rightNumBegin);
      if(nextDelim == string::npos) {
	nextDelim = src.size();
      }
      int left = getNum(src.c_str(), 0, p);
      int right = getNum(src.c_str(), rightNumBegin, nextDelim-rightNumBegin);
      for(int i = left; i <= right; i++) {
	range.push_back(i);
      }
      if(src.size() > nextDelim) {
	unfoldSubRange(src.substr(nextDelim+1), range);
      }
    }
  }
}

void Util::unfoldRange(const string& src, Integers& range) {
  unfoldSubRange(src, range);
  sort(range.begin(), range.end());
  range.erase(unique(range.begin(), range.end()), range.end());
}

string Util::getContentDispositionFilename(const string& header) {
  string::size_type attributesp = header.find("filename=\"");
  if(attributesp == string::npos) {
    return "";
  }
  string::size_type filenamesp = attributesp+strlen("filename=\"");
  string::size_type filenameep = header.find("\"", filenamesp);
  if(filenameep == string::npos) {
    return "";
  }
  return trim(header.substr(filenamesp, filenameep-filenamesp));
}

#ifdef ENABLE_MESSAGE_DIGEST
void Util::sha1Sum(unsigned char* digest, const void* data, int dataLength) {
  MessageDigestContext ctx(DIGEST_ALGO_SHA1);
  ctx.digestInit();
  ctx.digestUpdate(data, dataLength);
  ctx.digestFinal(digest);
  ctx.digestFree();
}
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_MESSAGE_DIGEST
void Util::fileChecksum(const string& filename, unsigned char* digest,
			MessageDigestContext::DigestAlgo algo) {
  MessageDigestContext ctx(algo);
  ctx.digestInit();

  int BUFLEN = 4096;
  char buf[BUFLEN];

  int fd;
  if((fd = open(filename.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
    throw new DlAbortEx(EX_FILE_OPEN, filename.c_str(), strerror(errno));
  }
  while(1) {
    int size = read(fd, buf, BUFLEN);
    if(size == -1) {
      if(errno == EINTR) {
	continue;
      } else {
	close(fd);
	throw new DlAbortEx(EX_FILE_READ, filename.c_str(), strerror(errno));
      }
    } else if(size > 0) {
      ctx.digestUpdate(buf, size);
    }
    if(size < BUFLEN) {
      break;
    }
  }
  ctx.digestFinal(digest);
  ctx.digestFree();
}
#endif // ENABLE_MESSAGE_DIGEST

#ifdef ENABLE_BITTORRENT
Integers Util::computeFastSet(string ipaddr, const unsigned char* infoHash,
			     int pieces, int fastSetSize) {
  Integers fastSet;
  struct in_addr saddr;
  if(inet_aton(ipaddr.c_str(), &saddr) == 0) {
    abort();
  }
  unsigned char tx[24];
  memcpy(tx, (void*)&saddr.s_addr, 4);
  if((tx[0] & 0x80) == 0 || (tx[0] & 0x40) == 0) {
    tx[2] = 0x00;
    tx[3] = 0x00;
  } else {
    tx[3] = 0x00;
  }
  memcpy(tx+4, infoHash, 20);
  unsigned char x[20];
  sha1Sum(x, tx, 24);
  while((int)fastSet.size() < fastSetSize) {
    for(int i = 0; i < 5 && (int)fastSet.size() < fastSetSize; i++) {
      int j = i*4;
      unsigned int ny;
      memcpy(&ny, x+j, 4);
      unsigned int y = ntohl(ny);
      int index = y%pieces;
      if(find(fastSet.begin(), fastSet.end(), index) == fastSet.end()) {
	fastSet.push_back(index);
      }
    }
    unsigned char temp[20];
    sha1Sum(temp, x, 20);
    memcpy(x, temp, sizeof(x));
  }
  return fastSet;
}
#endif // ENABLE_BITTORRENT

/*
int Util::countBit(unsigned int n) {
  int count = 0;
  while(n > 0) {
    count++;
    n &= (n-1);
  }
  return count;
}
*/

static int nbits[] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, 
};

int Util::countBit(unsigned int n) {
  /*
  return
    nbits[n & 0xffu]+
    nbits[(n >> 8) & 0xffu]+
    nbits[(n >> 16) & 0xffu]+
    nbits[(n >> 24) & 0xffu];
  */
  int count = 0;
  int size = sizeof(unsigned int);
  for(int i = 0; i < size; i++) {
    count += nbits[(n >> i*8) & 0xffu];
  }

  return count;
}

string Util::randomAlpha(int length) {
  string str;
  for(int i = 0; i < length; i++) {
    int index = (int)(((double)52)*random()/(RAND_MAX+1.0));
    char ch;
    if(index < 26) {
      ch = (char)('A'+index);
    } else {
      ch = (char)('a'+index-26);
    }
    str += ch;
  }
  return str;
}

class UpperCase {
public:
  void operator()(char& ch) {
    ch = toupper(ch);
  }
};

string Util::toUpper(const string& src) {
  string temp = src;
  for_each(temp.begin(), temp.end(), UpperCase());
  return temp;
}

class LowerCase {
public:
  void operator()(char& ch) {
    ch = tolower(ch);
  }
};

string Util::toLower(const string& src) {
  string temp = src;
  for_each(temp.begin(), temp.end(), LowerCase());
  return temp;
}

bool Util::isNumbersAndDotsNotation(const string& name) {
  struct sockaddr_in sockaddr;
  if(inet_aton(name.c_str(), &sockaddr.sin_addr)) {
    return true;
  } else {
    return false;
  }
}
