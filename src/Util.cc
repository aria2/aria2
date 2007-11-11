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
#include "SimpleRandomizer.h"
#include "a2netcompat.h"
#include "a2time.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <iomanip>

#ifndef HAVE_SLEEP
# ifdef HAVE_WINSOCK_H
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif // HAVE_WINSOCK_H
#endif // HAVE_SLEEP

template<typename T>
string uint2str(T value, bool comma) {
  string str;
  if(value == 0) {
    str = "0";
    return str;
  }
  int32_t count = 0;
  while(value) {
    ++count;
    char digit = value%10+'0';
    str.insert(str.begin(), digit);
    value /= 10;
    if(comma && count > 3 && count%3 == 1) {
      str.insert(str.begin()+1, ',');
    }
  }
  return str;
}

template<typename T>
string int2str(T value, bool comma) {
  bool flag = false;
  if(value < 0) {
    flag = true;
    value = -value;
  }
  string str = uint2str<T>(value, comma);
  if(flag) {
    str.insert(str.begin(), '-');
  }
  return str;
}



string Util::uitos(uint16_t value, bool comma) {
  return uint2str<uint16_t>(value, comma);
}

string Util::itos(int16_t value, bool comma) {
  return int2str<int16_t>(value, comma);
}

string Util::uitos(uint32_t value, bool comma) {
  return uint2str<uint32_t>(value, comma);
}

string Util::itos(int32_t value, bool comma) {
  return int2str<int32_t>(value, comma);
}

string Util::ullitos(uint64_t value, bool comma) {
  return uint2str<uint64_t>(value, comma);
}

string Util::llitos(int64_t value, bool comma)
{
  return int2str<int64_t>(value, comma);
}

string Util::trim(const string& src, const string& trimCharset) {
  string::size_type sp = src.find_first_not_of(trimCharset);
  string::size_type ep = src.find_last_not_of(trimCharset);
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

pair<string, string> Util::split(const string& src, const string& delims)
{
  pair<string, string> hp;
  hp.first = "";
  hp.second = "";
  string::size_type p = src.find_first_of(delims);
  if(p == string::npos) {
    hp.first = src;
    hp.second = "";
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
  }
  return hp;
}

int64_t Util::difftv(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec || tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec) {
    return 0;
  }
  return ((int64_t)(tv1.tv_sec-tv2.tv_sec)*1000000+
	  tv1.tv_usec-tv2.tv_usec);
}

int32_t Util::difftvsec(struct timeval tv1, struct timeval tv2) {
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

bool Util::shouldUrlencode(const char c)
{
  return !(// ALPHA
	   'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z' ||
	   // DIGIT
	   '0' <= c && c <= '9' ||
	   // safe
	   '$' == c || '-' == c || '_' == c || '.' == c ||
	   // extra
	   '!' == c || '*' == c || '\'' == c ||'(' == c ||
	   ')' == c || ',' == c ||
	   // reserved
	   ';' == c || '/' == c || '?' == c  || ':' == c ||
	   '@' == c || '&' == c || '=' == c || '+' == c);   
}

string Util::urlencode(const unsigned char* target, int32_t len) {
  string dest;
  for(int32_t i = 0; i < len; i++) {
    if(shouldUrlencode(target[i])) {
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

string Util::torrentUrlencode(const unsigned char* target, int32_t len) {
  string dest;
  for(int32_t i = 0; i < len; i++) {
    if('0' <= target[i] && target[i] <= '9' ||
       'A' <= target[i] && target[i] <= 'Z' ||
       'a' <= target[i] && target[i] <= 'z') {
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

string Util::toHex(const unsigned char* src, int32_t len) {
  char* temp = new char[len*2+1];
  for(int32_t i = 0; i < len; i++) {
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

void Util::rangedFileCopy(const string& dest, const string& src, int64_t srcOffset, int64_t length) {
  int32_t bufSize = 4096;
  char buf[bufSize];
  int32_t destFd = -1;
  int32_t srcFd = -1;
  try {
    if((destFd = open(dest.c_str(), O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, OPEN_MODE)) == -1) {
      throw new DlAbortEx(EX_FILE_OPEN, dest.c_str(), strerror(errno));
    }
    if((srcFd = open(src.c_str(), O_RDONLY|O_BINARY, OPEN_MODE)) == -1) {
      throw new DlAbortEx(EX_FILE_OPEN, src.c_str(), strerror(errno));
    }
    if(lseek(srcFd, srcOffset, SEEK_SET) != srcOffset) {
      throw new DlAbortEx(EX_FILE_SEEK, src.c_str(), strerror(errno));
    }
    int32_t x = length/bufSize;
    int32_t r = length%bufSize;
    for(int32_t i = 0; i < x; i++) {
      int32_t readLength;
      if((readLength = read(srcFd, buf, bufSize)) == -1 || readLength != bufSize) {
	throw new DlAbortEx(EX_FILE_READ, src.c_str(), strerror(errno));
      }
      if(write(destFd, buf, readLength) == -1) {
	throw new DlAbortEx(EX_FILE_WRITE, dest.c_str(), strerror(errno));
      }
    }
    if(r > 0) {
      int32_t readLength;
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
  } catch(RecoverableException* e) {
    if(srcFd != -1) {
      close(srcFd);
    }
    if(destFd != -1) {
      close(destFd);
    }
    throw;
  }
}

bool Util::isPowerOf(int32_t num, int32_t base) {
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

string Util::secfmt(int32_t sec) {
  string str;
  if(sec >= 3600) {
    str = itos(sec/3600)+"h";
    sec %= 3600;
  }
  if(sec >= 60) {
    int32_t min = sec/60;
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

int32_t Util::expandBuffer(char** pbuf, int32_t curLength, int32_t newLength) {
  char* newbuf = new char[newLength];
  memcpy(newbuf, *pbuf, curLength);
  delete [] *pbuf;
  *pbuf = newbuf;
  return newLength;
}

int32_t getNum(const char* buf, int32_t offset, int32_t length) {
  char* temp = new char[length+1];
  memcpy(temp, buf+offset, length);
  temp[length] = '\0';
  int32_t x = strtol(temp, NULL, 10);
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
      int32_t num = getNum(src.c_str(), 0, p);
      range.push_back(num);
      unfoldSubRange(src.substr(p+1), range);
    } else if(src.at(p) == '-') {
      int32_t rightNumBegin = p+1;
      string::size_type nextDelim = src.find_first_of(",", rightNumBegin);
      if(nextDelim == string::npos) {
	nextDelim = src.size();
      }
      int32_t left = getNum(src.c_str(), 0, p);
      int32_t right = getNum(src.c_str(), rightNumBegin, nextDelim-rightNumBegin);
      for(int32_t i = left; i <= right; i++) {
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
  string keyName = "filename=";
  string::size_type attributesp = header.find(keyName);
  if(attributesp == string::npos) {
    return "";
  }
  string::size_type filenamesp = attributesp+strlen(keyName.c_str());
  string::size_type filenameep;
  if(filenamesp == header.size()) {
    return "";
  }
  
  if(header[filenamesp] == '\'' || header[filenamesp] == '"') {
    char quoteChar = header[filenamesp];
    filenameep = header.find(quoteChar, filenamesp+1);
  } else {
    filenameep = header.find(';', filenamesp);
  }
  if(filenameep == string::npos) {
    filenameep = header.size();
  }
  return trim(header.substr(filenamesp, filenameep-filenamesp), "\r\n '\"");
}

static int32_t nbits[] = {
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

int32_t Util::countBit(uint32_t n) {
  return
    nbits[n&0xffu]+
    nbits[(n >> 8)&0xffu]+
    nbits[(n >> 16)&0xffu]+
    nbits[(n >> 24)&0xffu];
}

string Util::randomAlpha(int32_t length) {
  static char *random_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  string str;
  for(int32_t i = 0; i < length; i++) {
    int32_t index = SimpleRandomizer::getInstance()->getRandomNumber(strlen(random_chars));
    str += random_chars[index];
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

void Util::setGlobalSignalHandler(int32_t sig, void (*handler)(int), int32_t flags) {
#ifdef HAVE_SIGACTION
  struct sigaction sigact;
  sigact.sa_handler = handler;
  sigact.sa_flags = flags;
  sigemptyset(&sigact.sa_mask);
  sigaction(sig, &sigact, NULL);
#else
  signal(sig, handler);
#endif // HAVE_SIGACTION
}

void Util::indexRange(int32_t& startIndex, int32_t& endIndex,
		      int64_t offset, int32_t srcLength, int32_t destLength)
{
  int64_t _startIndex = offset/destLength;
  int64_t _endIndex = (offset+srcLength-1)/destLength;
  assert(_startIndex <= INT32_MAX);
  assert(_endIndex <= INT32_MAX);
  startIndex = _startIndex;
  endIndex = _endIndex;
}

string Util::getHomeDir()
{
  const char* p = getenv("HOME");
  if(p) {
    return p;
  } else {
    return "";
  }
}

int64_t Util::getRealSize(const string& sizeWithUnit)
{
  string::size_type p = sizeWithUnit.find_first_of("KM");
  string size;
  int32_t mult = 1;
  if(p == string::npos) {
    size = sizeWithUnit;
  } else {
    if(sizeWithUnit[p] == 'K') {
      mult = 1024;
    } else if(sizeWithUnit[p] == 'M') {
      mult = 1024*1024;
    }
    size = sizeWithUnit.substr(0, p);
  }
  return strtoll(size.c_str(), 0, 10)*mult;
}

string Util::abbrevSize(int64_t size)
{
  if(size < 1024) {
    return Util::llitos(size, true);
  }
  char units[] = { 'K', 'M' };
  int32_t numUnit = sizeof(units)/sizeof(char);
  int32_t i = 0;
  int32_t r = size&0x3ff;
  size >>= 10;
  for(; i < numUnit-1 && size >= 1024; ++i) {
    r = size&0x3ff;
    size >>= 10;
  }
  return Util::llitos(size, true)+"."+Util::itos(r*10/1024)+units[i]+"i";
}

time_t Util::httpGMT(const string& httpStdTime)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  strptime(httpStdTime.c_str(), "%a, %Y-%m-%d %H:%M:%S GMT", &tm);
  time_t thetime = timegm(&tm);
  return thetime;
}

void Util::toStream(ostream& os, const FileEntries& fileEntries)
{
  os << _("Files:") << "\n";
  os << "idx|path/length" << "\n";
  os << "===+===========================================================================" << "\n";
  int32_t count = 1;
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); count++, itr++) {
    os << setw(3) << count << "|" << (*itr)->getPath() << "\n";
    os << "   |" << Util::llitos((*itr)->getLength(), true) << " bytes" << "\n";
    os << "---+---------------------------------------------------------------------------" << "\n";
  }
}

void Util::sleep(long seconds) {
#ifdef HAVE_SLEEP
  ::sleep(seconds);
#elif defined(HAVE_USLEEP)
  ::usleep(seconds * 1000000);
#elif defined(HAVE_WINSOCK2_H)
  ::Sleep(seconds * 1000);
#else
#error no sleep function is available (nanosleep?)
#endif
}

void Util::usleep(long microseconds) {
#ifdef HAVE_USLEEP
  ::usleep(microseconds);
#elif defined(HAVE_WINSOCK2_H)

	LARGE_INTEGER current, freq, end;

	static enum {GET_FREQUENCY, GET_MICROSECONDS, SKIP_MICROSECONDS} state = GET_FREQUENCY;

	if (state == GET_FREQUENCY) {
		if (QueryPerformanceFrequency(&freq))
			state = GET_MICROSECONDS;
		else
			state = SKIP_MICROSECONDS;
	}
	
	long msec = microseconds / 1000;
	microseconds %= 1000;    

	if (state == GET_MICROSECONDS && microseconds) {
		QueryPerformanceCounter(&end);

		end.QuadPart += (freq.QuadPart * microseconds) / 1000000;

		while (QueryPerformanceCounter(&current) && (current.QuadPart <= end.QuadPart))
			/* noop */ ;
	}

	if (msec)
		Sleep(msec);
#else
	#error no usleep function is available (nanosleep?)
#endif
}

bool Util::isNumber(const string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!isdigit(what[i])) {
      return false;
    }
  }
  return true;
}

bool Util::isLowercase(const string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!('a' <= what[i] && what[i] <= 'z')) {
      return false;
    }
  }
  return true;
}

bool Util::isUppercase(const string& what)
{
  if(what.empty()) {
    return false;
  }
  for(uint32_t i = 0; i < what.size(); ++i) {
    if(!('A' <= what[i] && what[i] <= 'Z')) {
      return false;
    }
  }
  return true;
}

int32_t Util::alphaToNum(const string& alphabets)
{
  if(alphabets.empty()) {
    return 0;
  }
  char base;
  if(islower(alphabets[0])) {
    base = 'a';
  } else {
    base = 'A';
  }
  int32_t num = 0;
  for(uint32_t i = 0; i < alphabets.size(); ++i) {
    int32_t v = alphabets[i]-base;
    num = num*26+v;
  }
  return num;
}

