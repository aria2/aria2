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
  unsigned int sp = src.find_first_not_of(" ");
  unsigned int ep = src.find_last_not_of(" ");
  if(sp == string::npos || ep == string::npos) {
    return "";
  } else {
    return src.substr(sp, ep-sp+1);
  }
}

void Util::split(pair<string, string>& hp, string src, char delim) {
  hp.first = "";
  hp.second = "";
  unsigned int p = src.find(delim);
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
  int p = 0;
  while(1) {
    unsigned int np = src.find(delim, p);
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
