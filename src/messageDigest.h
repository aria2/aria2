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
#ifndef _D_MESSAGE_DIGEST_H_
#define _D_MESSAGE_DIGEST_H_

#include "common.h"

#include <cstring>
#include <map>

#include "SharedHandle.h"
#include "DlAbortEx.h"
#include "StringFormat.h"

#ifdef HAVE_LIBSSL
#include <openssl/evp.h>
#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGCRYPT
#include <gcrypt.h>
#endif // HAVE_LIBGCRYPT

namespace aria2 {

class MessageDigestContext {
public:
#ifdef HAVE_LIBSSL
  typedef const EVP_MD* DigestAlgo;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
  typedef int DigestAlgo;
#endif // HAVE_LIBGCRYPT

  static const std::string SHA1;

  static const std::string SHA256;

  static const std::string MD5;
private:
#ifdef HAVE_LIBSSL
  EVP_MD_CTX ctx_;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
  gcry_md_hd_t ctx_;
#endif // HAVE_LIBGCRYPT  
  DigestAlgo algo_;
public:
  MessageDigestContext():algo_(getDigestAlgo(MessageDigestContext::SHA1))
  {}

  ~MessageDigestContext()
  {
    digestFree();
  }

  void trySetAlgo(const std::string& algostring)
  {
    algo_ = getDigestAlgo(algostring);
  }

  static bool supports(const std::string& algostring);

  static DigestAlgo getDigestAlgo(const std::string& algostring);

  static std::string getSupportedAlgoString();

  static size_t digestLength(const std::string& algostring)
  {
    return digestLength(getDigestAlgo(algostring));
  }

  // Returns true if hash algorithm specified by lhs is stronger than
  // the one specified by rhs. If either lhs or rhs is not supported
  // or both are not supported, returns false.
  static bool isStronger(const std::string& lhs, const std::string& rhs);

  static bool isValidHash
  (const std::string& algostring, const std::string& hashstring);

  // Returns canonical hash algorithm name of given algostring.  If
  // given algostring is not supported, then returns algostring
  // unchanged.
  static std::string getCanonicalAlgo(const std::string& algostring);

  std::string digestFinal();

#if defined(HAVE_OLD_LIBSSL)
  void digestInit()
  {
    EVP_DigestInit(&ctx_, algo_);
  }

  void digestReset()
  {
    EVP_DigestInit(&ctx_, algo_);
  }

  void digestUpdate(const void* data, size_t length)
  {
    EVP_DigestUpdate(&ctx_, data, length);
  }

  void digestFinal(unsigned char* md) {
    unsigned int len;
    EVP_DigestFinal(&ctx_, md, &len);
  }
  void digestFree() {/*empty*/}
  size_t digestLength() const {
    return digestLength(algo_);
  }
  static size_t digestLength(DigestAlgo algo) {
    return EVP_MD_size(algo);
  }

#elif defined(HAVE_LIBSSL)
  void digestInit() {
    EVP_MD_CTX_init(&ctx_);
    digestReset();
  }
  void digestReset() {
    EVP_DigestInit_ex(&ctx_, algo_, 0);
  }
  void digestUpdate(const void* data, size_t length) {
    EVP_DigestUpdate(&ctx_, data, length);
  }
  void digestFinal(unsigned char* md) {
    unsigned int len;
    EVP_DigestFinal_ex(&ctx_, md, &len);
  }
  void digestFree() {
    EVP_MD_CTX_cleanup(&ctx_);
  }
  size_t digestLength() const {
    return digestLength(algo_);
  }
  static size_t digestLength(DigestAlgo algo) {
    return EVP_MD_size(algo);
  }

#elif defined(HAVE_LIBGCRYPT)
  void digestInit() {
    gcry_md_open(&ctx_, algo_, 0);
  }
  void digestReset() {
    gcry_md_reset(ctx_);
  }
  void digestUpdate(const void* data, size_t length) {
    gcry_md_write(ctx_, data, length);
  }
  void digestFinal(unsigned char* md) {
    gcry_md_final(ctx_);
    memcpy(md, gcry_md_read(ctx_, 0), gcry_md_get_algo_dlen(algo_));
  }
  void digestFree() {
    gcry_md_close(ctx_);
  }
  size_t digestLength() const {
    return digestLength(algo_);
  }
  static size_t digestLength(DigestAlgo algo) {
    return gcry_md_get_algo_dlen(algo);
  }
#endif // HAVE_LIBGCRYPT
};
typedef SharedHandle<MessageDigestContext> MessageDigestContextHandle;

} // namespace aria2

#endif // _D_MESSAGE_DIGEST_H_
