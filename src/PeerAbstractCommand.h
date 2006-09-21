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
#ifndef _D_PEER_ABSTRACT_COMMAND_H_
#define _D_PEER_ABSTRACT_COMMAND_H_

#include "Command.h"
#include "Request.h"
#include "TorrentDownloadEngine.h"
#include "TimeA2.h"

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
