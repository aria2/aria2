/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_FTP_CONNECTION_H_
#define _D_FTP_CONNECTION_H_

#include "Socket.h"
#include "Option.h"
#include "Logger.h"
#include "Segment.h"
#include "Request.h"
#include "common.h"
#include <utility>

using namespace std;

class FtpConnection {
private:
  int cuid;
  const Socket* socket;
  const Request* req;
  const Option* option;
  const Logger* logger;

  string strbuf;

  int getStatus(const string& response) const;
  bool isEndOfResponse(int status, const string& response) const;
  bool bulkReceiveResponse(pair<int, string>& response);
public:
  FtpConnection(int cuid, const Socket* socket, const Request* req, const Option* op);
  ~FtpConnection();
  void sendUser() const;
  void sendPass() const;
  void sendType() const;
  void sendCwd() const;
  void sendSize() const;
  void sendPasv() const;
  Socket* sendPort() const;
  void sendRest(const Segment& segment) const;
  void sendRetr() const;

  int receiveResponse();
  int receiveSizeResponse(long long int& size);
  int receivePasvResponse(pair<string, int>& dest);
};

#endif // _D_FTP_CONNECTION_H_
