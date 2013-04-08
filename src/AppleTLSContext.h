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
#ifndef D_APPLE_TLS_CONTEXT_H
#define D_APPLE_TLS_CONTEXT_H

#include "common.h"

#include <string>
#include <Security/Security.h>
#include <Security/SecureTransport.h>

#include "TLSContext.h"
#include "DlAbortEx.h"

namespace aria2 {

class AppleTLSContext : public TLSContext {
public:
  AppleTLSContext(TLSSessionSide side)
    : side_(side),
      verifyPeer_(true),
      credentials_(0)
  {}

  virtual ~AppleTLSContext();

  // private key `keyfile' must be decrypted.
  virtual bool addCredentialFile(const std::string& certfile,
                                 const std::string& keyfile);

  virtual bool addSystemTrustedCACerts() {
    return true;
  }

  // certfile can contain multiple certificates.
  virtual bool addTrustedCACertFile(const std::string& certfile);

  virtual bool good() const {
    return true;
  }
  virtual TLSSessionSide getSide() const {
    return side_;
  }

  virtual bool getVerifyPeer() const {
    return verifyPeer_;
  }
  virtual void setVerifyPeer(bool verify) {
    verifyPeer_ = verify;
  }

  SecIdentityRef getCredentials();

private:
  TLSSessionSide side_;
  bool verifyPeer_;
  SecIdentityRef credentials_;

  bool tryAsFingerprint(const std::string& fingerprint);
};

} // namespace aria2

#endif // D_LIBSSL_TLS_CONTEXT_H
