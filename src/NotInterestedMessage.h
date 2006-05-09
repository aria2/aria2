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
#ifndef _D_NOT_INTERESTED_MESSAGE_H_
#define _D_NOT_INTERESTED_MESSAGE_H_

#include "PeerMessage.h"

class NotInterestedMessage : public PeerMessage {
public:
  NotInterestedMessage():PeerMessage() {}
  virtual ~NotInterestedMessage() {}

  enum ID {
    ID = 3
  };

  virtual int getId() const { return ID; }
  virtual void receivedAction();
  virtual void send();
  virtual string toString() const;
};

#endif // _D_NOT_INTERESTED_MESSAGE_H_
