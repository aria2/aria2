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

#ifdef ENABLE_BITTORRENT

#ifdef HAVE_LIBSSL
#include <openssl/evp.h>
#define MessageDigestContext EVP_MD_CTX
#define sha1DigestInit(CTX) EVP_MD_CTX_init(&CTX)
#define sha1DigestReset(CTX) EVP_DigestInit_ex(&CTX, EVP_sha1(), NULL)
#define sha1DigestUpdate(CTX, DATA, LENGTH) EVP_DigestUpdate(&CTX, DATA, LENGTH)
#define sha1DigestFinal(CTX, HASH) \
{\
int len;\
EVP_DigestFinal_ex(&CTX, HASH, (unsigned int*)&len);\
}
#define sha1DigestFree(CTX) EVP_MD_CTX_cleanup(&CTX)
#endif // HAVE_LIBSSL

#ifdef HAVE_LIBGCRYPT
#include <gcrypt.h>
#define MessageDigestContext gcry_md_hd_t
#define sha1DigestInit(CTX) gcry_md_open(&CTX, GCRY_MD_SHA1, 0)
#define sha1DigestReset(CTX) gcry_md_reset(CTX)
#define sha1DigestUpdate(CTX, DATA, LENGTH) gcry_md_write(CTX, DATA, LENGTH)
#define sha1DigestFinal(CTX, HASH) \
{\
gcry_md_final(CTX);\
memcpy(HASH, gcry_md_read(CTX, 0), 20);\
}
#define sha1DigestFree(CTX) gcry_md_close(CTX)
#endif // HAVE_LIBGCRYPT

#endif // ENABLE_BITTORRENT

#endif // _D_MESSAGE_DIGEST_H_
