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
#include "messageDigest.h"
#include "util.h"
#include "array_fun.h"

namespace aria2 {

const std::string MessageDigestContext::SHA1("sha-1");

const std::string MessageDigestContext::SHA256("sha-256");

const std::string MessageDigestContext::MD5("md5");

namespace {
struct DigestAlgoEntry {
  MessageDigestContext::DigestAlgo algo;
  int strength;
  DigestAlgoEntry(const MessageDigestContext::DigestAlgo& algo, int strength):
    algo(algo), strength(strength) {}
};
} // namespace

typedef std::map<std::string, DigestAlgoEntry>
DigestAlgoMap;

namespace {
const DigestAlgoMap& getDigestAlgos()
{
  enum AlgoStrength {
    STRENGTH_MD5 = 0,
    STRENGTH_SHA_1 = 1,
    STRENGTH_SHA_256 = 2
  };
  static const DigestAlgoMap::value_type digests[] = {
#ifdef HAVE_LIBSSL
    DigestAlgoMap::value_type("md5", DigestAlgoEntry(EVP_md5(), STRENGTH_MD5)),
    DigestAlgoMap::value_type
    ("sha-1", DigestAlgoEntry(EVP_sha1(), STRENGTH_SHA_1)),
# ifdef HAVE_EVP_SHA256
    DigestAlgoMap::value_type
    ("sha-256", DigestAlgoEntry(EVP_sha256(), STRENGTH_SHA_256)),
# endif // HAVE_EVP_SHA256
#elif HAVE_LIBGCRYPT
    DigestAlgoMap::value_type
    ("md5", DigestAlgoEntry(GCRY_MD_MD5, STRENGTH_MD5)),
    DigestAlgoMap::value_type
    ("sha-1", DigestAlgoEntry(GCRY_MD_SHA1, STRENGTH_SHA_1)),
    DigestAlgoMap::value_type
    ("sha-256", DigestAlgoEntry(GCRY_MD_SHA256, STRENGTH_SHA_256)),
#endif // HAVE_LIBGCRYPT
  };
  static const DigestAlgoMap algomap(vbegin(digests), vend(digests));
  return algomap;
}
} // namespace

std::string MessageDigestContext::getCanonicalAlgo
(const std::string& algostring)
{
  if(strcasecmp("sha-1", algostring.c_str()) == 0 ||
     strcasecmp("sha1", algostring.c_str()) == 0) {
    return SHA1;
  } else if(strcasecmp("sha-256", algostring.c_str()) == 0 ||
            strcasecmp("sha256", algostring.c_str()) == 0) {
    return SHA256;
  } else if(strcasecmp("md5", algostring.c_str()) == 0) {
    return MD5;
  } else {
    return algostring;
  }
}

std::string MessageDigestContext::digestFinal()
{
  size_t length = digestLength(algo_);
  unsigned char* rawMD = new unsigned char[length];
  digestFinal(rawMD);
  std::string rawMDString(&rawMD[0], &rawMD[length]);
  delete [] rawMD;
  return rawMDString;
}

bool MessageDigestContext::supports(const std::string& algostring)
{
  const DigestAlgoMap& allAlgos = getDigestAlgos();
  DigestAlgoMap::const_iterator itr = allAlgos.find(algostring);
  if(itr == allAlgos.end()) {
    return false;
  } else {
    return true;
  }
}

MessageDigestContext::DigestAlgo
MessageDigestContext::getDigestAlgo(const std::string& algostring)
{
  const DigestAlgoMap& allAlgos = getDigestAlgos();
  DigestAlgoMap::const_iterator itr = allAlgos.find(algostring);
  if(itr == allAlgos.end()) {
    throw DL_ABORT_EX
      (StringFormat("Digest algorithm %s is not supported.",
                    algostring.c_str()).str());
  }
  return (*itr).second.algo;
}

std::string MessageDigestContext::getSupportedAlgoString()
{
  const DigestAlgoMap& allAlgos = getDigestAlgos();
  std::string algos;
  for(DigestAlgoMap::const_iterator itr = allAlgos.begin(),
        eoi = allAlgos.end(); itr != eoi; ++itr) {
    algos += (*itr).first;
    algos += ", ";
  }
  return util::strip(algos, ", ");
}

bool MessageDigestContext::isStronger
(const std::string& lhs, const std::string& rhs)
{
  const DigestAlgoMap& allAlgos = getDigestAlgos();
  DigestAlgoMap::const_iterator lhsitr = allAlgos.find(lhs);
  DigestAlgoMap::const_iterator rhsitr = allAlgos.find(rhs);
  if(lhsitr == allAlgos.end() || rhsitr == allAlgos.end()) {
    return false;
  }
  return (*lhsitr).second.strength > (*rhsitr).second.strength;
}

bool MessageDigestContext::isValidHash
(const std::string& algostring, const std::string& hashstring)
{
  return util::isHexDigit(hashstring) &&
    supports(algostring) && digestLength(algostring)*2 == hashstring.size();
}

} // namespace aria2
