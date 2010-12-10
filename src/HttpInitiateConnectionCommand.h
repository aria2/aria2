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
#ifndef D_HTTP_INITIATE_CONNECTION_COMMAND_H
#define D_HTTP_INITIATE_CONNECTION_COMMAND_H

#include "InitiateConnectionCommand.h"

namespace aria2 {

// HttpInitiateConnectionCommand determines remote host to connect and
// resolves IP address from that hostname and creates subsequent
// command.  Usually, remote host is the host in URI. If proxy is
// used, remote host becomes proxy server. This command searches
// pooled socket using resolved IP addresses and use pooled socket if
// available.  The following chart shows what Command is followed
// after this command based on conditions.
//
// HttpInitiateConnectionCommand
//                 |
//                 |  proxy is used?
//                 +----------------+
//                 |                |
//                 |                |  pooled socket?
//                 |                +------------> HttpRequestCommand
//                 |                |  tunnel?
//                 |                +------------> HttpProxyRequestCommand
//                 |                |  otherwise
//                 |                +------------> HttpRequestCommand
//                 | direct connection
//                 +-----------------------------> HttpRequestCommand
//
// HttpInitiateConnectionCommand::execute() returns true when DNS is
// in synchronous mode and address resolution was complete.  When DNS
// is in asynchronous mode, it may return false: This means address
// resolution is in progress. After address resolution completed,
// calling execute() returns true.
class HttpInitiateConnectionCommand : public InitiateConnectionCommand {
protected:
  virtual Command* createNextCommand
  (const std::string& hostname, const std::string& addr, uint16_t port,
   const std::vector<std::string>& resolvedAddresses,
   const SharedHandle<Request>& proxyRequest);
public:
  HttpInitiateConnectionCommand(cuid_t cuid, const SharedHandle<Request>& req,
                                const SharedHandle<FileEntry>& fileEntry,
                                RequestGroup* requestGroup,
                                DownloadEngine* e);

  virtual ~HttpInitiateConnectionCommand();
};

} // namespace aria2

#endif // D_HTTP_INITIATE_CONNECTION_COMMAND_H


