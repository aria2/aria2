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
#include "messageDigest.h"
#include "Util.h"

namespace aria2 {

static MessageDigestContext::DigestAlgoMap::value_type digests[] = {
#ifdef HAVE_LIBSSL
  MessageDigestContext::DigestAlgoMap::value_type("md5", EVP_md5()),
  MessageDigestContext::DigestAlgoMap::value_type("sha1", EVP_sha1()),
# ifdef HAVE_EVP_SHA256
  MessageDigestContext::DigestAlgoMap::value_type("sha256", EVP_sha256()),
# endif // HAVE_EVP_SHA256
#elif HAVE_LIBGCRYPT
  MessageDigestContext::DigestAlgoMap::value_type("md5", GCRY_MD_MD5),
  MessageDigestContext::DigestAlgoMap::value_type("sha1", GCRY_MD_SHA1),
  MessageDigestContext::DigestAlgoMap::value_type("sha256", GCRY_MD_SHA256),
#endif // HAVE_LIBGCRYPT
};

MessageDigestContext::DigestAlgoMap
MessageDigestContext::digestAlgos(&digests[0],
				  &digests[sizeof(digests)/sizeof(DigestAlgoMap::value_type)]);

std::string MessageDigestContext::digestFinal()
{
  size_t length = digestLength(algo);
  unsigned char* rawMD = new unsigned char[length];
  digestFinal(rawMD);
  std::string rawMDString(&rawMD[0], &rawMD[length]);
  delete [] rawMD;
  return rawMDString;
}

std::string MessageDigestContext::getSupportedAlgoString()
{
  std::string algos;
  for(DigestAlgoMap::const_iterator itr = digestAlgos.begin();
      itr != digestAlgos.end(); ++itr) {
    algos += (*itr).first+", ";
  }
  return Util::trim(algos, ", ");
}

} // namespace aria2
