/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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
#include "AppleTLSContext.h"

#include <algorithm>
#include <functional>

#include "LogFactory.h"
#include "Logger.h"
#include "MessageDigest.h"
#include "fmt.h"
#include "message.h"
#include "util.h"

namespace {
  using namespace aria2;

#if defined(__MAC_10_6)

#if defined(__MAC_10_7)
  static const void *query_keys[] = {
    kSecClass,
    kSecReturnRef,
    kSecMatchPolicy,
    kSecMatchLimit
  };
#endif // defined(__MAC_10_7)

  class cfrelease {
    const void *ptr_;
  public:
    inline cfrelease(const void *ptr) : ptr_(ptr) {}
    inline ~cfrelease() { if (ptr_) CFRelease(ptr_); }
  };

  static inline bool isWhitespace(char c)
  {
    // Fingerprints are often separated by colons
    return isspace(c) || c == ':';
  }
  static inline std::string stripWhitespace(std::string str)
  {
    str.erase(std::remove_if(str.begin(), str.end(), isWhitespace), str.end());
    return str;
  }

  struct hash_validator {
    const std::string& hash_;
    hash_validator(const std::string& hash) : hash_(hash) {}
    inline bool operator()(std::string type) const {
      return MessageDigest::isValidHash(type, hash_);
    }
  };

  struct hash_finder {
    CFDataRef data_;
    const std::string& hash_;
    hash_finder(CFDataRef data, const std::string& hash)
      : data_(data), hash_(hash)
    {}
    inline bool operator()(std::string type) const {
      std::string hash = MessageDigest::create(type)->update(
          CFDataGetBytePtr(data_), CFDataGetLength(data_)).digest();
      hash = util::toHex(hash);
      return hash == hash_;
    }
  };


  std::string errToString(OSStatus err)
  {
    std::string rv = "Unkown error";
    CFStringRef cerr = SecCopyErrorMessageString(err, 0);
    if (cerr) {
      size_t len = CFStringGetLength(cerr) * 4;
      char *buf = new char[len];
      if (CFStringGetCString(cerr, buf, len, kCFStringEncodingUTF8)) {
        rv = buf;
      }
      delete [] buf;
      CFRelease(cerr);
    }
    return rv;
  }

  bool checkIdentity(const SecIdentityRef id, const std::string& fingerPrint,
                     const std::vector<std::string> supported)
  {
    SecCertificateRef ref = 0;
    if (SecIdentityCopyCertificate(id, &ref) != errSecSuccess) {
      A2_LOG_ERROR("Failed to get a certref!");
      return false;
    }
    cfrelease del_ref(ref);
    CFDataRef data = SecCertificateCopyData(ref);
    if (!data) {
      A2_LOG_ERROR("Failed to get a data!");
      return false;
    }
    cfrelease del_data(data);

    // Do try all supported hash algorithms.
    // Usually the fingerprint would be sha1 or md5, however this is more
    // future-proof. Also "usually" doesn't cut it; there is already software
    // using SHA-2 class algos, and SHA-3 is standardized and potential users
    // cannot be far.
    return std::find_if(supported.begin(), supported.end(),
                        hash_finder(data, fingerPrint)) != supported.end();
  }

#endif // defined(__MAC_10_6)

}

namespace aria2 {

TLSContext* TLSContext::make(TLSSessionSide side)
{
  return new AppleTLSContext(side);
}

AppleTLSContext::~AppleTLSContext()
{
  if (credentials_) {
    CFRelease(credentials_);
    credentials_ = 0;
  }
}

bool AppleTLSContext::addCredentialFile(const std::string& certfile,
                                        const std::string& keyfile)
{
  if (tryAsFingerprint(certfile)) {
    return true;
  }

  A2_LOG_WARN("TLS credential files are not supported. Use the KeyChain to manage your certificates and provide a fingerprint. See the manual.");
  return false;
}

bool AppleTLSContext::addTrustedCACertFile(const std::string& certfile)
{
  A2_LOG_INFO("TLS CA bundle files are not supported. Use the KeyChain to manage your certificates.");
  return false;
}

SecIdentityRef AppleTLSContext::getCredentials()
{
  return credentials_;
}

bool AppleTLSContext::tryAsFingerprint(const std::string& fingerprint)
{
  std::string fp = stripWhitespace(fingerprint);
  // Verify this is a valid hex representation and normalize.
  fp = util::toHex(util::fromHex(fp.begin(), fp.end()));

  // Verify this can represent a hash
  std::vector<std::string> ht = MessageDigest::getSupportedHashTypes();
  if (std::find_if(ht.begin(), ht.end(), hash_validator(fp)) == ht.end()) {
    A2_LOG_INFO(fmt("%s is not a fingerprint, invalid hash representation", fingerprint.c_str()));
    return false;
  }

#if defined(__MAC_10_7)
  A2_LOG_DEBUG(fmt("Looking for cert with fingerprint %s", fp.c_str()));

  // Build and run the KeyChain the query.
  SecPolicyRef policy = SecPolicyCreateSSL(true, 0);
  if (!policy) {
    A2_LOG_ERROR("Failed to create SecPolicy");
    return false;
  }
  cfrelease del_policy(policy);
  const void *query_values[] = {
    kSecClassIdentity,
    kCFBooleanTrue,
    policy,
    kSecMatchLimitAll
  };
  CFDictionaryRef query = CFDictionaryCreate(0, query_keys, query_values,
                                             4, 0, 0);
  if (!query) {
    A2_LOG_ERROR("Failed to create identity query");
    return false;
  }
  cfrelease del_query(query);
  CFArrayRef identities;
  OSStatus err = SecItemCopyMatching(query, (CFTypeRef*)&identities);
  if (err != errSecSuccess) {
    A2_LOG_ERROR("Query failed: " + errToString(err));
    return false;
  }

  // Alrighty, search the fingerprint.
  const size_t nvals = CFArrayGetCount(identities);
  for (size_t i = 0; i < nvals; ++i) {
    SecIdentityRef id = (SecIdentityRef)CFArrayGetValueAtIndex(identities, i);
    if (!id) {
      A2_LOG_ERROR("Failed to get a value!");
      continue;
    }
    if (!checkIdentity(id, fp, ht)) {
      continue;
    }
    A2_LOG_INFO("Found cert with matching fingerprint");
    credentials_ = id;
    CFRetain(id);
    return true;
  }

  A2_LOG_ERROR(fmt("Failed to lookup %s in your KeyChain", fingerprint.c_str()));
  return false;

#else // defined(__MAC_10_7)
#if defined(__MAC_10_6)

  SecIdentitySearchRef search;

  // Deprecated as of 10.7
  OSStatus err = SecIdentitySearchCreate(0, CSSM_KEYUSE_SIGN, &search);
  if (err != errSecSuccess) {
    A2_LOG_ERROR("Certificate search failed: " + errToString(err));
  }
  cfrelease del_search(search);

  SecIdentityRef id;
  while (SecIdentitySearchCopyNext(search, &id) == errSecSuccess) {
    if (!checkIdentity(id, fp, ht)) {
      continue;
    }
    A2_LOG_INFO("Found cert with matching fingerprint");
    credentials_ = id;
    return true;
  }

  A2_LOG_ERROR(fmt("Failed to lookup %s in your KeyChain", fingerprint.c_str()));
  return false;

#else // defined(__MAC_10_6)

  A2_LOG_ERROR("Your system does not support creditials via fingerprints; Upgrade to OSX 10.6 or later");
  return false;

#endif // defined(__MAC_10_6)
#endif // defined(__MAC_10_7)
}

} // namespace aria2
