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
#include "PeerAbstractCommand.h"
#include "Peer.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "DlAbortEx.h"
#include "Socket.h"
#include "Logger.h"
#include "message.h"
#include "prefs.h"
#include "DownloadFailureException.h"
#include "StringFormat.h"
#include "wallclock.h"
#include "util.h"
#include "RequestGroupMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "ServerStatMan.h"
#include "FileEntry.h"

namespace aria2 {

PeerAbstractCommand::PeerAbstractCommand(cuid_t cuid,
                                         const SharedHandle<Peer>& peer,
                                         DownloadEngine* e,
                                         const SocketHandle& s):
  Command(cuid),
  _checkPoint(global::wallclock),
  // TODO referring global option
  _timeout(e->getOption()->getAsInt(PREF_BT_TIMEOUT)),
  _e(e),
  _socket(s),
  _peer(peer),
  _checkSocketIsReadable(false),
  _checkSocketIsWritable(false),
  _noCheck(false)
{
  if(!_socket.isNull() && _socket->isOpen()) {
    setReadCheckSocket(_socket);
  }
}

PeerAbstractCommand::~PeerAbstractCommand()
{
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

bool PeerAbstractCommand::execute()
{
  if(getLogger()->debug()) {
    getLogger()->debug("CUID#%s -"
                       " socket: read:%d, write:%d, hup:%d, err:%d, noCheck:%d",
                       util::itos(getCuid()).c_str(),
                       readEventEnabled(), writeEventEnabled(),
                       hupEventEnabled(), errorEventEnabled(),
                       _noCheck);
  }
  if(exitBeforeExecute()) {
    onAbort();
    return true;
  }
  try {
    if(_noCheck ||
       (_checkSocketIsReadable && readEventEnabled()) ||
       (_checkSocketIsWritable && writeEventEnabled()) ||
       hupEventEnabled()) {
      _checkPoint = global::wallclock;
    } else if(errorEventEnabled()) {
      throw DL_ABORT_EX
        (StringFormat(MSG_NETWORK_PROBLEM,
                      _socket->getSocketError().c_str()).str());
    }
    if(_checkPoint.difference(global::wallclock) >= _timeout) {
      throw DL_ABORT_EX(EX_TIME_OUT);
    }
    return executeInternal();
  } catch(DownloadFailureException& err) {
    getLogger()->error(EX_DOWNLOAD_ABORTED, err);
    onAbort();
    onFailure();
    return true;
  } catch(RecoverableException& err) {
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_TORRENT_DOWNLOAD_ABORTED, err,
                         util::itos(getCuid()).c_str());
      getLogger()->debug(MSG_PEER_BANNED,
                         util::itos(getCuid()).c_str(), _peer->ipaddr.c_str(),
                         _peer->port);
    }
    onAbort();
    return prepareForNextPeer(0);
  }
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerAbstractCommand::prepareForNextPeer(time_t wait)
{
  return true;
}

void PeerAbstractCommand::disableReadCheckSocket()
{
  if(_checkSocketIsReadable) {
    _e->deleteSocketForReadCheck(_readCheckTarget, this);
    _checkSocketIsReadable = false;
    _readCheckTarget.reset();
  }  
}

void PeerAbstractCommand::setReadCheckSocket(const SocketHandle& socket)
{
  if(!socket->isOpen()) {
    disableReadCheckSocket();
  } else {
    if(_checkSocketIsReadable) {
      if(_readCheckTarget != socket) {
        _e->deleteSocketForReadCheck(_readCheckTarget, this);
        _e->addSocketForReadCheck(socket, this);
        _readCheckTarget = socket;
      }
    } else {
      _e->addSocketForReadCheck(socket, this);
      _checkSocketIsReadable = true;
      _readCheckTarget = socket;
    }
  }
}

void PeerAbstractCommand::disableWriteCheckSocket()
{
  if(_checkSocketIsWritable) {
    _e->deleteSocketForWriteCheck(_writeCheckTarget, this);
    _checkSocketIsWritable = false;
    _writeCheckTarget.reset();
  }
}

void PeerAbstractCommand::setWriteCheckSocket(const SocketHandle& socket)
{
  if(!socket->isOpen()) {
    disableWriteCheckSocket();
  } else {
    if(_checkSocketIsWritable) {
      if(_writeCheckTarget != socket) {
        _e->deleteSocketForWriteCheck(_writeCheckTarget, this);
        _e->addSocketForWriteCheck(socket, this);
        _writeCheckTarget = socket;
      }
    } else {
      _e->addSocketForWriteCheck(socket, this);
      _checkSocketIsWritable = true;
      _writeCheckTarget = socket;
    }
  }
}

void PeerAbstractCommand::setNoCheck(bool check)
{
  _noCheck = check;
}

void PeerAbstractCommand::updateKeepAlive()
{
  _checkPoint = global::wallclock;
}

void PeerAbstractCommand::createSocket()
{
  _socket.reset(new SocketCore());
}

} // namespace aria2
