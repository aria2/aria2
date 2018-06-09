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

#include <sstream>

#ifdef HAVE_LIBGNUTLS
#  include <gnutls/x509.h>
#  include <gnutls/pkcs12.h>
#endif // HAVE_LIBGNUTLS

#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"
#include "message.h"
#include "BufferedFile.h"

namespace aria2 {

TLSContext* TLSContext::make(TLSSessionSide side, TLSVersion ver)
{
  return new GnuTLSContext(side, ver);
}

GnuTLSContext::GnuTLSContext(TLSSessionSide side, TLSVersion ver)
    : certCred_(0), side_(side), minTLSVer_(ver), verifyPeer_(true)
{
  int r = gnutls_certificate_allocate_credentials(&certCred_);
  if (r == GNUTLS_E_SUCCESS) {
    good_ = true;
    gnutls_certificate_set_verify_flags(certCred_,
                                        GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
  }
  else {
    good_ = false;
    A2_LOG_ERROR(fmt("gnutls_certificate_allocate_credentials() failed."
                     " Cause: %s",
                     gnutls_strerror(r)));
  }
}

GnuTLSContext::~GnuTLSContext()
{
  if (certCred_) {
    gnutls_certificate_free_credentials(certCred_);
  }
}

bool GnuTLSContext::good() const { return good_; }

bool GnuTLSContext::addCredentialFile(const std::string& certfile,
                                      const std::string& keyfile)
{
  if (keyfile.empty()) {
    return addP12CredentialFile(certfile);
  }
  int ret = gnutls_certificate_set_x509_key_file(
      certCred_, certfile.c_str(), keyfile.c_str(), GNUTLS_X509_FMT_PEM);
  if (ret == GNUTLS_E_SUCCESS) {
    A2_LOG_INFO(
        fmt("Credential files(cert=%s, key=%s) were successfully added.",
            certfile.c_str(), keyfile.c_str()));
    return true;
  }
  else {
    A2_LOG_ERROR(fmt("Failed to load certificate from %s and"
                     " private key from %s. Cause: %s",
                     certfile.c_str(), keyfile.c_str(), gnutls_strerror(ret)));
    return false;
  }
}
bool GnuTLSContext::addP12CredentialFile(const std::string& p12file)
{
  std::stringstream ss;
  BufferedFile(p12file.c_str(), BufferedFile::READ).transfer(ss);
  auto datastr = ss.str();
  const gnutls_datum_t data = {(unsigned char*)datastr.c_str(),
                               (unsigned int)datastr.length()};
  int err = gnutls_certificate_set_x509_simple_pkcs12_mem(
      certCred_, &data, GNUTLS_X509_FMT_DER, "");
  if (err != GNUTLS_E_SUCCESS) {
    if (side_ == TLS_SERVER) {
      A2_LOG_ERROR("Failed to import PKCS12 file. "
                   "If you meant to use PEM, you'll also have to specify "
                   "--rpc-private-key. See the manual.");
    }
    else {
      A2_LOG_ERROR("Failed to import PKCS12 file. "
                   "If you meant to use PEM, you'll also have to specify "
                   "--private-key. See the manual.");
    }
    return false;
  }
  return true;
}

bool GnuTLSContext::addSystemTrustedCACerts()
{
#ifdef HAVE_GNUTLS_CERTIFICATE_SET_X509_SYSTEM_TRUST
  int ret = gnutls_certificate_set_x509_system_trust(certCred_);
  if (ret < 0) {
    A2_LOG_INFO(
        fmt(MSG_LOADING_SYSTEM_TRUSTED_CA_CERTS_FAILED, gnutls_strerror(ret)));
    return false;
  }
  else {
    A2_LOG_INFO(fmt("%d certificate(s) were imported.", ret));
    return true;
  }
#else
  A2_LOG_INFO("System certificates not supported");
  return false;
#endif
}

bool GnuTLSContext::addTrustedCACertFile(const std::string& certfile)
{
  int ret = gnutls_certificate_set_x509_trust_file(certCred_, certfile.c_str(),
                                                   GNUTLS_X509_FMT_PEM);
  if (ret < 0) {
    A2_LOG_ERROR(fmt(MSG_LOADING_TRUSTED_CA_CERT_FAILED, certfile.c_str(),
                     gnutls_strerror(ret)));
    return false;
  }
  else {
    A2_LOG_INFO(fmt("%d certificate(s) were imported.", ret));
    return true;
  }
}

gnutls_certificate_credentials_t GnuTLSContext::getCertCred() const
{
  return certCred_;
}

} // namespace aria2
