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
#ifndef LIBGNUTLS_TLS_SESSION_H
#define LIBGNUTLS_TLS_SESSION_H

#include "common.h"

#include <gnutls/gnutls.h>

#include "LibgnutlsTLSContext.h"
#include "TLSSession.h"
#include "a2netcompat.h"

namespace aria2 {

class GnuTLSSession : public TLSSession {
public:
  GnuTLSSession(GnuTLSContext* tlsContext);
  ~GnuTLSSession();
  virtual int init(sock_t sockfd) CXX11_OVERRIDE;
  virtual int setSNIHostname(const std::string& hostname) CXX11_OVERRIDE;
  virtual int closeConnection() CXX11_OVERRIDE;
  virtual int checkDirection() CXX11_OVERRIDE;
  virtual ssize_t writeData(const void* data, size_t len) CXX11_OVERRIDE;
  virtual ssize_t readData(void* data, size_t len) CXX11_OVERRIDE;
  virtual int tlsConnect(const std::string& hostname, TLSVersion& version,
                         std::string& handshakeErr) CXX11_OVERRIDE;
  virtual int tlsAccept(TLSVersion& version) CXX11_OVERRIDE;
  virtual std::string getLastErrorString() CXX11_OVERRIDE;
  virtual size_t getRecvBufferedLength() CXX11_OVERRIDE { return 0; }

private:
  gnutls_session_t sslSession_;
  GnuTLSContext* tlsContext_;
  // Last error code from gnutls library functions
  int rv_;
};

} // namespace aria2

#endif // LIBGNUTLS_TLS_SESSION_H
