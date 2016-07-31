/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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

#ifndef D_MESSAGE_DIGEST_IMPL_H
#define D_MESSAGE_DIGEST_IMPL_H

#include "common.h"

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <tuple>

#include "a2functional.h"

namespace aria2 {

class MessageDigestImpl {
public:
  typedef std::function<std::unique_ptr<MessageDigestImpl>()> factory_t;
  typedef std::tuple<factory_t, size_t> hash_info_t;
  typedef std::map<std::string, hash_info_t> hashes_t;

  template <typename T> inline static hash_info_t make_hi()
  {
    return std::make_tuple([]() { return make_unique<T>(); }, T::length());
  }

private:
  static hashes_t hashes;

  MessageDigestImpl(const MessageDigestImpl&) = delete;
  MessageDigestImpl& operator=(const MessageDigestImpl&) = delete;

public:
  virtual ~MessageDigestImpl() = default;
  static std::unique_ptr<MessageDigestImpl> sha1();

  inline static std::unique_ptr<MessageDigestImpl>
  create(const std::string& hashType)
  {
    auto i = hashes.find(hashType);
    if (i == hashes.end()) {
      return nullptr;
    }
    return std::get<0>(i->second)();
  }

  inline static bool supports(const std::string& hashType)
  {
    auto i = hashes.find(hashType);
    return i != hashes.end();
  }

  inline static size_t getDigestLength(const std::string& hashType)
  {
    auto i = hashes.find(hashType);
    if (i == hashes.end()) {
      return 0;
    }
    return std::get<1>(i->second);
  }

public:
  virtual size_t getDigestLength() const = 0;
  virtual void reset() = 0;
  virtual void update(const void* data, size_t length) = 0;
  virtual void digest(unsigned char* md) = 0;

protected:
  MessageDigestImpl() = default;
};

} // namespace aria2

#endif // D_MESSAGE_DIGEST_IMPL_H
