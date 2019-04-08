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
#ifndef D_TLS_CONTEXT_H
#define D_TLS_CONTEXT_H

#include <string>

#include "common.h"

namespace aria2 {

enum TLSSessionSide { TLS_CLIENT, TLS_SERVER };

enum TLSVersion {
  TLS_PROTO_NONE,
  TLS_PROTO_TLS11,
  TLS_PROTO_TLS12,
  TLS_PROTO_TLS13,
};

class TLSContext {
public:
  static TLSContext* make(TLSSessionSide side, TLSVersion minVer);
  virtual ~TLSContext() = default;

  // private key `keyfile' must be decrypted.
  virtual bool addCredentialFile(const std::string& certfile,
                                 const std::string& keyfile) = 0;

  virtual bool addSystemTrustedCACerts() = 0;

  // certfile can contain multiple certificates.
  virtual bool addTrustedCACertFile(const std::string& certfile) = 0;

  virtual bool good() const = 0;

  virtual TLSSessionSide getSide() const = 0;
  virtual bool getVerifyPeer() const = 0;
  virtual void setVerifyPeer(bool) = 0;
};

} // namespace aria2

#endif // D_TLS_CONTEXT_H
