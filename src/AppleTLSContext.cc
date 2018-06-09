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
#include <sstream>

#ifdef __MAC_10_6
#  include <Security/SecImportExport.h>
#endif

#include "LogFactory.h"
#include "BufferedFile.h"
#include "Logger.h"
#include "MessageDigest.h"
#include "fmt.h"
#include "message.h"
#include "util.h"

namespace {
using namespace aria2;

#if defined(__MAC_10_6)

#  if defined(__MAC_10_7)
static const void* query_keys[] = {kSecClass, kSecReturnRef, kSecMatchPolicy,
                                   kSecMatchLimit};
#  endif // defined(__MAC_10_7)

template <typename T> class CFRef {
  T ref_;

public:
  CFRef() : ref_(nullptr) {}

  CFRef(T ref) : ref_(ref) {}

  ~CFRef() { reset(nullptr); }

  void reset(T ref)
  {
    if (ref_) {
      CFRelease(ref_);
    }
    ref_ = ref;
  }

  T get() { return ref_; }

  const T get() const { return ref_; }

  operator bool() const { return !!ref_; }
};

static inline bool isWhitespace(char c)
{
  // Fingerprints are often separated by colons
  return isspace(c) || c == ':';
}

static inline std::string stripWhitespace(std::string str)
{
  str.erase(std::remove_if(std::begin(str), std::end(str), isWhitespace),
            std::end(str));
  return str;
}

struct hash_validator {
  const std::string& hash_;

  hash_validator(const std::string& hash) : hash_(hash) {}

  inline bool operator()(std::string type) const
  {
    return MessageDigest::isValidHash(type, hash_);
  }
};

struct hash_finder {
  CFDataRef data_;
  const std::string& hash_;

  hash_finder(CFDataRef data, const std::string& hash)
      : data_(data), hash_(hash)
  {
  }

  inline bool operator()(std::string type) const
  {
    std::string hash =
        MessageDigest::create(type)
            ->update(CFDataGetBytePtr(data_), CFDataGetLength(data_))
            .digest();
    hash = util::toHex(hash);
    return hash == hash_;
  }
};

std::string errToString(OSStatus err)
{
  std::string rv = "Unkown error";
  CFRef<CFStringRef> cerr(SecCopyErrorMessageString(err, nullptr));
  if (!cerr) {
    return rv;
  }
  size_t len = CFStringGetLength(cerr.get()) * 4;
  auto buf = make_unique<char[]>(len);
  if (CFStringGetCString(cerr.get(), buf.get(), len, kCFStringEncodingUTF8)) {
    rv = buf.get();
  }
  return rv;
}

bool checkIdentity(const SecIdentityRef id, const std::string& fingerPrint,
                   const std::vector<std::string> supported)
{
  CFRef<SecCertificateRef> ref;
  SecCertificateRef raw_ref = nullptr;
  if (SecIdentityCopyCertificate(id, &raw_ref) != errSecSuccess) {
    A2_LOG_ERROR("Failed to get a certref!");
    return false;
  }
  ref.reset(raw_ref);

  CFRef<CFDataRef> data(SecCertificateCopyData(ref.get()));
  if (!data) {
    A2_LOG_ERROR("Failed to get a data!");
    return false;
  }

  // Do try all supported hash algorithms.
  // Usually the fingerprint would be sha1 or md5, however this is more
  // future-proof. Also "usually" doesn't cut it; there is already software
  // using SHA-2 class algos, and SHA-3 is standardized and potential users
  // cannot be far.
  return std::find_if(std::begin(supported), std::end(supported),
                      hash_finder(data.get(), fingerPrint)) !=
         std::end(supported);
}

#endif // defined(__MAC_10_6)
} // namespace

namespace aria2 {

TLSContext* TLSContext::make(TLSSessionSide side, TLSVersion ver)
{
  return new AppleTLSContext(side, ver);
}

AppleTLSContext::~AppleTLSContext()
{
  if (credentials_) {
    CFRelease(credentials_);
    credentials_ = nullptr;
  }
}

bool AppleTLSContext::addCredentialFile(const std::string& certfile,
                                        const std::string& keyfile)
{
  if (certfile.empty()) {
    return false;
  }

  if (tryAsFingerprint(certfile)) {
    return true;
  }
  if (tryAsPKCS12(certfile)) {
    return true;
  }

  A2_LOG_WARN("Only PKCS12/PFX files with a blank password and fingerprints of "
              "certificates in your KeyChain are supported. See the manual.");
  return false;
}

bool AppleTLSContext::addTrustedCACertFile(const std::string& certfile)
{
  A2_LOG_INFO("TLS CA bundle files are not supported. Use the KeyChain to "
              "manage your certificates.");
  return false;
}

SecIdentityRef AppleTLSContext::getCredentials() { return credentials_; }

bool AppleTLSContext::tryAsFingerprint(const std::string& fingerprint)
{
  auto fp = stripWhitespace(fingerprint);
  // Verify this is a valid hex representation and normalize.
  fp = util::toHex(util::fromHex(std::begin(fp), std::end(fp)));

  // Verify this can represent a hash
  auto ht = MessageDigest::getSupportedHashTypes();
  if (std::find_if(std::begin(ht), std::end(ht), hash_validator(fp)) ==
      std::end(ht)) {
    A2_LOG_INFO(fmt("%s is not a fingerprint, invalid hash representation",
                    fingerprint.c_str()));
    return false;
  }

#if defined(__MAC_10_7)
  A2_LOG_DEBUG(fmt("Looking for cert with fingerprint %s", fp.c_str()));

  // Build and run the KeyChain the query.
  CFRef<SecPolicyRef> policy(SecPolicyCreateSSL(true, nullptr));
  if (!policy) {
    A2_LOG_ERROR("Failed to create SecPolicy");
    return false;
  }
  const void* query_values[] = {kSecClassIdentity, kCFBooleanTrue, policy.get(),
                                kSecMatchLimitAll};
  CFRef<CFDictionaryRef> query(CFDictionaryCreate(
      nullptr, query_keys, query_values, 4, nullptr, nullptr));
  if (!query) {
    A2_LOG_ERROR("Failed to create identity query");
    return false;
  }
  CFArrayRef identities;
  OSStatus err = SecItemCopyMatching(query.get(), (CFTypeRef*)&identities);
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

  A2_LOG_ERROR(
      fmt("Failed to lookup %s in your KeyChain", fingerprint.c_str()));
  return false;

#else // defined(__MAC_10_7)
#  if defined(__MAC_10_6)

  CFRef<SecIdentitySearchRef> search;
  SecIdentitySearchRef raw_search;

  // Deprecated as of 10.7
  OSStatus err = SecIdentitySearchCreate(0, CSSM_KEYUSE_SIGN, &raw_search);
  if (err != errSecSuccess) {
    A2_LOG_ERROR("Certificate search failed: " + errToString(err));
  }
  search.reset(raw_search);

  SecIdentityRef id;
  while (SecIdentitySearchCopyNext(search, &id) == errSecSuccess) {
    if (!checkIdentity(id, fp, ht)) {
      continue;
    }
    A2_LOG_INFO("Found cert with matching fingerprint");
    credentials_ = id;
    return true;
  }

  A2_LOG_ERROR(
      fmt("Failed to lookup %s in your KeyChain", fingerprint.c_str()));
  return false;

#  else // defined(__MAC_10_6)

  A2_LOG_ERROR("Your system does not support creditials via fingerprints; "
               "Upgrade to OSX 10.6 or later");
  return false;

#  endif // defined(__MAC_10_6)
#endif   // defined(__MAC_10_7)
}

bool AppleTLSContext::tryAsPKCS12(const std::string& certfile)
{
#if defined(__MAC_10_6)
  std::stringstream ss;
  BufferedFile(certfile.c_str(), BufferedFile::READ).transfer(ss);
  auto data = ss.str();
  if (data.empty()) {
    A2_LOG_ERROR("Couldn't read certificate file.");
    return false;
  }
  CFRef<CFDataRef> dataRef(
      CFDataCreate(nullptr, (const UInt8*)data.c_str(), data.size()));
  if (!dataRef) {
    A2_LOG_ERROR("Couldn't allocate PKCS12 data");
    return false;
  }

  return tryAsPKCS12(dataRef.get(), "") || tryAsPKCS12(dataRef.get(), nullptr);

#else // defined(__MAC_10_6)
  A2_LOG_INFO("PKCS12 files are only supported in OSX 10.6 or later.");
  return false;

#endif // defined(__MAC_10_6)
}

bool AppleTLSContext::tryAsPKCS12(CFDataRef data, const char* password)
{
#if defined(__MAC_10_6)
  CFRef<CFStringRef> passwordRef;
  if (password) {
    passwordRef.reset(CFStringCreateWithBytes(nullptr, (const UInt8*)password,
                                              strlen(password),
                                              kCFStringEncodingUTF8, false));
  }
  const void* keys[] = {kSecImportExportPassphrase};
  const void* values[] = {passwordRef.get()};
  CFRef<CFDictionaryRef> options(
      CFDictionaryCreate(nullptr, keys, values, 1, nullptr, nullptr));
  if (!options) {
    A2_LOG_ERROR("Failed to create options");
    return false;
  }

  CFRef<CFArrayRef> items;
  CFArrayRef raw_items = nullptr;
  OSStatus rv = SecPKCS12Import(data, options.get(), &raw_items);
  if (rv != errSecSuccess) {
    A2_LOG_DEBUG(
        fmt("Failed to parse PKCS12 data: %s", errToString(rv).c_str()));
    return false;
  }
  items.reset(raw_items);

  CFDictionaryRef idAndTrust =
      (CFDictionaryRef)CFArrayGetValueAtIndex(items.get(), 0);
  if (!idAndTrust) {
    A2_LOG_ERROR("Failed to get identity and trust from PKCS12 data");
    return false;
  }
  credentials_ =
      (SecIdentityRef)CFDictionaryGetValue(idAndTrust, kSecImportItemIdentity);
  if (!credentials_) {
    A2_LOG_ERROR("Failed to get credentials PKCS12 data");
    return false;
  }
  CFRetain(credentials_);
  A2_LOG_INFO("Loaded certificate from file");
  return true;

#else // defined(__MAC_10_6)
  return false;

#endif // defined(__MAC_10_6)
}

} // namespace aria2
