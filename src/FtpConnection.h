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
#ifndef _D_FTP_CONNECTION_H_
#define _D_FTP_CONNECTION_H_

#include "common.h"
#include "Socket.h"
#include "Option.h"
#include "Logger.h"
#include "Segment.h"
#include "Request.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include <utility>

class FtpConnection {
private:
  int32_t cuid;
  SocketHandle socket;
  RequestHandle req;
  const Option* option;
  const Logger* logger;

  string strbuf;

  int32_t getStatus(const string& response) const;
  bool isEndOfResponse(int32_t status, const string& response) const;
  bool bulkReceiveResponse(pair<int32_t, string>& response) throw(DlRetryEx*);
public:
  FtpConnection(int32_t cuid, const SocketHandle& socket,
		const RequestHandle req, const Option* op);
  ~FtpConnection();
  void sendUser() const throw(DlRetryEx*);
  void sendPass() const throw(DlRetryEx*);
  void sendType() const throw(DlRetryEx*);
  void sendCwd() const throw(DlRetryEx*);
  void sendSize() const throw(DlRetryEx*);
  void sendPasv() const throw(DlRetryEx*);
  SocketHandle sendPort() const throw(DlAbortEx*, DlRetryEx*);
  void sendRest(const SegmentHandle& segment) const throw(DlRetryEx*);
  void sendRetr() const throw(DlRetryEx*);

  int32_t receiveResponse() throw(DlRetryEx*);
  int32_t receiveSizeResponse(int64_t& size) throw(DlRetryEx*);
  int32_t receivePasvResponse(pair<string, int32_t>& dest) throw(DlRetryEx*);
};

#endif // _D_FTP_CONNECTION_H_
