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
#ifndef _D_LIBGCRYPT_DH_KEY_EXCHANGE_H_
#define _D_LIBGCRYPT_DH_KEY_EXCHANGE_H_

#include "common.h"
#include "DlAbortEx.h"
#include <gcrypt.h>

namespace aria2 {

class DHKeyExchange {
private:
  size_t _keyLength;

  gcry_mpi_t _prime;

  gcry_mpi_t _generator;

  gcry_mpi_t _privateKey;

  gcry_mpi_t _publicKey;

  void handleError(int errno) const
  {
    throw new DlAbortEx("Exception in libgcrypt routine(DHKeyExchange class): %s",
			gcry_strerror(errno));
  }
public:
  DHKeyExchange():
    _keyLength(0),
    _prime(0),
    _generator(0),
    _privateKey(0),
    _publicKey(0) {}

  ~DHKeyExchange()
  {
    gcry_mpi_release(_prime);
    gcry_mpi_release(_generator);
    gcry_mpi_release(_privateKey);
    gcry_mpi_release(_publicKey);
  }

  void init(const unsigned char* prime, size_t primeBits,
	    const unsigned char* generator,
	    size_t privateKeyBits)
  {
    gcry_mpi_release(_prime);
    gcry_mpi_release(_generator);
    gcry_mpi_release(_privateKey);
    {
      gcry_error_t r = gcry_mpi_scan(&_prime, GCRYMPI_FMT_HEX,
				     prime, 0, 0);
      if(r) {
	handleError(r);
      }
    }
    {
      gcry_error_t r = gcry_mpi_scan(&_generator, GCRYMPI_FMT_HEX,
				     generator, 0, 0);
      if(r) {
	handleError(r);
      }
    }
    _privateKey = gcry_mpi_new(0);
    gcry_mpi_randomize(_privateKey, privateKeyBits, GCRY_STRONG_RANDOM);
    _keyLength = (primeBits+7)/8;
  }

  void generatePublicKey()
  {
    gcry_mpi_release(_publicKey);
    _publicKey = gcry_mpi_new(0);
    gcry_mpi_powm(_publicKey, _generator, _privateKey, _prime);
  }

  size_t getPublicKey(unsigned char* out, size_t outLength) const
  {
    if(outLength < _keyLength) {
      throw new DlAbortEx("Insufficient buffer for public key. expect:%u, actual:%u",
			  _keyLength, outLength);
    }
    memset(out, 0, outLength);
    size_t publicKeyBytes = (gcry_mpi_get_nbits(_publicKey)+7)/8;
    size_t offset = _keyLength-publicKeyBytes;
    size_t nwritten;
    gcry_error_t r = gcry_mpi_print(GCRYMPI_FMT_USG, out+offset,
				    outLength-offset, &nwritten, _publicKey);
    if(r) {
      handleError(r);
    }
    return nwritten;
  }

  void generateNonce(unsigned char* out, size_t outLength) const
  {
    gcry_create_nonce(out, outLength);
  }

  size_t computeSecret(unsigned char* out, size_t outLength,
		       const unsigned char* peerPublicKeyData,
		       size_t peerPublicKeyLength) const
  {
    if(outLength < _keyLength) {
      throw new DlAbortEx("Insufficient buffer for secret. expect:%u, actual:%u",
			  _keyLength, outLength);
    }
    gcry_mpi_t peerPublicKey;
    {
      gcry_error_t r = gcry_mpi_scan(&peerPublicKey, GCRYMPI_FMT_USG, peerPublicKeyData,
				     peerPublicKeyLength, 0);
      if(r) {
	handleError(r);
      }
    }
    gcry_mpi_t secret = gcry_mpi_new(0);
    gcry_mpi_powm(secret, peerPublicKey, _privateKey, _prime);
    gcry_mpi_release(peerPublicKey);

    memset(out, 0, outLength);
    size_t secretBytes = (gcry_mpi_get_nbits(secret)+7)/8;
    size_t offset = _keyLength-secretBytes;
    size_t nwritten;
    {
      gcry_error_t r = gcry_mpi_print(GCRYMPI_FMT_USG, out+offset,
				      outLength-offset, &nwritten, secret);
      gcry_mpi_release(secret);
      if(r) {
	handleError(r);
      }
    }
    return nwritten;
  }
};

} // namespace aria2

#endif // _D_LIBGCRYPT_DH_KEY_EXCHANGE_H_
