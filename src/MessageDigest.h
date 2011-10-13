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
#ifndef D_MESSAGE_DIGEST_H
#define D_MESSAGE_DIGEST_H

#include "common.h"

#include <string>

#include "SharedHandle.h"

namespace aria2 {

class MessageDigestImpl;

class MessageDigest {
private:
  SharedHandle<MessageDigestImpl> pImpl_;

  MessageDigest();

  // We don't implement copy ctor.
  MessageDigest(const MessageDigest&);
  // We don't implement assignment operator.
  MessageDigest& operator=(const MessageDigest&);
public:
  ~MessageDigest();

  // Factory functions
  static SharedHandle<MessageDigest> sha1();

  // Factory function which takes hashType as string.  Throws
  // exception if hashType is not supported.
  static SharedHandle<MessageDigest> create(const std::string& hashType);

  // Returns true if hashType is supported. Otherwise returns false.
  static bool supports(const std::string& hashType);

  // Returns string containing supported hash function textual names
  // joined with ','.
  static std::string getSupportedHashTypeString();

  // Returns the number of bytes needed to store digest for hashType.
  static size_t getDigestLength(const std::string& hashType);

  // Returns true if hash type specified by lhs is stronger than the
  // one specified by rhs. Returns false if at least one of lhs and
  // rhs are not supported. Otherwise returns false.
  static bool isStronger(const std::string& lhs, const std::string& rhs);

  static bool isValidHash
  (const std::string& hashType, const std::string& hexDigest);

  // Returns canonical hash algorithm name of given algostring.  If
  // given algostring is not supported, then returns algostring
  // unchanged.
  static std::string getCanonicalHashType(const std::string& hashType);

  size_t getDigestLength() const;

  // Resets this object so that it can be reused.
  void reset();

  void update(const void* data, size_t length);

  // Stores digest in the region pointed by md. It is caller's
  // responsibility to allocate memory at least getDigestLength().
  // This call can only be called once. To reuse this object, call
  // reset().
  void digest(unsigned char* md);

  // Returns raw digest, not hex digest.  This call can only be called
  // once. To reuse this object, call reset().
  std::string digest();
};

} // namespace aria2

#endif // D_MESSAGE_DIGEST_H
