/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#ifndef D_HASH_FUNC_ENTRY_H
#define D_HASH_FUNC_ENTRY_H

#include "common.h"

#include <string>
#include <algorithm>

#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

template<typename HashFunc>
struct HashFuncEntry {
  typedef HashFunc HashFuncType;
  std::string hashType;
  HashFunc hashFunc;
  HashFuncEntry(const std::string& hashType, const HashFunc& hashFunc):
    hashType(hashType), hashFunc(hashFunc)
  {}
};

template<typename HashFunc>
class FindHashFunc {
private:
  const std::string& hashType_;
public:
  FindHashFunc(const std::string& hashType):hashType_(hashType)
  {}

  bool operator()(const HashFuncEntry<HashFunc>& entry) const
  {
    return entry.hashType == hashType_;
  }
};

template<class HashFuncEntry>
const typename HashFuncEntry::HashFuncType& getHashFunc
(HashFuncEntry* first, HashFuncEntry* last, const std::string& hashType)
{
  HashFuncEntry* e =
    std::find_if(first, last,
                 FindHashFunc<typename HashFuncEntry::HashFuncType>
                 (hashType));
  if(e == last) {
    throw DL_ABORT_EX
      (fmt("Hash type %s is not supported.", hashType.c_str()));
  }
  return e->hashFunc;
}

} // namespace aria2

#endif // D_HASH_FUNC_ENTRY_H
