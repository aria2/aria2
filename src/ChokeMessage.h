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
#ifndef _D_CHOKE_MESSAGE_H_
#define _D_CHOKE_MESSAGE_H_

#include "SimplePeerMessage.h"

class ChokeMessage : public SimplePeerMessage {
private:
  char msg[5];
protected:
  virtual bool sendPredicate() const;
  virtual void onSendComplete();
public:
  ChokeMessage():SimplePeerMessage() {}
  virtual ~ChokeMessage() {}

  enum ID {
    ID = 0
  };

  virtual int getId() const { return ID; }
  virtual void receivedAction();
  virtual const char* getMessage();
  virtual int getMessageLength();
  virtual string toString() const;

  static ChokeMessage* create(const char* data, int dataLength);
};

#endif // _D_CHOKE_MESSAGE_H_
