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
#include "Base64.h"

string Base64::part_encode(const string& subplain)
{
  static char base64_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
  };

  int shift = 2;
  char carry = 0;
  bool ignore_flag = false;
  string crypted;

  for(unsigned int index = 0; index < subplain.size(); ++index) {
    if(ignore_flag) {
      crypted += '=';
    } else {
      char cur = subplain.at(index) >> shift | carry;
      if(subplain.at(index) == 0) ignore_flag = true;
      carry = (subplain.at(index) << (6-shift)) & 0x3F;
      shift += 2;
      crypted += base64_table[(unsigned int)cur];
    }
  }
  if(subplain.at(subplain.size()-1) == 0) {
    crypted += '=';
  } else {
    char cur = subplain.at(subplain.size()-1) & 0x3F;
    crypted += base64_table[(unsigned int)cur];
  }

  return crypted;
}

string Base64::encode(const string& plainSrc)
{
  string plain = plainSrc;
  int remainder = plain.size() % 3;
  if( remainder ) remainder = 3-remainder;
  for(int i = 0; i < remainder; ++i) plain += (char)0;
  string crypted;
  int start_pos = 0;
  for(unsigned int index = 0; plain.size() > index; index += 3) {
    string subplain = plain.substr(start_pos, 3);
    string subcrypted = part_encode(subplain);
    start_pos += 3;
    crypted += subcrypted;
  }
  return crypted;
}

char Base64::getValue(char ch)
{
  char retch;

  if(ch >= 'A' && ch <= 'Z') {
    retch = ch-'A';
  } else if(ch >= 'a' && ch <= 'z') {
    retch = ch-'a'+26;
  } else if(ch >= '0' && ch <= '9') {
    retch = ch-'0'+52;
  } else if(ch == '+') {
    retch = 62;
  } else if(ch == '/') {
    retch = 63;
  } else {
    retch = 0;
  }
  return retch;
}

string Base64::part_decode(const string& subCrypted)
{
  int shift = 2;
  string plain;

  for(unsigned int index = 0; index < subCrypted.size()-1; ++index) {
    if(subCrypted.at(index) == '=') break;
    char cur = getValue(subCrypted.at(index)) << shift;
    char carry = getValue(subCrypted.at(index+1)) >> (6-shift);
    shift += 2;
    plain += cur | carry;
  }

  return plain;
}

string Base64::decode(const string& crypted)
{
  string plain;
  int sIndex = 0;
  for(int index = 0; crypted.size() > (unsigned int)index; index +=4) {
    string subCrypted = crypted.substr(sIndex, 4);
    string subPlain = part_decode(subCrypted);
    sIndex += 4;
    plain += subPlain;
  }
  return plain;
}
