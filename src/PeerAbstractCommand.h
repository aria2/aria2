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
#ifndef _D_PEER_ABSTRACT_COMMAND_H_
#define _D_PEER_ABSTRACT_COMMAND_H_

#include "Command.h"
#include "Request.h"
#include "TorrentDownloadEngine.h"
#include "Time.h"

class PeerAbstractCommand : public Command {
private:
  Time checkPoint;
  int timeout;
protected:
  TorrentDownloadEngine* e;
  SocketHandle socket;
  PeerHandle peer;
  void setTimeout(int timeout) { this->timeout = timeout; }
  virtual bool prepareForNextPeer(int wait);
  virtual bool prepareForRetry(int wait);
  virtual void onAbort(Exception* ex);
  virtual bool executeInternal() = 0;
  void setReadCheckSocket(const SocketHandle& socket);
  void setWriteCheckSocket(const SocketHandle& socket);
  void disableReadCheckSocket();
  void disableWriteCheckSocket();
  void setUploadLimit(int uploadLimit);
  void setUploadLimitCheck(bool check);
  void setNoCheck(bool check);
private:
  bool checkSocketIsReadable;
  bool checkSocketIsWritable;
  SocketHandle readCheckTarget;
  SocketHandle writeCheckTarget;
  bool uploadLimitCheck;
  int uploadLimit;
  bool noCheck;
public:
  PeerAbstractCommand(int cuid, const PeerHandle& peer,
		      TorrentDownloadEngine* e,
		      const SocketHandle& s = SocketHandle());
  virtual ~PeerAbstractCommand();
  bool execute();
};

#endif // _D_PEER_ABSTRACT_COMMAND_H_
