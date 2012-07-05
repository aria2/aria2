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
#include "LibgnutlsTLSContext.h"

#ifdef HAVE_LIBGNUTLS
# include <gnutls/x509.h>
#endif // HAVE_LIBGNUTLS

#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"
#include "message.h"

namespace aria2 {

TLSContext::TLSContext()
  : certCred_(0),
    peerVerificationEnabled_(false)
{
  int r = gnutls_certificate_allocate_credentials(&certCred_);
  if(r == GNUTLS_E_SUCCESS) {
    good_ = true;
    gnutls_certificate_set_verify_flags(certCred_,
                                        GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
  } else {
    good_ =false;
    A2_LOG_ERROR(fmt("gnutls_certificate_allocate_credentials() failed."
                     " Cause: %s",
                     gnutls_strerror(r)));
  }
}

TLSContext::~TLSContext()
{
  if(certCred_) {
    gnutls_certificate_free_credentials(certCred_);
  }
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
  int ret = gnutls_certificate_set_x509_key_file(certCred_,
                                                 certfile.c_str(),
                                                 keyfile.c_str(),
                                                 GNUTLS_X509_FMT_PEM);
  if(ret == GNUTLS_E_SUCCESS) {
    A2_LOG_INFO(fmt("Client Key File(cert=%s, key=%s) were successfully added.",
                    certfile.c_str(), keyfile.c_str()));
    return true;
  } else {
    A2_LOG_ERROR(fmt("Failed to load client certificate from %s and"
                     " private key from %s. Cause: %s",
                     certfile.c_str(), keyfile.c_str(),
                     gnutls_strerror(ret)));
    return false;
  }
}

bool TLSContext::addSystemTrustedCACerts()
{
#ifdef HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST
  int ret = gnutls_certificate_set_x509_system_trust(certCred_);
  if(ret < 0) {
    A2_LOG_ERROR(fmt(MSG_LOADING_SYSTEM_TRUSTED_CA_CERTS_FAILED,
                     gnutls_strerror(ret)));
    return false;
  } else {
    A2_LOG_INFO(fmt("%d certificate(s) were imported.", ret));
    return true;
  }
#else
  return false;
#endif
}

bool TLSContext::addTrustedCACertFile(const std::string& certfile)
{
  int ret = gnutls_certificate_set_x509_trust_file(certCred_,
                                                   certfile.c_str(),
                                                   GNUTLS_X509_FMT_PEM);
  if(ret < 0) {
    A2_LOG_ERROR(fmt(MSG_LOADING_TRUSTED_CA_CERT_FAILED,
                     certfile.c_str(), gnutls_strerror(ret)));
    return false;
  } else {
    A2_LOG_INFO(fmt("%d certificate(s) were imported.", ret));
    return true;
  }
}

gnutls_certificate_credentials_t TLSContext::getCertCred() const
{
  return certCred_;
}

void TLSContext::enablePeerVerification()
{
  peerVerificationEnabled_ = true;
}

void TLSContext::disablePeerVerification()
{
  peerVerificationEnabled_ = false;
}

bool TLSContext::peerVerificationEnabled() const
{
  return peerVerificationEnabled_;
}

} // namespace aria2
