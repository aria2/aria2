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
#ifndef _D_ABSTRACT_COMMAND_H_
#define _D_ABSTRACT_COMMAND_H_

#include "Command.h"
#include "Request.h"
#include "DownloadEngine.h"
#include "SegmentMan.h"
#include "TimeA2.h"

class AbstractCommand : public Command {
private:
  Time checkPoint;
  int timeout;
protected:
  Request* req;
  DownloadEngine* e;
  SocketHandle socket;

  void tryReserved();
  virtual bool prepareForRetry(int wait);
  virtual void onAbort(Exception* ex);
  virtual bool executeInternal(Segment segment) = 0;

  void setReadCheckSocket(const SocketHandle& socket);
  void setWriteCheckSocket(const SocketHandle& socket);
  void disableReadCheckSocket();
  void disableWriteCheckSocket();
  void setTimeout(int timeout) { this->timeout = timeout; }
private:
  bool checkSocketIsReadable;
  bool checkSocketIsWritable;
  SocketHandle readCheckTarget;
  SocketHandle writeCheckTarget;
public:
  AbstractCommand(int cuid, Request* req, DownloadEngine* e, const SocketHandle& s = SocketHandle());
  virtual ~AbstractCommand();
  bool execute();
};

#endif // _D_ABSTRACT_COMMAND_H_
