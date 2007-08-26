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
#include "File.h"
#include "DlAbortEx.h"
#include "message.h"
#include "a2io.h"
#include <string.h>

MetaEntry* MetaFileUtil::parseMetaFile(const string& file) {
  File f(file);
  int32_t len = f.size();
  char* buf = new char[len];
  FILE* fp = fopen(file.c_str(), "r+b");
  try {
    if(fp == NULL) {
      throw new DlAbortEx("cannot open metainfo file");
    }
    if(fread(buf, len, 1, fp) != 1) {
      fclose(fp);
      throw new DlAbortEx("cannot read metainfo");
    }
    fclose(fp);
    fp = NULL;
    MetaEntry* entry = bdecoding(buf, len);
    delete [] buf;
    return entry;
  } catch(RecoverableException* ex) {
    delete [] buf;
    if(fp != NULL) {
      fclose(fp);
    }
    throw;
  }
}

MetaEntry* MetaFileUtil::bdecoding(const char* buf, int32_t len) {
  MetaEntry* entry = NULL;
  try{
    const char* p = buf;
    const char* end = buf+len;
    entry = bdecodingR(&p, end);
    return entry;
  } catch(DlAbortEx* ex) {
    if(entry != NULL) {
      delete entry;
    }
    throw;
  }
}

MetaEntry* MetaFileUtil::bdecodingR(const char** pp, const char* end) {
  if(*pp >= end) {
    throw new DlAbortEx("mulformed metainfo");
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

Dictionary* MetaFileUtil::parseDictionaryTree(const char** pp, const char* end) {
  if(*pp >= end) {
    throw new DlAbortEx("mulformed metainfo");
  }
  Dictionary* dic = new Dictionary();
  try {
    while(1) {
      if(**pp == 'e') {
	(*pp)++;
	break;
      }
      string name = decodeWordAsString(pp, end);
      MetaEntry* e = bdecodingR(pp, end);
      dic->put(name, e);
    }
    return dic;
  } catch(RecoverableException* ex) {
    delete dic;
    throw;
  }
}

List* MetaFileUtil::parseListTree(const char** pp, const char* end) {
  if(*pp >= end) {
    throw new DlAbortEx("mulformed metainfo");
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

Data* MetaFileUtil::decodeInt(const char** pp, const char* end) {
  if(*pp >= end) {
    throw new DlAbortEx(EX_MULFORMED_META_INFO);
  }
  char* endTerm = (char*)memchr(*pp, 'e', end-*pp);
  // TODO if endTerm is null
  if(endTerm == NULL) {
    throw new DlAbortEx(EX_MULFORMED_META_INFO);
  }
  int32_t numSize = endTerm-*pp;

  Data* data = new Data(*pp, numSize, true);
  *pp += numSize+1;
  return data;
}

Data* MetaFileUtil::decodeWord(const char** pp, const char* end) {
  if(*pp >= end) {
    throw new DlAbortEx("mulformed metainfo");
  }
  char* delim = (char*)memchr(*pp, ':', end-*pp);
  // TODO if delim is null
  if(delim == *pp || delim == NULL) {
    throw new DlAbortEx(EX_MULFORMED_META_INFO);
  }
  int32_t numSize = delim-*pp;
  char* temp = new char[numSize+1];
  memcpy(temp, *pp, numSize);
  temp[numSize] = '\0';
  char* endptr;
  int32_t size = strtol(temp, &endptr, 10);
  if(*endptr != '\0') {
    delete [] temp;
    throw new DlAbortEx(EX_MULFORMED_META_INFO);
  }    
  delete [] temp;

  if(delim+1+size > end) {
    throw new DlAbortEx(EX_MULFORMED_META_INFO);
  }

  Data* data = new Data(delim+1, size);
  *pp = delim+1+size;
  return data;
}

string MetaFileUtil::decodeWordAsString(const char** pp, const char* end) {
  Data* data = decodeWord(pp, end);
  string str = data->toString();
  delete data;
  return str;
}
