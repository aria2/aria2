/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_MESSAGE_DIGEST_H_
#define _D_MESSAGE_DIGEST_H_

#include "common.h"

#ifdef ENABLE_SSL

#ifdef HAVE_LIBSSL
#include <openssl/evp.h>
#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGCRYPT
#include <gcrypt.h>
#endif // HAVE_LIBGCRYPT

class MessageDigestContext {
public:
  enum HashAlgo {
    ALGO_MD5,
    ALGO_SHA1
  };
#ifdef HAVE_LIBSSL
  EVP_MD_CTX ctx;
  const EVP_MD* algo;
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
  gcry_md_hd_t ctx;
  int algo;
#endif // HAVE_LIBGCRYPT

  MessageDigestContext() {}
  MessageDigestContext(HashAlgo algo) {
    setAlgo(algo);
  }

  void setAlgo(HashAlgo algo) {
    switch(algo) {
    case ALGO_MD5:
#ifdef HAVE_LIBSSL
      this->algo = EVP_md5();
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
      this->algo = GCRY_MD_MD5;
#endif // HAVE_LIBGCRYPT
      break;
    case ALGO_SHA1:
#ifdef HAVE_LIBSSL
      this->algo = EVP_sha1();
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGCRYPT
      this->algo = GCRY_MD_SHA1;
#endif // HAVE_LIBGCRYPT
      break;
    default:
      break;
    }
  }
};

#ifdef HAVE_LIBSSL
#define digestInit(CTX) EVP_MD_CTX_init(&CTX.ctx)
#define digestReset(CTX) EVP_DigestInit_ex(&CTX.ctx, CTX.algo, NULL)
#define digestUpdate(CTX, DATA, LENGTH) EVP_DigestUpdate(&CTX.ctx, DATA, LENGTH)
#define digestFinal(CTX, HASH) \
{\
int len;\
EVP_DigestFinal_ex(&CTX.ctx, HASH, (unsigned int*)&len);\
}
#define digestFree(CTX) EVP_MD_CTX_cleanup(&CTX.ctx)

#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGCRYPT
#define digestInit(CTX) gcry_md_open(&CTX.ctx, CTX.algo, 0)
#define digestReset(CTX) gcry_md_reset(CTX.ctx)
#define digestUpdate(CTX, DATA, LENGTH) gcry_md_write(CTX.ctx, DATA, LENGTH)
#define digestFinal(CTX, HASH) \
{\
gcry_md_final(CTX.ctx);\
memcpy(HASH, gcry_md_read(CTX.ctx, 0), gcry_md_get_algo_dlen(CTX.algo));\
}
#define digestFree(CTX) gcry_md_close(CTX.ctx)
#endif // HAVE_LIBGCRYPT

#endif // ENABLE_SSL

#endif // _D_MESSAGE_DIGEST_H_
