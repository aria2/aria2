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
#include "LibgnutlsTLSSession.h"

#include <gnutls/x509.h>

#include "TLSContext.h"
#include "util.h"
#include "SocketCore.h"

namespace aria2 {

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new GnuTLSSession(static_cast<GnuTLSContext*>(ctx));
}

GnuTLSSession::GnuTLSSession(GnuTLSContext* tlsContext)
  : sslSession_(0),
    tlsContext_(tlsContext),
    rv_(0)
{}

GnuTLSSession::~GnuTLSSession()
{
  if(sslSession_) {
    gnutls_deinit(sslSession_);
  }
}

int GnuTLSSession::init(sock_t sockfd)
{
  rv_ = gnutls_init(&sslSession_,
                    tlsContext_->getSide() == TLS_CLIENT ?
                    GNUTLS_CLIENT : GNUTLS_SERVER);
  if(rv_ != GNUTLS_E_SUCCESS) {
    return TLS_ERR_ERROR;
  }
  // It seems err is not error message, but the argument string
  // which causes syntax error.
  const char* err;
  rv_ = gnutls_priority_set_direct(sslSession_, "NORMAL", &err);
  if(rv_ != GNUTLS_E_SUCCESS) {
    return TLS_ERR_ERROR;
  }
  // put the x509 credentials to the current session
  rv_ = gnutls_credentials_set(sslSession_, GNUTLS_CRD_CERTIFICATE,
                               tlsContext_->getCertCred());
  if(rv_ != GNUTLS_E_SUCCESS) {
    return TLS_ERR_ERROR;
  }
  // TODO Consider to use gnutls_transport_set_int() for GNUTLS 3.1.9
  // or later
  gnutls_transport_set_ptr(sslSession_,
                           (gnutls_transport_ptr_t)(ptrdiff_t)sockfd);
  return TLS_ERR_OK;
}

int GnuTLSSession::setSNIHostname(const std::string& hostname)
{
  // TLS extensions: SNI
  rv_ = gnutls_server_name_set(sslSession_, GNUTLS_NAME_DNS,
                               hostname.c_str(), hostname.size());
  if(rv_ != GNUTLS_E_SUCCESS) {
    return TLS_ERR_ERROR;
  }
  return TLS_ERR_OK;
}

int GnuTLSSession::closeConnection()
{
  rv_ = gnutls_bye(sslSession_, GNUTLS_SHUT_WR);
  if(rv_ == GNUTLS_E_SUCCESS) {
    return TLS_ERR_OK;
  } else if(rv_ == GNUTLS_E_AGAIN) {
    return TLS_ERR_WOULDBLOCK;
  } else {
    return TLS_ERR_ERROR;
  }
}

int GnuTLSSession::checkDirection()
{
  int direction = gnutls_record_get_direction(sslSession_);
  return direction == 0 ? TLS_WANT_READ : TLS_WANT_WRITE;
}

ssize_t GnuTLSSession::writeData(const void* data, size_t len)
{
  while((rv_ = gnutls_record_send(sslSession_, data, len)) ==
        GNUTLS_E_INTERRUPTED);
  if(rv_ >= 0) {
    ssize_t ret = rv_;
    rv_ = 0;
    return ret;
  } else if(rv_ == GNUTLS_E_AGAIN) {
    return TLS_ERR_WOULDBLOCK;
  } else {
    return TLS_ERR_ERROR;
  }
}

ssize_t GnuTLSSession::readData(void* data, size_t len)
{
  while((rv_ = gnutls_record_recv(sslSession_, data, len)) ==
        GNUTLS_E_INTERRUPTED);
  if(rv_ >= 0) {
    ssize_t ret = rv_;
    rv_ = 0;
    return ret;
  } else if(rv_ == GNUTLS_E_AGAIN) {
    return TLS_ERR_WOULDBLOCK;
  } else {
    return TLS_ERR_ERROR;
  }
}

int GnuTLSSession::tlsConnect(const std::string& hostname,
                           std::string& handshakeErr)
{
  handshakeErr = "";
  rv_ = gnutls_handshake(sslSession_);
  if(rv_ < 0) {
    if(rv_ == GNUTLS_E_AGAIN) {
      return TLS_ERR_WOULDBLOCK;
    } else {
      return TLS_ERR_ERROR;
    }
  }
  if(tlsContext_->getVerifyPeer()) {
    // verify peer
    unsigned int status;
    rv_ = gnutls_certificate_verify_peers2(sslSession_, &status);
    if(rv_ != GNUTLS_E_SUCCESS) {
      return TLS_ERR_ERROR;
    }
    if(status) {
      handshakeErr = "";
      if(status & GNUTLS_CERT_INVALID) {
        handshakeErr += " `not signed by known authorities or invalid'";
      }
      if(status & GNUTLS_CERT_REVOKED) {
        handshakeErr += " `revoked by its CA'";
      }
      if(status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
        handshakeErr += " `issuer is not known'";
      }
      // TODO should check GNUTLS_CERT_SIGNER_NOT_CA ?
      if(status & GNUTLS_CERT_INSECURE_ALGORITHM) {
        handshakeErr += " `insecure algorithm'";
      }
      if(status & GNUTLS_CERT_NOT_ACTIVATED) {
        handshakeErr += " `not activated yet'";
      }
      if(status & GNUTLS_CERT_EXPIRED) {
        handshakeErr += " `expired'";
      }
      // TODO Add GNUTLS_CERT_SIGNATURE_FAILURE here
      if(!handshakeErr.empty()) {
        return TLS_ERR_ERROR;
      }
    }
    // certificate type: only X509 is allowed.
    if(gnutls_certificate_type_get(sslSession_) != GNUTLS_CRT_X509) {
      handshakeErr = "certificate type must be X509";
      return TLS_ERR_ERROR;
    }
    unsigned int peerCertsLength;
    const gnutls_datum_t* peerCerts;
    peerCerts = gnutls_certificate_get_peers(sslSession_, &peerCertsLength);
    if(!peerCerts || peerCertsLength == 0 ) {
      handshakeErr = "certificate not found";
      return TLS_ERR_ERROR;
    }
    gnutls_x509_crt_t cert;
    rv_ = gnutls_x509_crt_init(&cert);
    if(rv_ != GNUTLS_E_SUCCESS) {
      return TLS_ERR_ERROR;
    }
    auto_delete<gnutls_x509_crt_t> certDeleter(cert, gnutls_x509_crt_deinit);
    rv_ = gnutls_x509_crt_import(cert, &peerCerts[0], GNUTLS_X509_FMT_DER);
    if(rv_ != GNUTLS_E_SUCCESS) {
      return TLS_ERR_ERROR;
    }
    std::string commonName;
    std::vector<std::string> dnsNames;
    std::vector<std::string> ipAddrs;
    int ret = 0;
    char altName[256];
    size_t altNameLen;
    for(int i = 0; !(ret < 0); ++i) {
      altNameLen = sizeof(altName);
      ret = gnutls_x509_crt_get_subject_alt_name(cert, i, altName,
                                                 &altNameLen, 0);
      if(ret == GNUTLS_SAN_DNSNAME) {
        dnsNames.push_back(std::string(altName, altNameLen));
      } else if(ret == GNUTLS_SAN_IPADDRESS) {
        ipAddrs.push_back(std::string(altName, altNameLen));
      }
    }
    altNameLen = sizeof(altName);
    ret = gnutls_x509_crt_get_dn_by_oid(cert,
                                        GNUTLS_OID_X520_COMMON_NAME, 0, 0,
                                        altName, &altNameLen);
    if(ret == 0) {
      commonName.assign(altName, altNameLen);
    }
    if(!net::verifyHostname(hostname, dnsNames, ipAddrs, commonName)) {
      handshakeErr = "hostname does not match";
      return TLS_ERR_ERROR;
    }
  }
  return TLS_ERR_OK;
}

int GnuTLSSession::tlsAccept()
{
  rv_ = gnutls_handshake(sslSession_);
  if(rv_ == GNUTLS_E_SUCCESS) {
    return TLS_ERR_OK;
  } else if(rv_ == GNUTLS_E_AGAIN) {
    return TLS_ERR_WOULDBLOCK;
  } else {
    return TLS_ERR_ERROR;
  }
}

std::string GnuTLSSession::getLastErrorString()
{
  return gnutls_strerror(rv_);
}

} // namespace aria2
