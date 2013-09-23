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

#include <sstream>

#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>

#include "LogFactory.h"
#include "Logger.h"
#include "fmt.h"
#include "message.h"
#include "BufferedFile.h"

namespace {
  struct bio_deleter {
    void operator()(BIO *b) {
      if (b) BIO_free(b);
    }
  };
  struct p12_deleter {
    void operator()(PKCS12 *p) {
      if (p) PKCS12_free(p);
    }
  };
} // namespace

namespace aria2 {

TLSContext* TLSContext::make(TLSSessionSide side)
{
  return new OpenSSLTLSContext(side);
}

OpenSSLTLSContext::OpenSSLTLSContext(TLSSessionSide side)
  : sslCtx_(0),
    side_(side),
    verifyPeer_(true)
{
  sslCtx_ = SSL_CTX_new(SSLv23_method());
  if(sslCtx_) {
    good_ = true;
  } else {
    good_ = false;
    A2_LOG_ERROR(fmt("SSL_CTX_new() failed. Cause: %s",
                     ERR_error_string(ERR_get_error(), 0)));
  }
  // Disable SSLv2 and enable all workarounds for buggy servers
  SSL_CTX_set_options(sslCtx_, SSL_OP_ALL | SSL_OP_NO_SSLv2
#ifdef SSL_OP_NO_COMPRESSION
                      | SSL_OP_NO_COMPRESSION
#endif // SSL_OP_NO_COMPRESSION
                      );
  SSL_CTX_set_mode(sslCtx_, SSL_MODE_AUTO_RETRY);
  SSL_CTX_set_mode(sslCtx_, SSL_MODE_ENABLE_PARTIAL_WRITE);
  #ifdef SSL_MODE_RELEASE_BUFFERS
  /* keep memory usage low */
  SSL_CTX_set_mode(sslCtx_, SSL_MODE_RELEASE_BUFFERS);
  #endif
}

OpenSSLTLSContext::~OpenSSLTLSContext()
{
  SSL_CTX_free(sslCtx_);
}

bool OpenSSLTLSContext::good() const
{
  return good_;
}

bool OpenSSLTLSContext::addCredentialFile(const std::string& certfile,
                                   const std::string& keyfile)
{
  if (keyfile.empty()) {
    return addP12CredentialFile(certfile);
  }
  if(SSL_CTX_use_PrivateKey_file(sslCtx_, keyfile.c_str(),
                                 SSL_FILETYPE_PEM) != 1) {
    A2_LOG_ERROR(fmt("Failed to load private key from %s. Cause: %s",
                     keyfile.c_str(),
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  }
  if(SSL_CTX_use_certificate_chain_file(sslCtx_, certfile.c_str()) != 1) {
    A2_LOG_ERROR(fmt("Failed to load certificate from %s. Cause: %s",
                     certfile.c_str(),
                     ERR_error_string(ERR_get_error(), 0)));
    return false;
  }
  A2_LOG_INFO(fmt("Credential files(cert=%s, key=%s) were successfully added.",
                  certfile.c_str(),
                  keyfile.c_str()));
  return true;
}
bool OpenSSLTLSContext::addP12CredentialFile(const std::string& p12file)
{
  // Need this to "decrypt" p12 files.
  OpenSSL_add_all_algorithms();

  std::stringstream ss;
  BufferedFile(p12file.c_str(), "rb").transfer(ss);

  auto data = ss.str();
  void *ptr = const_cast<char*>(data.c_str());
  size_t len = data.length();
  std::unique_ptr<BIO, bio_deleter> bio(BIO_new_mem_buf(ptr, len));
  A2_LOG_DEBUG(fmt("p12 size: %" PRIu64, len));

  if (!bio) {
    A2_LOG_ERROR("Failed to open p12 file: no memory");
    return false;
  }
  std::unique_ptr<PKCS12, p12_deleter> p12(d2i_PKCS12_bio(bio.get(), nullptr));
  if (!p12) {
    A2_LOG_ERROR(fmt("Failed to open p12 file: %s",
                     ERR_error_string(ERR_get_error(), nullptr)));
    return false;
  }
  EVP_PKEY *pkey;
  X509 *cert;
  STACK_OF(X509) *ca = 0;
  if (!PKCS12_parse(p12.get(), "", &pkey, &cert, &ca)) {
    A2_LOG_ERROR(fmt("Failed to parse p12 file: %s",
                     ERR_error_string(ERR_get_error(), nullptr)));
    return false;
  }

  bool rv = false;
  if (pkey && cert) {
    rv = SSL_CTX_use_PrivateKey(sslCtx_, pkey);
    if (!rv) {
      A2_LOG_ERROR(fmt("Failed to use p12 file pkey: %s",
                       ERR_error_string(ERR_get_error(), nullptr)));
    }
    if (rv) {
      rv = SSL_CTX_use_certificate(sslCtx_, cert);
      if (!rv) {
        A2_LOG_ERROR(fmt("Failed to use p12 file cert: %s",
                         ERR_error_string(ERR_get_error(), nullptr)));
      }
    }
    if (rv && ca && sk_X509_num(ca)) {
      rv = SSL_CTX_add_extra_chain_cert(sslCtx_, ca);
      if (!rv) {
        A2_LOG_ERROR(fmt("Failed to use p12 file chain: %s",
                         ERR_error_string(ERR_get_error(), nullptr)));
      }
    }
  }
  else {
    A2_LOG_ERROR(fmt("Failed to use p12 file: no pkey or cert %s",
                     ERR_error_string(ERR_get_error(), nullptr)));
  }
  if (pkey) EVP_PKEY_free(pkey);
  if (cert) X509_free(cert);
  if (ca) sk_X509_pop_free(ca, X509_free);

  if (!rv) {
    A2_LOG_ERROR(fmt("Failed to use p12 file: %s",
                     ERR_error_string(ERR_get_error(), nullptr)));
  }
  else {
    A2_LOG_INFO("Using certificate and key from p12 file");
  }
  return rv;
}

bool OpenSSLTLSContext::addSystemTrustedCACerts()
{
  if(SSL_CTX_set_default_verify_paths(sslCtx_) != 1) {
    A2_LOG_INFO(fmt(MSG_LOADING_SYSTEM_TRUSTED_CA_CERTS_FAILED,
                    ERR_error_string(ERR_get_error(), 0)));
    return false;
  } else {
    A2_LOG_INFO("System trusted CA certificates were successfully added.");
    return true;
  }
}

bool OpenSSLTLSContext::addTrustedCACertFile(const std::string& certfile)
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

} // namespace aria2
