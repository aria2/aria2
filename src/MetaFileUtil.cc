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
#include "MetaFileUtil.h"
#include "Data.h"
#include "Dictionary.h"
#include "List.h"
#include "File.h"
#include "DlAbortEx.h"
#include "message.h"
#include <cstring>
#include <cstdlib> // <-- TODO remove this if strtoul is replaced with Util::parseUInt()

namespace aria2 {

MetaEntry* MetaFileUtil::parseMetaFile(const std::string& file) {
  File f(file);
  size_t len = f.size();
  unsigned char* buf = new unsigned char[len];
  FILE* fp = fopen(file.c_str(), "r+b");
  try {
    if(!fp) {
      throw new DlAbortEx("cannot open metainfo file");
    }
    if(fread(buf, len, 1, fp) != 1) {
      fclose(fp);
      throw new DlAbortEx("cannot read metainfo");
    }
    fclose(fp);
    fp = 0;
    MetaEntry* entry = bdecoding(buf, len);
    delete [] buf;
    return entry;
  } catch(RecoverableException* ex) {
    delete [] buf;
    if(fp) {
      fclose(fp);
    }
    throw;
  }
}

MetaEntry* MetaFileUtil::bdecoding(const unsigned char* buf, size_t len)
{
  const unsigned char* p = buf;
  const unsigned char* end = buf+len;
  return bdecodingR(&p, end);
}

MetaEntry*
MetaFileUtil::bdecodingR(const unsigned char** pp, const unsigned char* end)
{
  if(*pp >= end) {
    throw new DlAbortEx("Malformed metainfo");
  }
  MetaEntry* e;
  switch(**pp) {
  case 'd':
    (*pp)++;
    e = parseDictionaryTree(pp, end);
    break;
  case 'l':
    (*pp)++;
    e = parseListTree(pp, end);
    break;
  case 'i':
    (*pp)++;
    e = decodeInt(pp, end);
    break;
  default:
    e = decodeWord(pp, end);
  }
  return e;
}

Dictionary*
MetaFileUtil::parseDictionaryTree(const unsigned char** pp, const unsigned char* end)
{
  if(*pp >= end) {
    throw new DlAbortEx("Malformed metainfo");
  }
  Dictionary* dic = new Dictionary();
  try {
    while(1) {
      if(**pp == 'e') {
	(*pp)++;
	break;
      }
      std::string name = decodeWordAsString(pp, end);
      MetaEntry* e = bdecodingR(pp, end);
      dic->put(name, e);
    }
    return dic;
  } catch(RecoverableException* ex) {
    delete dic;
    throw;
  }
}

List*
MetaFileUtil::parseListTree(const unsigned char** pp, const unsigned char* end)
{
  if(*pp >= end) {
    throw new DlAbortEx("Malformed metainfo");
  }
  List* lis = new List();
  try {
    while(1) {
      if(**pp == 'e') {
	(*pp)++;
	break;
      }
      MetaEntry* e = bdecodingR(pp, end);
      lis->add(e);
    }
    return lis;
  } catch(RecoverableException* ex) {
    delete lis;
    throw;
  }
}

Data*
MetaFileUtil::decodeInt(const unsigned char** pp, const unsigned char* end)
{
  if(*pp >= end) {
    throw new DlAbortEx(EX_MALFORMED_META_INFO);
  }
  unsigned char* endTerm = reinterpret_cast<unsigned char*>(memchr(*pp, 'e', end-*pp));
  // TODO if endTerm is null
  if(!endTerm) {
    throw new DlAbortEx(EX_MALFORMED_META_INFO);
  }
  size_t numSize = endTerm-*pp;

  Data* data = new Data(*pp, numSize, true);
  *pp += numSize+1;
  return data;
}

Data*
MetaFileUtil::decodeWord(const unsigned char** pp, const unsigned char* end)
{
  if(*pp >= end) {
    throw new DlAbortEx("Malformed metainfo");
  }
  unsigned char* delim = reinterpret_cast<unsigned char*>(memchr(*pp, ':', end-*pp));
  // TODO if delim is null
  if(delim == *pp || !delim) {
    throw new DlAbortEx(EX_MALFORMED_META_INFO);
  }
  size_t numSize = delim-*pp;
  unsigned char* temp = new unsigned char[numSize+1];
  memcpy(temp, *pp, numSize);
  temp[numSize] = '\0';
  char* endptr;
  unsigned long int size = strtoul(reinterpret_cast<const char*>(temp),
				   &endptr, 10);
  if(*endptr != '\0') {
    delete [] temp;
    throw new DlAbortEx(EX_MALFORMED_META_INFO);
  }    
  delete [] temp;

  if(delim+1+size > end) {
    throw new DlAbortEx(EX_MALFORMED_META_INFO);
  }

  Data* data = new Data(delim+1, size);
  *pp = delim+1+size;
  return data;
}

std::string
MetaFileUtil::decodeWordAsString(const unsigned char** pp, const unsigned char* end)
{
  Data* data = decodeWord(pp, end);
  std::string str = data->toString();
  delete data;
  return str;
}

} // namespace aria2
