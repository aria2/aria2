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
#include "FtpInitiateConnectionCommand.h"
#include "FtpNegotiationCommand.h"
#include "HttpRequestCommand.h"
#include "FtpTunnelRequestCommand.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "Util.h"

FtpInitiateConnectionCommand::FtpInitiateConnectionCommand(int cuid,
							   const RequestHandle& req,
							   DownloadEngine* e)
  :AbstractCommand(cuid, req, e)
{
  setTimeout(e->option->getAsInt(PREF_DNS_TIMEOUT));
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

FtpInitiateConnectionCommand::~FtpInitiateConnectionCommand() {
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(nameResolver);
#endif // ENABLE_ASYNC_DNS
}

bool FtpInitiateConnectionCommand::executeInternal() {
  string hostname;
  if(useHttpProxy()) {
    hostname = e->option->get(PREF_HTTP_PROXY_HOST);
  } else {
    hostname = req->getHost();
  }
  if(!Util::isNumbersAndDotsNotation(hostname)) {
    if(resolveHostname(hostname, nameResolver)) {
      hostname = nameResolver->getAddrString();
    } else {
      e->commands.push_back(this);
      return false;
    }
  }
  Command* command;
  if(useHttpProxy()) {
    logger->info(MSG_CONNECTING_TO_SERVER, cuid,
		 e->option->get(PREF_HTTP_PROXY_HOST).c_str(),
		 e->option->getAsInt(PREF_HTTP_PROXY_PORT));
    socket->establishConnection(hostname,
				e->option->getAsInt(PREF_HTTP_PROXY_PORT));
    
    if(useHttpProxyGet()) {
      command = new HttpRequestCommand(cuid, req, e, socket);
    } else if(useHttpProxyConnect()) {
      command = new FtpTunnelRequestCommand(cuid, req, e, socket);
    } else {
      // TODO
      throw new DlAbortEx("ERROR");
    }
  } else {
    logger->info(MSG_CONNECTING_TO_SERVER, cuid, req->getHost().c_str(),
		 req->getPort());
    socket->establishConnection(hostname, req->getPort());
    command = new FtpNegotiationCommand(cuid, req, e, socket);
  }
  e->commands.push_back(command);
  return true;
}

bool FtpInitiateConnectionCommand::useHttpProxy() const {
  return e->option->get(PREF_HTTP_PROXY_ENABLED) == V_TRUE;
}

bool FtpInitiateConnectionCommand::useHttpProxyGet() const {
  return useHttpProxy() && e->option->get(PREF_FTP_VIA_HTTP_PROXY) == V_GET;
}

bool FtpInitiateConnectionCommand::useHttpProxyConnect() const {
  return useHttpProxy() && e->option->get(PREF_FTP_VIA_HTTP_PROXY) == V_TUNNEL;
}
