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
#ifndef _D_ACTIVE_PEER_CONNECTION_COMMAND_H_
#define _D_ACTIVE_PEER_CONNECTION_COMMAND_H_

#include "Command.h"
#include "BtContextAwareCommand.h"
#include "RequestGroupAware.h"
#include "TimeA2.h"

namespace aria2 {

class DownloadEngine;
class Peer;

class ActivePeerConnectionCommand : public Command,
				    public BtContextAwareCommand,
				    public RequestGroupAware
{
private:
  time_t interval; // UNIT: sec
  DownloadEngine* e;
  Time checkPoint;
  unsigned int _thresholdSpeed; // UNIT: byte/sec
  size_t _numNewConnection; // the number of the connection to establish.
public:
  ActivePeerConnectionCommand(int cuid,
			      RequestGroup* requestGroup,
			      DownloadEngine* e,
			      const SharedHandle<BtContext>& btContext,
			      time_t interval);
     
  virtual ~ActivePeerConnectionCommand();

  virtual bool execute();

  void connectToPeer(const SharedHandle<Peer>& peer);

  void setThresholdSpeed(unsigned int speed)
  {
    _thresholdSpeed = speed;
  }

  void setNumNewConnection(size_t numNewConnection)
  {
    _numNewConnection = numNewConnection;
  }
};

} // namespace aria2

#endif // _D_ACTIVE_PEER_CONNECTION_COMMAND_H_
