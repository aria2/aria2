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
#ifndef _D_PEER_INTERACTION_COMMAND_H_
#define _D_PEER_INTERACTION_COMMAND_H_

#include "PeerAbstractCommand.h"
#include "PeerConnection.h"
#include "SendMessageQueue.h"

using namespace std;

#define MAX_PEER_CHOKING_INTERVAL (3*60)

class PeerInteractionCommand : public PeerAbstractCommand {
private:
  int sequence;
  SendMessageQueue* sendMessageQueue;
 
  struct timeval keepAliveCheckPoint;
  struct timeval chokeCheckPoint;
  struct timeval freqCheckPoint;
  int chokeUnchokeCount;
  int haveCount;
  int keepAliveCount;
  void receiveMessage();
  void detectMessageFlooding();
  void checkLongTimePeerChoking();
  void checkInactiveConnection();
  void detectTimeoutAndDuplicateBlock();
  void decideChoking();
  void keepAlive();
protected:
  bool executeInternal();
  bool prepareForRetry(int wait);
  bool prepareForNextPeer(int wait);
  void onAbort(Exception* ex);
  void beforeSocketCheck();
public:
  PeerInteractionCommand(int cuid, Peer* peer, TorrentDownloadEngine* e, const Socket* s, int sequence);
  ~PeerInteractionCommand();

  enum Seq {
    INITIATOR_SEND_HANDSHAKE,
    INITIATOR_WAIT_HANDSHAKE,
    RECEIVER_SEND_HANDSHAKE,
    RECEIVER_WAIT_HANDSHAKE,
    WIRED};
};

#endif // _D_PEER_INTERACTION_COMMAND_H_
