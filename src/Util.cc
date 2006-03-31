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
#include "Util.h"
#include "DlAbortEx.h"
#include "File.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
  string::size_type sp = src.find_first_not_of(" ");
  string::size_type ep = src.find_last_not_of(" ");
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
  return ((tv1.tv_sec-tv2.tv_sec)*1000000+
	  tv1.tv_usec-tv2.tv_usec);
}

void Util::slice(Strings& result, const string& src, char delim) {
  string::size_type p = 0;
  while(1) {
    string::size_type np = src.find(delim, p);
    if(np == string::npos) {
      string term = trim(src.substr(p));
      if(term.size() > 0) {
	result.push_back(term);
      }
      break;
    }
    string term = src.substr(p, np-p);
    p = np+1;
    result.push_back(trim(term));
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
      throw new DlAbortEx(strerror(errno));
    }
    if((srcFd = open(src.c_str(), O_RDONLY, S_IRUSR|S_IWUSR)) == -1) {
      throw new DlAbortEx(strerror(errno));
    }
    if(lseek(srcFd, srcOffset, SEEK_SET) != srcOffset) {
      throw new DlAbortEx(strerror(errno));
    }
    int x = length/bufSize;
    int r = length%bufSize;
    for(int i = 0; i < x; i++) {
      int readLength;
      if((readLength = read(srcFd, buf, bufSize)) == -1 || readLength != bufSize) {
	throw new DlAbortEx(strerror(errno));
      }
      if(write(destFd, buf, readLength) == -1) {
	throw new DlAbortEx(strerror(errno));
      }
    }
    if(r > 0) {
      int readLength;
      if((readLength = read(srcFd, buf, r)) == -1 || readLength != r) {
	throw new DlAbortEx(strerror(errno));
      }
      if(write(destFd, buf, r) == -1) {
	throw new DlAbortEx(strerror(errno));
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
