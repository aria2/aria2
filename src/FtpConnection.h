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
#ifndef D_FTP_CONNECTION_H
#define D_FTP_CONNECTION_H

#include "common.h"

#include <utility>
#include <string>
#include <memory>

#include "TimeA2.h"
#include "SocketBuffer.h"
#include "Command.h"
#include "a2functional.h"

namespace aria2 {

class Option;
class Segment;
class Request;
class SocketCore;
class AuthConfig;

class FtpConnection {
private:
  cuid_t cuid_;
  std::shared_ptr<SocketCore> socket_;
  std::shared_ptr<Request> req_;

  std::shared_ptr<AuthConfig> authConfig_;

  const Option* option_;

  std::string strbuf_;

  SocketBuffer socketBuffer_;

  std::string baseWorkingDir_;

  int getStatus(const std::string& response) const;
  std::string::size_type findEndOfResponse(int status,
                                           const std::string& buf) const;
  bool bulkReceiveResponse(std::pair<int, std::string>& response);

  // prepare for large banners
  static const size_t MAX_RECV_BUFFER = 64_k;

public:
  FtpConnection(cuid_t cuid, const std::shared_ptr<SocketCore>& socket,
                const std::shared_ptr<Request>& req,
                const std::shared_ptr<AuthConfig>& authConfig,
                const Option* op);
  ~FtpConnection();
  bool sendUser();
  bool sendPass();
  bool sendType();
  bool sendPwd();
  bool sendCwd(const std::string& dir);
  bool sendMdtm();
  bool sendSize();
  bool sendEpsv();
  bool sendPasv();
  std::shared_ptr<SocketCore> createServerSocket();
  bool sendEprt(const std::shared_ptr<SocketCore>& serverSocket);
  bool sendPort(const std::shared_ptr<SocketCore>& serverSocket);
  bool sendRest(const std::shared_ptr<Segment>& segment);
  bool sendRetr();

  int receiveResponse();
  int receiveSizeResponse(int64_t& size);
  // Returns status code of MDTM reply. If the status code is 213, parses
  // time-val and store it in time.
  // If a code other than 213 is returned, time is not touched.
  // Expect MDTM reply is YYYYMMDDhhmmss in GMT. If status is 213 but returned
  // date cannot be parsed, then assign Time::null() to given time.
  // If reply is not received yet, returns 0.
  int receiveMdtmResponse(Time& time);
  int receiveEpsvResponse(uint16_t& port);
  int receivePasvResponse(std::pair<std::string, uint16_t>& dest);
  int receivePwdResponse(std::string& pwd);

  void setBaseWorkingDir(const std::string& baseWorkingDir);

  const std::string& getBaseWorkingDir() const { return baseWorkingDir_; }

  const std::string& getUser() const;
};

} // namespace aria2

#endif // D_FTP_CONNECTION_H
