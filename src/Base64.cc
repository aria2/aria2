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
#include "Base64.h"

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

void Base64::part_encode(const unsigned char* sub, int32_t subLength,
			 unsigned char* buf)
{
  int32_t shift = 2;
  unsigned char carry = 0;
  int32_t index;
  for(index = 0; index < subLength; index++) {
    unsigned char cur = sub[index] >> shift | carry;
    carry = (sub[index] << (6-shift)) & 0x3f;
    shift += 2;
    buf[index] = base64_table[(uint32_t)cur];
  }
  if(subLength == 1) {
    buf[index] = base64_table[(uint32_t)carry];
    buf[index+1] = buf[index+2] = '=';
  } else if(subLength == 2) {
    buf[index] = base64_table[(uint32_t)carry];
    buf[index+1] = '=';
  } else {
    unsigned char cur = sub[subLength-1] & 0x3f;
    buf[index] = base64_table[(uint32_t)cur];
  }
}

string Base64::encode(const string& plainSrc)
{
  unsigned char* result = 0;
  int32_t resultLength = 0;

  encode((const unsigned char*)plainSrc.c_str(), plainSrc.size(),
	 result, resultLength);
  string encoded(&result[0], &result[resultLength]);
  delete [] result;
  return encoded;
}

void Base64::encode(const unsigned char* src, int32_t srcLength,
		    unsigned char*& result, int32_t& resultLength) {
  resultLength = (srcLength+(srcLength%3 == 0 ? 0 : 3-srcLength%3))/3*4;
  result = new unsigned char[resultLength];
  unsigned char* tail = result;
  for(int32_t index = 0; srcLength > index; index += 3) {
    unsigned char temp[4];
    part_encode(&src[index],
		srcLength >= index+3 ? 3 : srcLength-index,
		temp);
    memcpy(tail, temp, sizeof(temp)); 
    tail += sizeof(temp);
  }
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
  int32_t shift = 2;
  string plain;

  for(uint32_t index = 0; index < subCrypted.size()-1; ++index) {
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
  int32_t sIndex = 0;
  for(int32_t index = 0; crypted.size() > (uint32_t)index; index +=4) {
    string subCrypted = crypted.substr(sIndex, 4);
    string subPlain = part_decode(subCrypted);
    sIndex += 4;
    plain += subPlain;
  }
  return plain;
}
