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
#include "FtpInitiateConnectionCommand.h"
#include "FtpNegotiationCommand.h"
#include "HttpRequestCommand.h"
#include "FtpTunnelRequestCommand.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "Util.h"

FtpInitiateConnectionCommand::FtpInitiateConnectionCommand(int cuid,
							   Request* req,
							   DownloadEngine* e)
  :AbstractCommand(cuid, req, e)
{
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

FtpInitiateConnectionCommand::~FtpInitiateConnectionCommand() {
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(nameResolver);
#endif // ENABLE_ASYNC_DNS
}

bool FtpInitiateConnectionCommand::executeInternal(Segment& segment) {
  string hostname;
  if(useHttpProxy()) {
    hostname = e->option->get(PREF_HTTP_PROXY_HOST);
  } else {
    hostname = req->getHost();
  }
#ifdef ENABLE_ASYNC_DNS
  if(!Util::isNumbersAndDotsNotation(hostname)) {
    if(resolveHostname(hostname, nameResolver)) {
      hostname = nameResolver->getAddrString();
    } else {
      e->commands.push_back(this);
      return false;
    }
  }
#endif // ENABLE_ASYNC_DNS
  if(!e->segmentMan->downloadStarted) {
    e->segmentMan->filename = Util::urldecode(req->getFile());
    bool segFileExists = e->segmentMan->segmentFileExists();
    if(segFileExists) {
      e->segmentMan->load();
      e->segmentMan->diskWriter->openExistingFile(e->segmentMan->getFilePath());
      e->segmentMan->downloadStarted = true;
    } else {
      e->segmentMan->diskWriter->initAndOpenFile(e->segmentMan->getFilePath());
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
