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
#ifndef _D_MESSAGE_DIGEST_H_
#define _D_MESSAGE_DIGEST_H_

#include "common.h"
#include "SharedHandle.h"
#include "DlAbortEx.h"
#include "StringFormat.h"
#include <map>

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
  typedef std::map<std::string, MessageDigestContext::DigestAlgo> DigestAlgoMap;

  static const std::string SHA1;

  static const std::string SHA256;

  static const std::string MD5;
private:
#ifdef HAVE_LIBSSL
  EVP_MD_CTX ctx;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
  gcry_md_hd_t ctx;
#endif // HAVE_LIBGCRYPT  
  DigestAlgo algo;

  static DigestAlgoMap digestAlgos;
public:
  MessageDigestContext():algo(getDigestAlgo(MessageDigestContext::SHA1))
  {}

  ~MessageDigestContext()
  {
    digestFree();
  }

  void trySetAlgo(const std::string& algostring)
  {
    algo = getDigestAlgo(algostring);
  }

  static bool supports(const std::string& algostring)
  {
    DigestAlgoMap::const_iterator itr = digestAlgos.find(algostring);
    if(itr == digestAlgos.end()) {
      return false;
    } else {
      return true;
    }
  }

  static DigestAlgo getDigestAlgo(const std::string& algostring)
  {
    DigestAlgoMap::const_iterator itr = digestAlgos.find(algostring);
    if(itr == digestAlgos.end()) {
      throw DL_ABORT_EX
	(StringFormat("Digest algorithm %s is not supported.",
		      algostring.c_str()).str());
    }
    return (*itr).second;
  }

  static std::string getSupportedAlgoString();

  static size_t digestLength(const std::string& algostring)
  {
    return digestLength(getDigestAlgo(algostring));
  }

  std::string digestFinal();

#if defined(HAVE_OLD_LIBSSL)
  void digestInit() {EVP_DigestInit(&ctx, algo);}
  void digestReset() {EVP_DigestInit(&ctx, algo);}
  void digestUpdate(const void* data, size_t length) {EVP_DigestUpdate(&ctx, data, length);}
  void digestFinal(unsigned char* md) {
    unsigned int len;
    EVP_DigestFinal(&ctx, md, &len);
  }
  void digestFree() {/*empty*/}
  size_t digestLength() const {
    return digestLength(algo);
  }
  static size_t digestLength(DigestAlgo algo) {
    return EVP_MD_size(algo);
  }

#elif defined(HAVE_LIBSSL)
  void digestInit() {
    EVP_MD_CTX_init(&ctx);
    digestReset();
  }
  void digestReset() {
    EVP_DigestInit_ex(&ctx, algo, 0);
  }
  void digestUpdate(const void* data, size_t length) {
    EVP_DigestUpdate(&ctx, data, length);
  }
  void digestFinal(unsigned char* md) {
    unsigned int len;
    EVP_DigestFinal_ex(&ctx, md, &len);
  }
  void digestFree() {
    EVP_MD_CTX_cleanup(&ctx);
  }
  size_t digestLength() const {
    return digestLength(algo);
  }
  static size_t digestLength(DigestAlgo algo) {
    return EVP_MD_size(algo);
  }

#elif defined(HAVE_LIBGCRYPT)
  void digestInit() {
    gcry_md_open(&ctx, algo, 0);
  }
  void digestReset() {
    gcry_md_reset(ctx);
  }
  void digestUpdate(const void* data, size_t length) {
    gcry_md_write(ctx, data, length);
  }
  void digestFinal(unsigned char* md) {
    gcry_md_final(ctx);
    memcpy(md, gcry_md_read(ctx, 0), gcry_md_get_algo_dlen(algo));
  }
  void digestFree() {
    gcry_md_close(ctx);
  }
  size_t digestLength() const {
    return digestLength(algo);
  }
  static size_t digestLength(DigestAlgo algo) {
    return gcry_md_get_algo_dlen(algo);
  }
#endif // HAVE_LIBGCRYPT
};
typedef SharedHandle<MessageDigestContext> MessageDigestContextHandle;

} // namespace aria2

#endif // _D_MESSAGE_DIGEST_H_
