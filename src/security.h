/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2014 Nils Maier
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

#ifndef D_SECURITY_H
#define D_SECURITY_H

#include <string>

#include "common.h"
#include "a2functional.h"
#include "MessageDigest.h"

namespace aria2 {
namespace util {
namespace security {

/**
 * Compare to bytes in constant time.
 *
 * @param a First byte.
 * @param b Second byte.
 * @return True, if both match, false otherwise.
 */
bool compare(const uint8_t a, const uint8_t b);

/**
 * Compare two byte arrays in constant time. The arrays must have the same
 * length!
 *
 * @param a First byte array.
 * @param b Second byte array.
 * @return True, if both match, false otherwise.
 */
bool compare(const uint8_t *a, const uint8_t *b, size_t length);
inline bool compare(const char *a, const char *b, size_t length)
{
  return compare(
      reinterpret_cast<const uint8_t*>(a),
      reinterpret_cast<const uint8_t*>(b),
      length * sizeof(char)
      );
}

class HMACResult {
public:
  HMACResult(const std::string& result)
    : result_(result), len_(result.length())
  {}

  HMACResult(const char* result, size_t length)
    : result_(result, length), len_(length)
  {}

  HMACResult(const HMACResult& other) :
    result_(other.result_), len_(other.len_)
  {}

  HMACResult& operator=(const HMACResult& other) {
    result_ = other.result_;
    len_ = other.len_;
    return *this;
  }

  bool operator == (const HMACResult& other) const
  {
    if (len_ != other.len_) {
      throw std::domain_error("comparing different hmac is undefined");
    }
    return compare(result_.data(), other.result_.data(), len_);
  }

  bool operator != (const HMACResult& other) const
  {
    return !(*this == other);
  }

  size_t length() const
  {
    return len_;
  }

  const std::string& getBytes() const
  {
    return result_;
  }

private:
  std::string result_;
  size_t len_;
};

// See RFC 6234
class HMAC {
public:
  HMAC(const std::string& algorithm, const char* secret, size_t length);

  static std::unique_ptr<HMAC> create(
      const std::string& algorithm, const std::string& secret)
  {
    return create(algorithm, secret.data(), secret.length());
  }
  static std::unique_ptr<HMAC> create(
      const std::string& algorithm, const char* secret, size_t length)
  {
    if (!supports(algorithm)) {
      return nullptr;
    }
    return make_unique<HMAC>(algorithm, secret, length);
  }

  static std::unique_ptr<HMAC> create(const std::string& secret)
  {
    return create("sha-1", secret.data(), secret.length());
  }
  static std::unique_ptr<HMAC> create(const char* secret, size_t length)
  {
    return create("sha-1", secret, length);
  }

  // Create a Hmac with random secret.
  static std::unique_ptr<HMAC> createRandom(const std::string& algorithm);
  static std::unique_ptr<HMAC> createRandom()
  {
    return createRandom("sha-1");
  }

  static bool supports(const std::string& algorithm);

  size_t length() const
  {
    return md_->getDigestLength();
  }

  void reset()
  {
    if (clean_) {
      return;
    }
    md_->reset();
    md_->update(ipad_.data(), ipad_.length());
    clean_ = true;
  }

  void update(const std::string& data)
  {
    md_->update(data.data(), data.length());
    clean_ = false;
  }
  void update(const char* data, size_t length)
  {
    md_->update(data, length);
    clean_ = false;
  }

  HMACResult getResult()
  {
    auto rv = md_->digest();
    md_->reset();
    md_->update(opad_.data(), opad_.length());
    md_->update(rv.data(), rv.length());
    rv = md_->digest();
    clean_ = false;
    reset();
    return HMACResult(rv);
  }

  HMACResult getResult(const std::string& str)
  {
    reset();
    update(str);
    return getResult();
  }

  HMACResult getResult(const char* data, size_t len)
  {
    reset();
    update(data, len);
    return getResult();
  }

private:
  const size_t blockSize_;
  std::unique_ptr<MessageDigest> md_;
  std::string ipad_, opad_;
  bool clean_;
};

} // namespace security
} // namespace util
} // namespace aria2

#endif // D_SECURITY_H
