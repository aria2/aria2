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
#include "LibsslTLSContext.h"

#include <openssl/err.h>

#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"
#include "message.h"

namespace aria2 {

TLSContext::TLSContext()
  : sslCtx_(0),
    peerVerificationEnabled_(false)
{
  sslCtx_ = SSL_CTX_new(SSLv23_client_method());
  if(sslCtx_) {
    good_ = true;
  } else {
    good_ = false;
    A2_LOG_ERROR(fmt("SSL_CTX_new() failed. Cause: %s",
                     ERR_error_string(ERR_get_error(), 0)));
  }
  /* Disable SSLv2 and enable all workarounds for buggy servers */
  SSL_CTX_set_options(sslCtx_, SSL_OP_ALL|SSL_OP_NO_SSLv2);
  SSL_CTX_set_mode(sslCtx_, SSL_MODE_AUTO_RETRY);
  #ifdef SSL_MODE_RELEASE_BUFFERS
  /* keep memory usage low */
  SSL_CTX_set_mode(sslCtx_, SSL_MODE_RELEASE_BUFFERS);
  #endif
  
}

TLSContext::~TLSContext()
{
  SSL_CTX_free(sslCtx_);
}

bool TLSContext::good() const
{
  return good_;
}

bool TLSContext::bad() const
{
  return !good_;
}

bool TLSContext::addClientKeyFile(const std::string& certfile,
                                  const std::string& keyfile)
{
  if(SSL_CTX_use_PrivateKey_file(sslCtx_, keyfile.c_str(),
                                 SSL_FILETYPE_PEM) != 1) {
    A2_LOG_ERROR(fmt("Failed to load client private key from %s. Cause: %s",
                     keyfile.c_str(),
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  }
  if(SSL_CTX_use_certificate_chain_file(sslCtx_, certfile.c_str()) != 1) {
    A2_LOG_ERROR(fmt("Failed to load client certificate from %s. Cause: %s",
                     certfile.c_str(),
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  }
  A2_LOG_INFO(fmt("Client Key File(cert=%s, key=%s) were successfully added.",
                  certfile.c_str(),
                  keyfile.c_str()));
  return true;
}

bool TLSContext::addSystemTrustedCACerts()
{
  if(SSL_CTX_set_default_verify_paths(sslCtx_) != 1) {
    A2_LOG_ERROR(fmt(MSG_LOADING_SYSTEM_TRUSTED_CA_CERTS_FAILED,
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  } else {
    A2_LOG_INFO("System trusted CA certificates were successfully added.");
    return true;
  }
}

bool TLSContext::addTrustedCACertFile(const std::string& certfile)
{
  if(SSL_CTX_load_verify_locations(sslCtx_, certfile.c_str(), 0) != 1) {
    A2_LOG_ERROR(fmt(MSG_LOADING_TRUSTED_CA_CERT_FAILED,
                     certfile.c_str(),
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  } else {
    A2_LOG_INFO("Trusted CA certificates were successfully added.");
    return true;
  }
}

void TLSContext::enablePeerVerification()
{
  peerVerificationEnabled_ = true;
}

void TLSContext::disablePeerVerification()
{
  peerVerificationEnabled_ = false;
}

} // namespace aria2
