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

string Util::trim(string src) {
  string::size_type sp = src.find_first_not_of(" ");
  string::size_type ep = src.find_last_not_of(" ");
  if(sp == string::npos || ep == string::npos) {
    return "";
  } else {
    return src.substr(sp, ep-sp+1);
  }
}

void Util::split(pair<string, string>& hp, string src, char delim) {
  hp.first = "";
  hp.second = "";
  string::size_type p = src.find(delim);
  if(p == string::npos) {
    hp.first = src;
    hp.second = "";
  } else {
    hp.first = trim(src.substr(0, p));
    hp.second = trim(src.substr(p+1));
    /*
    unsigned int p2 = src.find_first_not_of(" ", p+1);
    if(p2 == string::npos) {
      hp.second = "";
    } else {
      hp.second = src.substr(p2);
    }
    */
  }
}

long long int Util::difftv(struct timeval tv1, struct timeval tv2) {
  if(tv1.tv_sec < tv2.tv_sec || tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec) {
    return 0;
  }
  return ((tv1.tv_sec-tv2.tv_sec)*1000000+
	  tv1.tv_usec-tv2.tv_usec);
}

void Util::slice(vector<string>& result, string src, char delim) {
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

bool Util::endsWith(string target, string part) {
  if(target.size() < part.size()) {
    return false;
  }
  if(target.compare(target.size()-part.size(), part.size(), part, 0, part.size()) == 0) {
    return true;
  } else {
    return false;
  }
}

string Util::replace(string target, string oldstr, string newstr) {
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

