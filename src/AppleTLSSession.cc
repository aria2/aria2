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

#include "AppleTLSSession.h"

#include <sstream>
#include <vector>

#include <CoreFoundation/CoreFoundation.h>

#include "LogFactory.h"
#include "a2functional.h"
#include "fmt.h"

#define ioErr -36
#define paramErr -50

#ifndef errSSLServerAuthCompleted
#  define errSSLServerAuthCompleted -9841
#endif

namespace {
#if !defined(__MAC_10_8)
static const SSLProtocol kTLSProtocol11 = (SSLProtocol)(kSSLProtocolAll + 1);
static const SSLProtocol kTLSProtocol12 = (SSLProtocol)(kSSLProtocolAll + 2);
#endif

#ifndef CIPHER_NO_DHPARAM
// Diffie-Hellman params, to seed the engine instead of having it spend up
// to 30 seconds on generating them. It should be save to share these. :p
// This was generated using: openssl dhparam -outform DER 2048
static const uint8_t dhparam[] =
    "\x30\x82\x01\x08\x02\x82\x01\x01\x00\x97\xea\xd0\x46\xf7\xae\xa7\x76\x80"
    "\x9c\x74\x56\x98\xd8\x56\x97\x2b\x20\x6c\x77\xe2\x82\xbb\xc8\x84\xbe\xe7"
    "\x63\xaf\xcc\x30\xd0\x67\x97\x7d\x1b\xab\x59\x30\xa9\x13\x67\x21\xd7\xd4"
    "\x0e\x46\xcf\xe5\x80\xdf\xc9\xb9\xba\x54\x9b\x46\x2f\x3b\x45\xfc\x2f\xaf"
    "\xad\xc0\x17\x56\xdd\x52\x42\x57\x45\x70\x14\xe5\xbe\x67\xaa\xde\x69\x75"
    "\x30\x0d\xf9\xa2\xc4\x63\x4d\x7a\x39\xef\x14\x62\x18\x33\x44\xa1\xf9\xc1"
    "\x52\xd1\xb6\x72\x21\x98\xf8\xab\x16\x1b\x7b\x37\x65\xe3\xc5\x11\x00\xf6"
    "\x36\x1f\xd8\x5f\xd8\x9f\x43\xa8\xce\x9d\xbf\x5e\xd6\x2d\xfa\x0a\xc2\x01"
    "\x54\xc2\xd9\x81\x54\x55\xb5\x26\xf8\x88\x37\xf5\xfe\xe0\xef\x4a\x34\x81"
    "\xdc\x5a\xb3\x71\x46\x27\xe3\xcd\x24\xf6\x1b\xf1\xe2\x0f\xc2\xa1\x39\x53"
    "\x5b\xc5\x38\x46\x8e\x67\x4c\xd9\xdd\xe4\x37\x06\x03\x16\xf1\x1d\x7a\xba"
    "\x2d\xc1\xe4\x03\x1a\x58\xe5\x29\x5a\x29\x06\x69\x61\x7a\xd8\xa9\x05\x9f"
    "\xc1\xa2\x45\x9c\x17\xad\x52\x69\x33\xdc\x18\x8d\x15\xa6\x5e\xcd\x94\xf4"
    "\x45\xbb\x9f\xc2\x7b\x85\x00\x61\xb0\x1a\xdc\x3c\x86\xaa\x9f\x5c\x04\xb3"
    "\x90\x0b\x35\x64\xff\xd9\xe3\xac\xf2\xf2\xeb\x3a\x63\x02\x01\x02";
#endif // CIPHER_NO_DHPARAM

static inline const char* protoToString(SSLProtocol proto)
{
  switch (proto) {
  case kSSLProtocol2:
    return "SSLv2 (!)";
  case kSSLProtocol3:
    return "SSLv3 (!)";
  case kTLSProtocol1:
    return "TLSv1";
  case kTLSProtocol11:
    return "TLSv1.1";
  case kTLSProtocol12:
    return "TLSv1.2";
  default:
    return "Unknown";
  }
}

#define SUITE(s, n)                                                            \
  {                                                                            \
    n, #s                                                                      \
  }
static struct {
  SSLCipherSuite suite;
  const char* name;
} kSuites[] = {
    // From CipherSuite.h (10.11)
    SUITE(SSL_NULL_WITH_NULL_NULL, 0x0000),
    SUITE(SSL_RSA_WITH_NULL_MD5, 0x0001),
    SUITE(SSL_RSA_WITH_NULL_SHA, 0x0002),
    SUITE(SSL_RSA_EXPORT_WITH_RC4_40_MD5, 0x0003),
    SUITE(SSL_RSA_WITH_RC4_128_MD5, 0x0004),
    SUITE(SSL_RSA_WITH_RC4_128_SHA, 0x0005),
    SUITE(SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5, 0x0006),
    SUITE(SSL_RSA_WITH_IDEA_CBC_SHA, 0x0007),
    SUITE(SSL_RSA_EXPORT_WITH_DES40_CBC_SHA, 0x0008),
    SUITE(SSL_RSA_WITH_DES_CBC_SHA, 0x0009),
    SUITE(SSL_RSA_WITH_3DES_EDE_CBC_SHA, 0x000A),
    SUITE(SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA, 0x000B),
    SUITE(SSL_DH_DSS_WITH_DES_CBC_SHA, 0x000C),
    SUITE(SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA, 0x000D),
    SUITE(SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA, 0x000E),
    SUITE(SSL_DH_RSA_WITH_DES_CBC_SHA, 0x000F),
    SUITE(SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA, 0x0010),
    SUITE(SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA, 0x0011),
    SUITE(SSL_DHE_DSS_WITH_DES_CBC_SHA, 0x0012),
    SUITE(SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA, 0x0013),
    SUITE(SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA, 0x0014),
    SUITE(SSL_DHE_RSA_WITH_DES_CBC_SHA, 0x0015),
    SUITE(SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA, 0x0016),
    SUITE(SSL_DH_anon_EXPORT_WITH_RC4_40_MD5, 0x0017),
    SUITE(SSL_DH_anon_WITH_RC4_128_MD5, 0x0018),
    SUITE(SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA, 0x0019),
    SUITE(SSL_DH_anon_WITH_DES_CBC_SHA, 0x001A),
    SUITE(SSL_DH_anon_WITH_3DES_EDE_CBC_SHA, 0x001B),
    SUITE(SSL_FORTEZZA_DMS_WITH_NULL_SHA, 0x001C),
    SUITE(SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA, 0x001D),
    SUITE(TLS_RSA_WITH_AES_128_CBC_SHA, 0x002F),
    SUITE(TLS_DH_DSS_WITH_AES_128_CBC_SHA, 0x0030),
    SUITE(TLS_DH_RSA_WITH_AES_128_CBC_SHA, 0x0031),
    SUITE(TLS_DHE_DSS_WITH_AES_128_CBC_SHA, 0x0032),
    SUITE(TLS_DHE_RSA_WITH_AES_128_CBC_SHA, 0x0033),
    SUITE(TLS_DH_anon_WITH_AES_128_CBC_SHA, 0x0034),
    SUITE(TLS_RSA_WITH_AES_256_CBC_SHA, 0x0035),
    SUITE(TLS_DH_DSS_WITH_AES_256_CBC_SHA, 0x0036),
    SUITE(TLS_DH_RSA_WITH_AES_256_CBC_SHA, 0x0037),
    SUITE(TLS_DHE_DSS_WITH_AES_256_CBC_SHA, 0x0038),
    SUITE(TLS_DHE_RSA_WITH_AES_256_CBC_SHA, 0x0039),
    SUITE(TLS_DH_anon_WITH_AES_256_CBC_SHA, 0x003A),
    SUITE(TLS_ECDH_ECDSA_WITH_NULL_SHA, 0xC001),
    SUITE(TLS_ECDH_ECDSA_WITH_RC4_128_SHA, 0xC002),
    SUITE(TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA, 0xC003),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA, 0xC004),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA, 0xC005),
    SUITE(TLS_ECDHE_ECDSA_WITH_NULL_SHA, 0xC006),
    SUITE(TLS_ECDHE_ECDSA_WITH_RC4_128_SHA, 0xC007),
    SUITE(TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA, 0xC008),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA, 0xC009),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA, 0xC00A),
    SUITE(TLS_ECDH_RSA_WITH_NULL_SHA, 0xC00B),
    SUITE(TLS_ECDH_RSA_WITH_RC4_128_SHA, 0xC00C),
    SUITE(TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA, 0xC00D),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_CBC_SHA, 0xC00E),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_CBC_SHA, 0xC00F),
    SUITE(TLS_ECDHE_RSA_WITH_NULL_SHA, 0xC010),
    SUITE(TLS_ECDHE_RSA_WITH_RC4_128_SHA, 0xC011),
    SUITE(TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA, 0xC012),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA, 0xC013),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA, 0xC014),
    SUITE(TLS_ECDH_anon_WITH_NULL_SHA, 0xC015),
    SUITE(TLS_ECDH_anon_WITH_RC4_128_SHA, 0xC016),
    SUITE(TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA, 0xC017),
    SUITE(TLS_ECDH_anon_WITH_AES_128_CBC_SHA, 0xC018),
    SUITE(TLS_ECDH_anon_WITH_AES_256_CBC_SHA, 0xC019),
    SUITE(TLS_NULL_WITH_NULL_NULL, 0x0000),
    SUITE(TLS_RSA_WITH_NULL_MD5, 0x0001),
    SUITE(TLS_RSA_WITH_NULL_SHA, 0x0002),
    SUITE(TLS_RSA_WITH_RC4_128_MD5, 0x0004),
    SUITE(TLS_RSA_WITH_RC4_128_SHA, 0x0005),
    SUITE(TLS_RSA_WITH_3DES_EDE_CBC_SHA, 0x000A),
    SUITE(TLS_RSA_WITH_AES_128_CBC_SHA, 0x002F),
    SUITE(TLS_RSA_WITH_AES_256_CBC_SHA, 0x0035),
    SUITE(TLS_RSA_WITH_NULL_SHA256, 0x003B),
    SUITE(TLS_RSA_WITH_AES_128_CBC_SHA256, 0x003C),
    SUITE(TLS_RSA_WITH_AES_256_CBC_SHA256, 0x003D),
    SUITE(TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA, 0x000D),
    SUITE(TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA, 0x0010),
    SUITE(TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA, 0x0013),
    SUITE(TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA, 0x0016),
    SUITE(TLS_DH_DSS_WITH_AES_128_CBC_SHA, 0x0030),
    SUITE(TLS_DH_RSA_WITH_AES_128_CBC_SHA, 0x0031),
    SUITE(TLS_DHE_DSS_WITH_AES_128_CBC_SHA, 0x0032),
    SUITE(TLS_DHE_RSA_WITH_AES_128_CBC_SHA, 0x0033),
    SUITE(TLS_DH_DSS_WITH_AES_256_CBC_SHA, 0x0036),
    SUITE(TLS_DH_RSA_WITH_AES_256_CBC_SHA, 0x0037),
    SUITE(TLS_DHE_DSS_WITH_AES_256_CBC_SHA, 0x0038),
    SUITE(TLS_DHE_RSA_WITH_AES_256_CBC_SHA, 0x0039),
    SUITE(TLS_DH_DSS_WITH_AES_128_CBC_SHA256, 0x003E),
    SUITE(TLS_DH_RSA_WITH_AES_128_CBC_SHA256, 0x003F),
    SUITE(TLS_DHE_DSS_WITH_AES_128_CBC_SHA256, 0x0040),
    SUITE(TLS_DHE_RSA_WITH_AES_128_CBC_SHA256, 0x0067),
    SUITE(TLS_DH_DSS_WITH_AES_256_CBC_SHA256, 0x0068),
    SUITE(TLS_DH_RSA_WITH_AES_256_CBC_SHA256, 0x0069),
    SUITE(TLS_DHE_DSS_WITH_AES_256_CBC_SHA256, 0x006A),
    SUITE(TLS_DHE_RSA_WITH_AES_256_CBC_SHA256, 0x006B),
    SUITE(TLS_DH_anon_WITH_RC4_128_MD5, 0x0018),
    SUITE(TLS_DH_anon_WITH_3DES_EDE_CBC_SHA, 0x001B),
    SUITE(TLS_DH_anon_WITH_AES_128_CBC_SHA, 0x0034),
    SUITE(TLS_DH_anon_WITH_AES_256_CBC_SHA, 0x003A),
    SUITE(TLS_DH_anon_WITH_AES_128_CBC_SHA256, 0x006C),
    SUITE(TLS_DH_anon_WITH_AES_256_CBC_SHA256, 0x006D),
    SUITE(TLS_PSK_WITH_RC4_128_SHA, 0x008A),
    SUITE(TLS_PSK_WITH_3DES_EDE_CBC_SHA, 0x008B),
    SUITE(TLS_PSK_WITH_AES_128_CBC_SHA, 0x008C),
    SUITE(TLS_PSK_WITH_AES_256_CBC_SHA, 0x008D),
    SUITE(TLS_DHE_PSK_WITH_RC4_128_SHA, 0x008E),
    SUITE(TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA, 0x008F),
    SUITE(TLS_DHE_PSK_WITH_AES_128_CBC_SHA, 0x0090),
    SUITE(TLS_DHE_PSK_WITH_AES_256_CBC_SHA, 0x0091),
    SUITE(TLS_RSA_PSK_WITH_RC4_128_SHA, 0x0092),
    SUITE(TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA, 0x0093),
    SUITE(TLS_RSA_PSK_WITH_AES_128_CBC_SHA, 0x0094),
    SUITE(TLS_RSA_PSK_WITH_AES_256_CBC_SHA, 0x0095),
    SUITE(TLS_PSK_WITH_NULL_SHA, 0x002C),
    SUITE(TLS_DHE_PSK_WITH_NULL_SHA, 0x002D),
    SUITE(TLS_RSA_PSK_WITH_NULL_SHA, 0x002E),
    SUITE(TLS_RSA_WITH_AES_128_GCM_SHA256, 0x009C),
    SUITE(TLS_RSA_WITH_AES_256_GCM_SHA384, 0x009D),
    SUITE(TLS_DHE_RSA_WITH_AES_128_GCM_SHA256, 0x009E),
    SUITE(TLS_DHE_RSA_WITH_AES_256_GCM_SHA384, 0x009F),
    SUITE(TLS_DH_RSA_WITH_AES_128_GCM_SHA256, 0x00A0),
    SUITE(TLS_DH_RSA_WITH_AES_256_GCM_SHA384, 0x00A1),
    SUITE(TLS_DHE_DSS_WITH_AES_128_GCM_SHA256, 0x00A2),
    SUITE(TLS_DHE_DSS_WITH_AES_256_GCM_SHA384, 0x00A3),
    SUITE(TLS_DH_DSS_WITH_AES_128_GCM_SHA256, 0x00A4),
    SUITE(TLS_DH_DSS_WITH_AES_256_GCM_SHA384, 0x00A5),
    SUITE(TLS_DH_anon_WITH_AES_128_GCM_SHA256, 0x00A6),
    SUITE(TLS_DH_anon_WITH_AES_256_GCM_SHA384, 0x00A7),
    SUITE(TLS_PSK_WITH_AES_128_GCM_SHA256, 0x00A8),
    SUITE(TLS_PSK_WITH_AES_256_GCM_SHA384, 0x00A9),
    SUITE(TLS_DHE_PSK_WITH_AES_128_GCM_SHA256, 0x00AA),
    SUITE(TLS_DHE_PSK_WITH_AES_256_GCM_SHA384, 0x00AB),
    SUITE(TLS_RSA_PSK_WITH_AES_128_GCM_SHA256, 0x00AC),
    SUITE(TLS_RSA_PSK_WITH_AES_256_GCM_SHA384, 0x00AD),
    SUITE(TLS_PSK_WITH_AES_128_CBC_SHA256, 0x00AE),
    SUITE(TLS_PSK_WITH_AES_256_CBC_SHA384, 0x00AF),
    SUITE(TLS_PSK_WITH_NULL_SHA256, 0x00B0),
    SUITE(TLS_PSK_WITH_NULL_SHA384, 0x00B1),
    SUITE(TLS_DHE_PSK_WITH_AES_128_CBC_SHA256, 0x00B2),
    SUITE(TLS_DHE_PSK_WITH_AES_256_CBC_SHA384, 0x00B3),
    SUITE(TLS_DHE_PSK_WITH_NULL_SHA256, 0x00B4),
    SUITE(TLS_DHE_PSK_WITH_NULL_SHA384, 0x00B5),
    SUITE(TLS_RSA_PSK_WITH_AES_128_CBC_SHA256, 0x00B6),
    SUITE(TLS_RSA_PSK_WITH_AES_256_CBC_SHA384, 0x00B7),
    SUITE(TLS_RSA_PSK_WITH_NULL_SHA256, 0x00B8),
    SUITE(TLS_RSA_PSK_WITH_NULL_SHA384, 0x00B9),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, 0xC023),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384, 0xC024),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256, 0xC025),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384, 0xC026),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256, 0xC027),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384, 0xC028),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256, 0xC029),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384, 0xC02A),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, 0xC02B),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, 0xC02C),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256, 0xC02D),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384, 0xC02E),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256, 0xC02F),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384, 0xC030),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256, 0xC031),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384, 0xC032),
    SUITE(TLS_EMPTY_RENEGOTIATION_INFO_SCSV, 0x00FF),
    SUITE(SSL_RSA_WITH_RC2_CBC_MD5, 0xFF80),
    SUITE(SSL_RSA_WITH_IDEA_CBC_MD5, 0xFF81),
    SUITE(SSL_RSA_WITH_DES_CBC_MD5, 0xFF82),
    SUITE(SSL_RSA_WITH_3DES_EDE_CBC_MD5, 0xFF83),
    SUITE(SSL_NO_SUCH_CIPHERSUITE, 0xFFFF)};
#undef SUITE

static inline std::string suiteToString(const SSLCipherSuite suite)
{
  for (auto& s : kSuites) {
    if (s.suite == suite) {
      return s.name;
    }
  }

  std::stringstream ss;
  ss << "Unknown suite (0x" << std::hex << suite
     << ") like TLS_NULL_WITH_NULL_NULL";
  return ss.str();
}

static const char* kBlocked[] = {"NULL", "anon", "MD5",     "EXPORT",
                                 "DES",  "IDEA", "NO_SUCH", "PSK"};

static inline bool isBlockedSuite(SSLCipherSuite suite)
{
  using namespace aria2;

  // Don't care about SSL2 suites!
  std::string name = suiteToString(suite);
  for (auto& blocked : kBlocked) {
    if (strstr(name.c_str(), blocked)) {
      A2_LOG_DEBUG(fmt("Removing blocked cipher suite: %s", name.c_str()));
      return true;
    }
  }

  return false;
}

typedef std::vector<SSLCipherSuite> SSLCipherSuiteList;
static SSLCipherSuiteList constructEnabledSuites(SSLContextRef ctx)
{
#ifndef CIPHER_CONSTRUCT_ALWAYS
  static
#endif
      SSLCipherSuiteList rv(0);

#ifndef CIPHER_CONSTRUCT_ALWAYS
  if (!rv.empty()) {
    return rv;
  }
#endif

  size_t supported = 0;
  OSStatus err = SSLGetNumberSupportedCiphers(ctx, &supported);
  if (err != noErr || !supported) {
    return rv;
  }

  rv.resize(supported, SSL_NO_SUCH_CIPHERSUITE);
  err = SSLGetSupportedCiphers(ctx, &rv[0], &supported);
  if (err != noErr || !supported) {
    rv.clear();
    return rv;
  }

  rv.erase(std::remove_if(std::begin(rv), std::end(rv), isBlockedSuite),
           std::end(rv));
  return rv;
}

} // namespace

namespace aria2 {

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new AppleTLSSession(static_cast<AppleTLSContext*>(ctx));
}

AppleTLSSession::AppleTLSSession(AppleTLSContext* ctx)
    : sslCtx_(nullptr),
      sockfd_(0),
      state_(st_constructed),
      lastError_(noErr),
      writeBuffered_(0)
{
#if defined(__MAC_10_8)
  sslCtx_ = SSLCreateContext(
      nullptr, ctx->getSide() == TLS_SERVER ? kSSLServerSide : kSSLClientSide,
      kSSLStreamType);
  lastError_ = sslCtx_ ? noErr : paramErr;
#else
  lastError_ = SSLNewContext(ctx->getSide() == TLS_SERVER, &sslCtx_);
#endif

  if (lastError_ != noErr) {
    state_ = st_error;
    return;
  }

#if defined(__MAC_10_8)
  switch (ctx->getMinTLSVersion()) {
  case TLS_PROTO_TLS11:
    (void)SSLSetProtocolVersionMin(sslCtx_, kTLSProtocol11);
    break;
  case TLS_PROTO_TLS12:
    (void)SSLSetProtocolVersionMin(sslCtx_, kTLSProtocol12);
    break;
  default:
    break;
  }
#else
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kSSLProtocolAll, false);
  switch (ctx->getMinTLSVersion()) {
  case TLS_PROTO_TLS11:
    (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol11, true);
  // fall through
  case TLS_PROTO_TLS12:
    (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol12, true);
  default:
    break;
  }
#endif

  // BEAST
  (void)SSLSetSessionOption(sslCtx_,
#if defined(__MAC_10_9)
                            kSSLSessionOptionSendOneByteRecord,
#else
                            (SSLSessionOption)0x4, // kSSLSessionOptionSendOneByteRecord
#endif
                            true);
// False Start, if available
#if defined(__MAC_10_9)
  (void)SSLSetSessionOption(sslCtx_, kSSLSessionOptionFalseStart, true);
#endif

#if defined(__MAC_10_8)
  if (!ctx->getVerifyPeer()) {
    // This disables client verification
    (void)SSLSetSessionOption(sslCtx_, kSSLSessionOptionBreakOnServerAuth,
                              true);
  }
#else
  (void)SSLSetEnableCertVerify(sslCtx_, ctx->getVerifyPeer());
#endif

#ifndef CIPHER_ENABLE_ALL
  SSLCipherSuiteList enabled = constructEnabledSuites(sslCtx_);
  if (enabled.empty()) {
    A2_LOG_ERROR("AppleTLS: Failed to construct enabled ciphers list");
    state_ = st_error;
    return;
  }
  for (const auto& suite : enabled) {
    A2_LOG_DEBUG(
        fmt("AppleTLS: Enabled suite %s", suiteToString(suite).c_str()));
  }
  if (SSLSetEnabledCiphers(sslCtx_, &enabled[0], enabled.size()) != noErr) {
    A2_LOG_ERROR("AppleTLS: Failed to set enabled ciphers list");
    state_ = st_error;
    return;
  }
#endif

  SecIdentityRef creds = ctx->getCredentials();
  if (!creds) {
    if (ctx->getSide() != TLS_SERVER) {
      // Done with client-only initialization
      return;
    }

    A2_LOG_ERROR("AppleTLS: No credentials");
    state_ = st_error;
    return;
  }

  CFArrayRef certs = CFArrayCreate(nullptr, (const void**)&creds, 1, nullptr);
  if (!certs) {
    A2_LOG_ERROR("AppleTLS: Failed to setup credentials");
    state_ = st_error;
    return;
  }

  std::unique_ptr<void, decltype(&CFRelease)> del_certs((void*)certs,
                                                        CFRelease);
  lastError_ = SSLSetCertificate(sslCtx_, certs);
  if (lastError_ != noErr) {
    A2_LOG_ERROR(fmt("AppleTLS: Failed to set credentials: %s",
                     getLastErrorString().c_str()));
    state_ = st_error;
    return;
  }

#ifndef CIPHER_NO_DHPARAM
  lastError_ = SSLSetDiffieHellmanParams(sslCtx_, dhparam, sizeof(dhparam));
  if (lastError_ != noErr) {
    A2_LOG_WARN(fmt("AppleTLS: Failed to set DHParams: %s",
                    getLastErrorString().c_str()));
    // Engine will still generate some for us, so this is no problem, except
    // it will take longer.
  }
#endif // CIPHER_NO_DHPARAM
}

AppleTLSSession::~AppleTLSSession()
{
  closeConnection();
  if (sslCtx_) {
#if defined(__MAC_10_8)
    CFRelease(sslCtx_);
#else
    SSLDisposeContext(sslCtx_);
#endif
    sslCtx_ = nullptr;
  }
  state_ = st_error;
}

int AppleTLSSession::init(sock_t sockfd)
{
  if (state_ != st_constructed) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }

  sockfd_ = sockfd;
  lastError_ = SSLSetIOFuncs(sslCtx_, SocketRead, SocketWrite);
  if (lastError_ != noErr) {
    state_ = st_error;
    return TLS_ERR_ERROR;
  }

  lastError_ = SSLSetConnection(sslCtx_, this);
  if (lastError_ != noErr) {
    state_ = st_error;
    return TLS_ERR_ERROR;
  }

  state_ = st_initialized;
  return TLS_ERR_OK;
}

int AppleTLSSession::setSNIHostname(const std::string& hostname)
{
  if (state_ != st_initialized) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }

  lastError_ =
      SSLSetPeerDomainName(sslCtx_, hostname.c_str(), hostname.length());
  return (lastError_ != noErr) ? TLS_ERR_ERROR : TLS_ERR_OK;
}

int AppleTLSSession::closeConnection()
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }

  lastError_ = SSLClose(sslCtx_);
  state_ = st_closed;
  return lastError_ == noErr ? TLS_ERR_OK : TLS_ERR_ERROR;
}

int AppleTLSSession::checkDirection()
{
  // See: https://github.com/aria2/aria2/pull/61#issuecomment-16051793
  if (state_ == st_connected) {
    // Need to check read first, as SocketCore kinda expects this
    size_t buffered;
    lastError_ = SSLGetBufferedReadSize(sslCtx_, &buffered);
    if (lastError_ == noErr && buffered) {
      return TLS_WANT_READ;
    }
  }

  if (writeBuffered_) {
    return TLS_WANT_WRITE;
  }

  // Default to WANT_READ, as SocketCore kinda expects this
  return TLS_WANT_READ;
}

ssize_t AppleTLSSession::writeData(const void* data, size_t len)
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }

  size_t processed = 0;
  if (writeBuffered_) {
    lastError_ = SSLWrite(sslCtx_, nullptr, 0, &processed);
    switch (lastError_) {
    case noErr:
      processed = writeBuffered_;
      writeBuffered_ = 0;
      return processed;

    case errSSLWouldBlock:
      return TLS_ERR_WOULDBLOCK;

    case errSSLClosedGraceful:
    case errSSLClosedNoNotify:
      closeConnection();
      return TLS_ERR_ERROR;

    default:
      closeConnection();
      state_ = st_error;
      return TLS_ERR_ERROR;
    }
  }

  lastError_ = SSLWrite(sslCtx_, data, len, &processed);
  switch (lastError_) {
  case noErr:
    return processed;

  case errSSLWouldBlock:
    writeBuffered_ = len;
    return TLS_ERR_WOULDBLOCK;

  case errSSLClosedGraceful:
  case errSSLClosedNoNotify:
    closeConnection();
    return TLS_ERR_ERROR;

  default:
    closeConnection();
    state_ = st_error;
    return TLS_ERR_ERROR;
  }
}

OSStatus AppleTLSSession::sockWrite(const void* data, size_t* len)
{
  size_t remain = *len;
  const uint8_t* buffer = static_cast<const uint8_t*>(data);
  *len = 0;
  while (remain) {
    ssize_t w = write(sockfd_, buffer, remain);
    if (w <= 0) {
      switch (errno) {
      case EAGAIN:
        return errSSLWouldBlock;

      default:
        return errSSLClosedAbort;
      }
    }
    remain -= w;
    buffer += w;
    *len += w;
  }
  return noErr;
}

ssize_t AppleTLSSession::readData(void* data, size_t len)
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }

  size_t processed = 0;
  lastError_ = SSLRead(sslCtx_, data, len, &processed);
  switch (lastError_) {
  case noErr:
    return processed;

  case errSSLWouldBlock:
    if (processed) {
      return processed;
    }
    return TLS_ERR_WOULDBLOCK;

  case errSSLClosedGraceful:
  case errSSLClosedNoNotify:
    closeConnection();
    return TLS_ERR_ERROR;

  default:
    closeConnection();
    state_ = st_error;
    return TLS_ERR_ERROR;
  }
}

OSStatus AppleTLSSession::sockRead(void* data, size_t* len)
{
  size_t remain = *len;
  uint8_t* buffer = static_cast<uint8_t*>(data);
  *len = 0;
  while (remain) {
    ssize_t r = read(sockfd_, buffer, remain);
    if (r == 0) {
      return errSSLClosedGraceful;
    }
    if (r < 0) {
      switch (errno) {
      case ENOENT:
        return errSSLClosedGraceful;

      case ECONNRESET:
        return errSSLClosedAbort;

      case EAGAIN:
        return errSSLWouldBlock;

      default:
        return errSSLClosedAbort;
      }
    }
    remain -= r;
    buffer += r;
    *len += r;
  }
  return noErr;
}

int AppleTLSSession::tlsConnect(const std::string& hostname,
                                TLSVersion& version, std::string& handshakeErr)
{
  if (state_ != st_initialized) {
    return TLS_ERR_ERROR;
  }

  if (!hostname.empty()) {
    setSNIHostname(hostname);
  }
  lastError_ = SSLHandshake(sslCtx_);
  switch (lastError_) {
  case noErr:
    break;

  case errSSLWouldBlock:
    return TLS_ERR_WOULDBLOCK;

  case errSSLServerAuthCompleted:
    return tlsConnect(hostname, version, handshakeErr);

  default:
    handshakeErr = getLastErrorString();
    state_ = st_error;
    return TLS_ERR_ERROR;
  }

  state_ = st_connected;

  SSLProtocol proto = kSSLProtocolUnknown;
  (void)SSLGetNegotiatedProtocolVersion(sslCtx_, &proto);
  SSLCipherSuite suite = SSL_NO_SUCH_CIPHERSUITE;
  (void)SSLGetNegotiatedCipher(sslCtx_, &suite);
  A2_LOG_INFO(fmt("AppleTLS: Connected to %s with %s (%s)", hostname.c_str(),
                  protoToString(proto), suiteToString(suite).c_str()));

  switch (proto) {
  case kTLSProtocol11:
    version = TLS_PROTO_TLS11;
    break;
  case kTLSProtocol12:
    version = TLS_PROTO_TLS12;
    break;
  default:
    version = TLS_PROTO_NONE;
    break;
  }

  return TLS_ERR_OK;
}

int AppleTLSSession::tlsAccept(TLSVersion& version)
{
  std::string hostname, err;
  return tlsConnect(hostname, version, err);
}

std::string AppleTLSSession::getLastErrorString()
{
  switch (lastError_) {
  case errSSLProtocol:
    return "Protocol error";

  case errSSLNegotiation:
    return "No common cipher suites";

  case errSSLFatalAlert:
    return "Received fatal alert";

  case errSSLSessionNotFound:
    return "Unknown session";

  case errSSLClosedGraceful:
    return "Closed gracefully";

  case errSSLClosedAbort:
    return "Connection aborted";

  case errSSLXCertChainInvalid:
    return "Invalid certificate chain";

  case errSSLBadCert:
    return "Invalid certificate format";

  case errSSLCrypto:
    return "Cryptographic error";

  case paramErr:
  case errSSLInternal:
    return "Internal SSL error";

  case errSSLUnknownRootCert:
    return "Self-signed certificate";

  case errSSLNoRootCert:
    return "No root certificate";

  case errSSLCertExpired:
    return "Certificate expired";

  case errSSLCertNotYetValid:
    return "Certificate not yet valid";

  case errSSLClosedNoNotify:
    return "Closed without notification";

  case errSSLBufferOverflow:
    return "Buffer not large enough";

  case errSSLBadCipherSuite:
    return "Bad cipher suite";

  case errSSLPeerUnexpectedMsg:
    return "Unexpected peer message";

  case errSSLPeerBadRecordMac:
    return "Bad MAC";

  case errSSLPeerDecryptionFail:
    return "Decryption failure";

  case errSSLHostNameMismatch:
    return "Invalid hostname";

  case errSSLConnectionRefused:
    return "Connection refused";

  default:
    return fmt("Unspecified error %ld", (long)lastError_);
  }
}

} // namespace aria2
