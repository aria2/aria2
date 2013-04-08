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

#include <vector>

#include <CoreFoundation/CoreFoundation.h>

#include "LogFactory.h"
#include "a2functional.h"
#include "fmt.h"

#define ioErr -36
#define paramErr -50
#define errSSLServerAuthCompleted -9841

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

  static inline const char *protoToString(SSLProtocol proto) {
    switch (proto) {
      case kSSLProtocol2:
        return "SSLv2 (!)";
      case kSSLProtocol3:
        return "SSLv3";
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

#define SUITE(s) { s, #s }
  static struct {
    SSLCipherSuite suite;
    const char *name;
  } kSuites[] = {
    SUITE(SSL_NULL_WITH_NULL_NULL),
    SUITE(SSL_RSA_WITH_NULL_MD5),
    SUITE(SSL_RSA_WITH_NULL_SHA),
    SUITE(SSL_RSA_EXPORT_WITH_RC4_40_MD5),
    SUITE(SSL_RSA_WITH_RC4_128_MD5),
    SUITE(SSL_RSA_WITH_RC4_128_SHA),
    SUITE(SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5),
    SUITE(SSL_RSA_WITH_IDEA_CBC_SHA),
    SUITE(SSL_RSA_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_RSA_WITH_DES_CBC_SHA),
    SUITE(SSL_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_DH_DSS_WITH_DES_CBC_SHA),
    SUITE(SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_DH_RSA_WITH_DES_CBC_SHA),
    SUITE(SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_DHE_DSS_WITH_DES_CBC_SHA),
    SUITE(SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_DHE_RSA_WITH_DES_CBC_SHA),
    SUITE(SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_DH_anon_EXPORT_WITH_RC4_40_MD5),
    SUITE(SSL_DH_anon_WITH_RC4_128_MD5),
    SUITE(SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA),
    SUITE(SSL_DH_anon_WITH_DES_CBC_SHA),
    SUITE(SSL_DH_anon_WITH_3DES_EDE_CBC_SHA),
    SUITE(SSL_FORTEZZA_DMS_WITH_NULL_SHA),
    SUITE(SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA),
    SUITE(TLS_RSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_DH_DSS_WITH_AES_128_CBC_SHA),
    SUITE(TLS_DH_RSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_DHE_DSS_WITH_AES_128_CBC_SHA),
    SUITE(TLS_DHE_RSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_DH_anon_WITH_AES_128_CBC_SHA),
    SUITE(TLS_RSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_DH_DSS_WITH_AES_256_CBC_SHA),
    SUITE(TLS_DH_RSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_DHE_DSS_WITH_AES_256_CBC_SHA),
    SUITE(TLS_DHE_RSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_DH_anon_WITH_AES_256_CBC_SHA),
    SUITE(TLS_ECDH_ECDSA_WITH_NULL_SHA),
    SUITE(TLS_ECDH_ECDSA_WITH_RC4_128_SHA),
    SUITE(TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_ECDHE_ECDSA_WITH_NULL_SHA),
    SUITE(TLS_ECDHE_ECDSA_WITH_RC4_128_SHA),
    SUITE(TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_ECDH_RSA_WITH_NULL_SHA),
    SUITE(TLS_ECDH_RSA_WITH_RC4_128_SHA),
    SUITE(TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_ECDHE_RSA_WITH_NULL_SHA),
    SUITE(TLS_ECDHE_RSA_WITH_RC4_128_SHA),
    SUITE(TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA),
    SUITE(TLS_ECDH_anon_WITH_NULL_SHA),
    SUITE(TLS_ECDH_anon_WITH_RC4_128_SHA),
    SUITE(SSL_RSA_WITH_RC2_CBC_MD5),
    SUITE(SSL_RSA_WITH_IDEA_CBC_MD5),
    SUITE(SSL_RSA_WITH_DES_CBC_MD5),
    SUITE(SSL_RSA_WITH_3DES_EDE_CBC_MD5),

#if defined(__MAC_10_8)
    SUITE(TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_DHE_DSS_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_DHE_DSS_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_DHE_DSS_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_DHE_DSS_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_DHE_RSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_DHE_RSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_DHE_RSA_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_DHE_RSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_DH_DSS_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_DH_DSS_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_DH_DSS_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_DH_DSS_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_DH_RSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_DH_RSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_DH_RSA_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_DH_RSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_DH_anon_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_DH_anon_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_DH_anon_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_DH_anon_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_DH_anon_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_DH_anon_WITH_RC4_128_MD5),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384),
    SUITE(TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384),
    SUITE(TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384),
    SUITE(TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384),
    SUITE(TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_EMPTY_RENEGOTIATION_INFO_SCSV),
    SUITE(TLS_NULL_WITH_NULL_NULL),
    SUITE(TLS_RSA_WITH_3DES_EDE_CBC_SHA),
    SUITE(TLS_RSA_WITH_AES_128_CBC_SHA256),
    SUITE(TLS_RSA_WITH_AES_128_GCM_SHA256),
    SUITE(TLS_RSA_WITH_AES_256_CBC_SHA256),
    SUITE(TLS_RSA_WITH_AES_256_GCM_SHA384),
    SUITE(TLS_RSA_WITH_NULL_MD5),
    SUITE(TLS_RSA_WITH_NULL_SHA),
    SUITE(TLS_RSA_WITH_NULL_SHA256),
    SUITE(TLS_RSA_WITH_RC4_128_MD5),
    SUITE(TLS_RSA_WITH_RC4_128_SHA),
#endif

    SUITE(SSL_NO_SUCH_CIPHERSUITE)
  };
  static const size_t nSuites = sizeof(kSuites) / sizeof(*kSuites);
#undef SUITE

  static inline const char* suiteToString(const SSLCipherSuite suite)
  {
    for (size_t i = 0; i < nSuites; ++i) {
      if (kSuites[i].suite == suite) {
        return kSuites[i].name;
      }
    }
    return "Unknown suite";
  }

  static const char* kBlocked[] = {
    "NULL", "anon", "MD5", "EXPORT", "DES", "IDEA", "NO_SUCH", "EMPTY"
  };
  static const size_t nBlocked = sizeof(kBlocked) / sizeof(*kBlocked);

  static inline bool isBlockedSuite(SSLCipherSuite suite)
  {
    const char* name = suiteToString(suite);
    for (size_t i = 0; i < nBlocked; ++i) {
      if (strstr(name, kBlocked[i])) {
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

    rv.erase(std::remove_if(rv.begin(), rv.end(), isBlockedSuite), rv.end());
    return rv;
  }
}

namespace aria2 {

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new AppleTLSSession(static_cast<AppleTLSContext*>(ctx));
}

AppleTLSSession::AppleTLSSession(AppleTLSContext* ctx)
  : ctx_(ctx),
    sslCtx_(0),
    sockfd_(0),
    state_(st_constructed),
    lastError_(noErr),
    writeBuffered_(0)
{
  lastError_ = SSLNewContext(ctx->getSide() == TLS_SERVER, &sslCtx_) == noErr;
  if (lastError_ == noErr) {
    state_ = st_error;
    return;
  }
#if defined(__MAC_10_8)
  (void)SSLSetProtocolVersionMin(sslCtx_, kSSLProtocol3);
  (void)SSLSetProtocolVersionMax(sslCtx_, kTLSProtocol12);
#else
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kSSLProtocolAll, false);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kSSLProtocol3, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol1, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol11, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol12, true);
#endif

  (void)SSLSetEnableCertVerify(sslCtx_, ctx->getVerifyPeer());

#ifndef CIPHER_ENABLE_ALL
  SSLCipherSuiteList enabled = constructEnabledSuites(sslCtx_);
  if (enabled.empty()) {
    A2_LOG_ERROR("AppleTLS: Failed to construct enabled ciphers list");
    state_ = st_error;
    return;
  }
  for (SSLCipherSuiteList::iterator i = enabled.begin(), e = enabled.end(); i != e; ++i) {
    A2_LOG_INFO(fmt("AppleTLS: Enabled suite %s", suiteToString(*i)));
  }
  if (SSLSetEnabledCiphers(sslCtx_, &enabled[0], enabled.size()) != noErr) {
    A2_LOG_ERROR("AppleTLS: Failed to set enabled ciphers list");
    state_ = st_error;
    return;
  }
#endif

  if (ctx->getSide() == TLS_SERVER) {
    SecIdentityRef creds = ctx->getCredentials();
    if (!creds) {
      A2_LOG_ERROR("AppleTLS: No credentials");
      state_ = st_error;
      return;
    }
    CFArrayRef certs = CFArrayCreate(0, (const void**)&creds, 1, 0);
    if (!certs) {
      A2_LOG_ERROR("AppleTLS: Failed to setup credentials");
      state_ = st_error;
      return;
    }
    auto_delete<const void*> del_certs(certs, CFRelease);
    lastError_ = SSLSetCertificate(sslCtx_, certs);
    if (lastError_ != noErr) {
      A2_LOG_ERROR(fmt("AppleTLS: Failed to set credentials: %s", getLastErrorString().c_str()));
      state_ = st_error;
      return;
    }

#ifndef CIPHER_NO_DHPARAM
    lastError_ = SSLSetDiffieHellmanParams(sslCtx_, dhparam, sizeof(dhparam));
    if (lastError_ != noErr) {
      A2_LOG_WARN(fmt("AppleTLS: Failed to set DHParams: %s", getLastErrorString().c_str()));
      // Engine will still generate some for us, so this is no problem, except
      // it will take longer.
    }
#endif // CIPHER_NO_DHPARAM

  }
}

AppleTLSSession::~AppleTLSSession()
{
  closeConnection();
  if (sslCtx_) {
    SSLDisposeContext(sslCtx_);
    sslCtx_ = 0;
  }
  state_ = st_error;
}

int AppleTLSSession::init(sock_t sockfd)
{
  if (state_ != st_constructed) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
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
  sockfd_ = sockfd;
  state_ = st_initialized;
  return TLS_ERR_OK;
}

int AppleTLSSession::setSNIHostname(const std::string& hostname)
{
  if (state_ != st_initialized) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  lastError_ = SSLSetPeerDomainName(sslCtx_, hostname.c_str(), hostname.length());
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
  return lastError_ == noErr ?  TLS_ERR_OK : TLS_ERR_ERROR;
}

int AppleTLSSession::checkDirection() {
  // See: https://github.com/tatsuhiro-t/aria2/pull/61#issuecomment-16051793
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
    lastError_ = SSLWrite(sslCtx_, 0, 0, &processed);
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
  const uint8_t *buffer = static_cast<const uint8_t*>(data);
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
  uint8_t *buffer = static_cast<uint8_t*>(data);
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

int AppleTLSSession::tlsConnect(const std::string& hostname, std::string& handshakeErr)
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
      return tlsConnect(hostname, handshakeErr);
    default:
      handshakeErr = getLastErrorString();
      return TLS_ERR_ERROR;
  }
  state_ = st_connected;

  SSLProtocol proto = kSSLProtocolUnknown;
  (void)SSLGetNegotiatedProtocolVersion(sslCtx_, &proto);
  SSLCipherSuite suite = SSL_NO_SUCH_CIPHERSUITE;
  (void)SSLGetNegotiatedCipher(sslCtx_, &suite);
  A2_LOG_INFO(fmt("AppleTLS: Connected to %s with %s (%s)",
                  hostname.c_str(),
                  protoToString(proto),
                  suiteToString(suite)));

  return TLS_ERR_OK;
}

int AppleTLSSession::tlsAccept()
{
  std::string hostname, err;
  return tlsConnect(hostname, err);
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
      return fmt("Unspecified error %d", lastError_);
  }
}

}
