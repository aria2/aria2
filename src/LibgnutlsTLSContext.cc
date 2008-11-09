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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "StringFormat.h"
#include "message.h"

namespace aria2 {

TLSContext::TLSContext():_certCred(0),
			 _peerVerificationEnabled(false),
			 _logger(LogFactory::getInstance())
{
  int r = gnutls_certificate_allocate_credentials(&_certCred);
  if(r == GNUTLS_E_SUCCESS) {
    _good = true;
    gnutls_certificate_set_verify_flags(_certCred,
					GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT);
  } else {
    _good =false;
    _logger->error("gnutls_certificate_allocate_credentials() failed."
		   " Cause: %s", gnutls_strerror(r));
  }
}

TLSContext::~TLSContext()
{
  if(_certCred) {
    gnutls_certificate_free_credentials(_certCred);
  }
}

bool TLSContext::good() const
{
  return _good;
}

bool TLSContext::bad() const
{
  return !_good;
}

void TLSContext::addClientKeyFile(const std::string& certfile,
				  const std::string& keyfile)
  throw(DlAbortEx)
{
  int ret = gnutls_certificate_set_x509_key_file(_certCred,
						 certfile.c_str(),
						 keyfile.c_str(),
						 GNUTLS_X509_FMT_PEM);
  if(ret != GNUTLS_E_SUCCESS) {
    throw DlAbortEx
      (StringFormat("Failed to load client certificate from %s and"
		    " private key from %s. Cause: %s",
		    certfile.c_str(), keyfile.c_str(),
		    gnutls_strerror(ret)).str());
  }
}

void TLSContext::addTrustedCACertFile(const std::string& certfile)
  throw(DlAbortEx)
{
  int ret = gnutls_certificate_set_x509_trust_file(_certCred,
						   certfile.c_str(),
						   GNUTLS_X509_FMT_PEM);
  if(ret < 0) {
    throw DlAbortEx
      (StringFormat
       (MSG_LOADING_TRUSTED_CA_CERT_FAILED,
	certfile.c_str(), gnutls_strerror(ret)).str());
  }
  _logger->info("%d certificate(s) were imported.", ret);
}

gnutls_certificate_credentials_t TLSContext::getCertCred() const
{
  return _certCred;
}

void TLSContext::enablePeerVerification()
{
  _peerVerificationEnabled = true;
}

void TLSContext::disablePeerVerification()
{
  _peerVerificationEnabled = false;
}

bool TLSContext::peerVerificationEnabled() const
{
  return _peerVerificationEnabled;
}

} // namespace aria2
