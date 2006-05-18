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
#ifndef _D_PORT_MESSAGE_H_
#define _D_PORT_MESSAGE_H_

#include "PeerMessage.h"

class PortMessage : public PeerMessage {
private:
  int port;
public:
  PortMessage():PeerMessage() {}
  virtual ~PortMessage() {}

  enum ID {
    ID = 9
  };

  int getPort() const { return port; }
  void setPort(int port) { this->port = port; }

  virtual int getId() const { return ID; }

  static PortMessage* create(const char* data, int dataLength);

  virtual void receivedAction() {
    logger->info("DHT is not supported yet.");
  }
  virtual void send() {}
  virtual string toString() const {
    return "port";
  }
};

#endif // _D_PORT_MESSAGE_H_
