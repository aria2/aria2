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
#ifndef _D_HTTP_INITIATE_CONNECTION_COMMAND_H_
#define _D_HTTP_INITIATE_CONNECTION_COMMAND_H_

#include "AbstractCommand.h"

class HttpInitiateConnectionCommand : public AbstractCommand {
private:
  bool useProxy();
  bool useProxyGet();
  bool useProxyTunnel();
protected:
  /**
   * Connect to the server.
   * This method just send connection request to the server.
   * Using nonblocking mode of socket, this funtion returns immediately
   * after send connection packet to the server.
   * Whether or not the connection is established successfully is
   * evaluated by RequestCommand.
   */
  bool executeInternal(Segment segment);
public:
  HttpInitiateConnectionCommand(int cuid, Request* req, DownloadEngine* e);
  ~HttpInitiateConnectionCommand();
};

#endif // _D_HTTP_INITIATE_CONNECTION_COMMAND_H_


