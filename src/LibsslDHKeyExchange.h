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
#ifndef _D_LIBSSL_DH_KEY_EXCHANGE_H_
#define _D_LIBSSL_DH_KEY_EXCHANGE_H_

#include "common.h"
#include "DlAbortEx.h"
#include <openssl/dh.h>
#include <openssl/rand.h>
#include <openssl/err.h>

namespace aria2 {

class DHKeyExchange {
private:

  BN_CTX* _bnCtx;

  BIGNUM* _prime;

  BIGNUM* _generator;

  BIGNUM* _privateKey;

  BIGNUM* _publicKey;

  void handleError() const
  {
    throw new DlAbortEx("Exception in libssl routine(DHKeyExchange class): %s",
			ERR_error_string(ERR_get_error(), 0));
  }

public:
  DHKeyExchange():_bnCtx(0),
		  _prime(0),
		  _generator(0),
		  _privateKey(0),
		  _publicKey(0) {}

  ~DHKeyExchange()
  {
    BN_CTX_free(_bnCtx);
    BN_free(_prime);
    BN_free(_generator);
    BN_free(_privateKey);
    BN_free(_publicKey);
  }

  void init(const unsigned char* prime, const unsigned char* generator,
	    size_t privateKeyBits)
  {
    BN_CTX_free(_bnCtx);
    _bnCtx = BN_CTX_new();
    if(!_bnCtx) {
      handleError();
    }

    BN_free(_prime);
    _prime = 0;
    BN_free(_generator);
    _generator = 0;
    BN_free(_privateKey);
    _privateKey = 0;

    if(BN_hex2bn(&_prime, reinterpret_cast<const char*>(prime)) == 0) {
      handleError();
    }
    if(BN_hex2bn(&_generator, reinterpret_cast<const char*>(generator)) == 0) {
      handleError();
    }
    _privateKey = BN_new();
    if(!BN_rand(_privateKey, privateKeyBits, -1, false)) {
      handleError();
    }
  }

  void generatePublicKey()
  {
    BN_free(_publicKey);
    _publicKey = BN_new();
    BN_mod_exp(_publicKey, _generator, _privateKey, _prime, _bnCtx);
  }

  size_t publicKeyLength() const
  {
    return BN_num_bytes(_publicKey);
  }

  size_t getPublicKey(unsigned char* out, size_t outLength) const
  {
    if(outLength < publicKeyLength()) {
      throw new DlAbortEx("Insufficient buffer for public key. expect:%u, actual:%u",
			  publicKeyLength(), outLength);
    }
    size_t nwritten = BN_bn2bin(_publicKey, out);
    if(!nwritten) {
      handleError();
    }
    return nwritten;
  }

  void generateNonce(unsigned char* out, size_t outLength) const
  {
    if(!RAND_bytes(out, outLength)) {
      handleError();
    }
  }

  size_t computeSecret(unsigned char* out, size_t outLength,
		       const unsigned char* peerPublicKeyData,
		       size_t peerPublicKeyLength) const
  {
    if(outLength < publicKeyLength()) {
      throw new DlAbortEx("Insufficient buffer for secret. expect:%u, actual:%u",
			  publicKeyLength(), outLength);
    }


    BIGNUM* peerPublicKey = BN_bin2bn(peerPublicKeyData, peerPublicKeyLength, 0);
    if(!peerPublicKey) {
      handleError();
    }

    BIGNUM* secret = BN_new();
    BN_mod_exp(secret, peerPublicKey, _privateKey, _prime, _bnCtx);
    BN_free(peerPublicKey);

    size_t nwritten = BN_bn2bin(secret, out);
    BN_free(secret);
    if(!nwritten) {
      handleError();
    }
    return nwritten;
  }
};

} // namespace aria2

#endif // _D_LIBSSL_DH_KEY_EXCHANGE_H_
