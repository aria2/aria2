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
#include "StringFormat.h"
#include "message.h"

namespace aria2 {

TLSContext::TLSContext():_sslCtx(0),
                         _peerVerificationEnabled(false),
                         _logger(LogFactory::getInstance())
{
  _sslCtx = SSL_CTX_new(SSLv23_client_method());
  if(_sslCtx) {
    _good = true;
  } else {
    _good = false;
    _logger->error("SSL_CTX_new() failed. Cause: %s",
                   ERR_error_string(ERR_get_error(), 0));
  }
  SSL_CTX_set_mode(_sslCtx, SSL_MODE_AUTO_RETRY);
}

TLSContext::~TLSContext()
{
  SSL_CTX_free(_sslCtx);
}

bool TLSContext::good() const
{
  return _good;
}

bool TLSContext::bad() const
{
  return !_good;
}

bool TLSContext::addClientKeyFile(const std::string& certfile,
                                  const std::string& keyfile)
{
  if(SSL_CTX_use_PrivateKey_file(_sslCtx, keyfile.c_str(),
                                 SSL_FILETYPE_PEM) != 1) {
    _logger->error("Failed to load client private key from %s. Cause: %s",
                   keyfile.c_str(), ERR_error_string(ERR_get_error(), 0));
    return false;
  }
  if(SSL_CTX_use_certificate_chain_file(_sslCtx, certfile.c_str()) != 1) {
    _logger->error("Failed to load client certificate from %s. Cause: %s",
                   certfile.c_str(), ERR_error_string(ERR_get_error(), 0));
    return false;
  }
  _logger->info("Client Key File(cert=%s, key=%s) were successfully added.",
                certfile.c_str(), keyfile.c_str());
  return true;
}

bool TLSContext::addTrustedCACertFile(const std::string& certfile)
{
  if(SSL_CTX_load_verify_locations(_sslCtx, certfile.c_str(), 0) != 1) {
    _logger->error(MSG_LOADING_TRUSTED_CA_CERT_FAILED,
                   certfile.c_str(), ERR_error_string(ERR_get_error(), 0));
    return false;
  } else {
    _logger->info("Trusted CA certificates were successfully added.");
    return true;
  }
}

void TLSContext::enablePeerVerification()
{
  _peerVerificationEnabled = true;
}

void TLSContext::disablePeerVerification()
{
  _peerVerificationEnabled = false;
}

} // namespace aria2
