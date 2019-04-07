/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "LibsslTLSSession.h"

#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "LogFactory.h"
#include "util.h"
#include "SocketCore.h"

namespace aria2 {

#if !OPENSSL_101_API
namespace {
const unsigned char* ASN1_STRING_get0_data(ASN1_STRING* x)
{
  return ASN1_STRING_data(x);
}
} // namespace
#endif // !OPENSSL_101_API

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new OpenSSLTLSSession(static_cast<OpenSSLTLSContext*>(ctx));
}

OpenSSLTLSSession::OpenSSLTLSSession(OpenSSLTLSContext* tlsContext)
    : ssl_(nullptr), tlsContext_(tlsContext), rv_(1)
{
}

OpenSSLTLSSession::~OpenSSLTLSSession()
{
  if (ssl_) {
    SSL_free(ssl_);
  }
}

int OpenSSLTLSSession::init(sock_t sockfd)
{
  ERR_clear_error();
  ssl_ = SSL_new(tlsContext_->getSSLCtx());
  if (!ssl_) {
    return TLS_ERR_ERROR;
  }
  rv_ = SSL_set_fd(ssl_, sockfd);
  if (rv_ == 0) {
    return TLS_ERR_ERROR;
  }
  return TLS_ERR_OK;
}

int OpenSSLTLSSession::setSNIHostname(const std::string& hostname)
{
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
  ERR_clear_error();
  // TLS extensions: SNI.  There is not documentation about the
  // return code for this function (actually this is macro
  // wrapping SSL_ctrl at the time of this writing).
  SSL_set_tlsext_host_name(ssl_, hostname.c_str());
#endif // SSL_CTRL_SET_TLSEXT_HOSTNAME
  return TLS_ERR_OK;
}

int OpenSSLTLSSession::closeConnection()
{
  ERR_clear_error();
  SSL_shutdown(ssl_);
  // TODO handle return value
  return TLS_ERR_OK;
}

int OpenSSLTLSSession::checkDirection()
{
  int error = SSL_get_error(ssl_, rv_);
  if (error == SSL_ERROR_WANT_WRITE) {
    return TLS_WANT_WRITE;
  }
  else {
    // TODO We ignore error other than SSL_ERR_WANT_READ here for now
    return TLS_WANT_READ;
  }
}

namespace {
bool wouldblock(SSL* ssl, int rv)
{
  int error = SSL_get_error(ssl, rv);
  return error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE;
}
} // namespace

ssize_t OpenSSLTLSSession::writeData(const void* data, size_t len)
{
  ERR_clear_error();
  rv_ = SSL_write(ssl_, data, len);
  if (rv_ <= 0) {
    if (wouldblock(ssl_, rv_)) {
      return TLS_ERR_WOULDBLOCK;
    }
    else {
      return TLS_ERR_ERROR;
    }
  }
  else {
    ssize_t ret = rv_;
    rv_ = 1;
    return ret;
  }
}

ssize_t OpenSSLTLSSession::readData(void* data, size_t len)
{
  ERR_clear_error();
  rv_ = SSL_read(ssl_, data, len);
  if (rv_ <= 0) {
    if (wouldblock(ssl_, rv_)) {
      return TLS_ERR_WOULDBLOCK;
    }

    if (rv_ == 0) {
      auto err = SSL_get_error(ssl_, rv_);

      if (err == SSL_ERROR_ZERO_RETURN) {
        return 0;
      }
    }

    return TLS_ERR_ERROR;
  }

  ssize_t ret = rv_;
  rv_ = 1;
  return ret;
}

int OpenSSLTLSSession::handshake(TLSVersion& version)
{
  ERR_clear_error();
  if (tlsContext_->getSide() == TLS_CLIENT) {
    rv_ = SSL_connect(ssl_);
  }
  else {
    rv_ = SSL_accept(ssl_);
  }
  if (rv_ <= 0) {
    int sslError = SSL_get_error(ssl_, rv_);
    switch (sslError) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_X509_LOOKUP:
    case SSL_ERROR_ZERO_RETURN:
      // TODO Now assume we are doing non-blocking. Then above 2
      // errors are OK.
      break;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      return TLS_ERR_WOULDBLOCK;
    default:
      return TLS_ERR_ERROR;
    }
  }

  switch (SSL_version(ssl_)) {
#ifdef TLS1_1_VERSION
  case TLS1_1_VERSION:
    version = TLS_PROTO_TLS11;
    break;
#endif // TLS1_1_VERSION

#ifdef TLS1_2_VERSION
  case TLS1_2_VERSION:
    version = TLS_PROTO_TLS12;
    break;
#endif // TLS1_2_VERSION

#ifdef TLS1_3_VERSION
  case TLS1_3_VERSION:
    version = TLS_PROTO_TLS13;
    break;
#endif // TLS1_3_VERSION

  default:
    version = TLS_PROTO_NONE;
    break;
  }

  return TLS_ERR_OK;
}

int OpenSSLTLSSession::tlsConnect(const std::string& hostname,
                                  TLSVersion& version,
                                  std::string& handshakeErr)
{
  handshakeErr = "";
  int ret;
  ret = handshake(version);
  if (ret != TLS_ERR_OK) {
    return ret;
  }
  if (tlsContext_->getSide() == TLS_CLIENT && tlsContext_->getVerifyPeer()) {
    // verify peer
    X509* peerCert = SSL_get_peer_certificate(ssl_);
    if (!peerCert) {
      handshakeErr = "certificate not found";
      return TLS_ERR_ERROR;
    }
    std::unique_ptr<X509, decltype(&X509_free)> certDeleter(peerCert,
                                                            X509_free);
    long verifyResult = SSL_get_verify_result(ssl_);
    if (verifyResult != X509_V_OK) {
      handshakeErr = X509_verify_cert_error_string(verifyResult);
      return TLS_ERR_ERROR;
    }
    std::string commonName;
    std::vector<std::string> dnsNames;
    std::vector<std::string> ipAddrs;
    GENERAL_NAMES* altNames;
    altNames = reinterpret_cast<GENERAL_NAMES*>(
        X509_get_ext_d2i(peerCert, NID_subject_alt_name, nullptr, NULL));
    if (altNames) {
      std::unique_ptr<GENERAL_NAMES, decltype(&GENERAL_NAMES_free)>
          altNamesDeleter(altNames, GENERAL_NAMES_free);
      size_t n = sk_GENERAL_NAME_num(altNames);
      for (size_t i = 0; i < n; ++i) {
        const GENERAL_NAME* altName = sk_GENERAL_NAME_value(altNames, i);
        if (altName->type == GEN_DNS) {
          auto name = ASN1_STRING_get0_data(altName->d.ia5);
          if (!name) {
            continue;
          }
          size_t len = ASN1_STRING_length(altName->d.ia5);
          if (len == 0) {
            continue;
          }
          if (name[len - 1] == '.') {
            --len;
            if (len == 0) {
              continue;
            }
          }
          dnsNames.push_back(std::string(name, name + len));
        }
        else if (altName->type == GEN_IPADD) {
          const unsigned char* ipAddr = altName->d.iPAddress->data;
          if (!ipAddr) {
            continue;
          }
          size_t len = altName->d.iPAddress->length;
          ipAddrs.push_back(
              std::string(reinterpret_cast<const char*>(ipAddr), len));
        }
      }
    }
    X509_NAME* subjectName = X509_get_subject_name(peerCert);
    if (!subjectName) {
      handshakeErr = "could not get X509 name object from the certificate.";
      return TLS_ERR_ERROR;
    }
    int lastpos = -1;
    while (1) {
      lastpos =
          X509_NAME_get_index_by_NID(subjectName, NID_commonName, lastpos);
      if (lastpos == -1) {
        break;
      }
      X509_NAME_ENTRY* entry = X509_NAME_get_entry(subjectName, lastpos);
      unsigned char* out;
      int outlen = ASN1_STRING_to_UTF8(&out, X509_NAME_ENTRY_get_data(entry));
      if (outlen < 0) {
        continue;
      }
      if (outlen == 0) {
        OPENSSL_free(out);
        continue;
      }
      if (out[outlen - 1] == '.') {
        --outlen;
        if (outlen == 0) {
          OPENSSL_free(out);
          continue;
        }
      }
      commonName.assign(&out[0], &out[outlen]);
      OPENSSL_free(out);
      break;
    }
    if (!net::verifyHostname(hostname, dnsNames, ipAddrs, commonName)) {
      handshakeErr = "hostname does not match";
      return TLS_ERR_ERROR;
    }
  }

  return TLS_ERR_OK;
}

int OpenSSLTLSSession::tlsAccept(TLSVersion& version)
{
  return handshake(version);
}

std::string OpenSSLTLSSession::getLastErrorString()
{
  if (rv_ <= 0) {
    int sslError = SSL_get_error(ssl_, rv_);
    switch (sslError) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_X509_LOOKUP:
    case SSL_ERROR_ZERO_RETURN:
      return "";
    case SSL_ERROR_SYSCALL: {
      int err = ERR_get_error();
      if (err == 0) {
        if (rv_ == 0) {
          return "EOF was received";
        }
        else if (rv_ == -1) {
          return "SSL I/O error";
        }
        else {
          return "unknown syscall error";
        }
      }
      else {
        return ERR_error_string(err, nullptr);
      }
    }
    case SSL_ERROR_SSL:
      return "protocol error";
    default:
      return "unknown error";
    }
  }
  else {
    return "";
  }
}

} // namespace aria2
