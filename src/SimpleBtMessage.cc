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
#include "SimpleBtMessage.h"
#include "message.h"
#include "BtRegistry.h"

SimpleBtMessage::SimpleBtMessage():leftDataLength(0) {}

SimpleBtMessage::~SimpleBtMessage() {}

void SimpleBtMessage::send() {
  if(invalidate) {
    return;
  }
  if(sendPredicate() || sendingInProgress) {
    const unsigned char* msg = getMessage();
    int msgLength = getMessageLength();
    if(!sendingInProgress) {
      logger->info(MSG_SEND_PEER_MESSAGE,
		   cuid, peer->ipaddr.c_str(), peer->port, toString().c_str());
      leftDataLength = getMessageLength();
    }
    sendingInProgress = false;
    int writtenLength = PEER_CONNECTION(btContext, peer)->sendMessage(msg+msgLength-leftDataLength, leftDataLength);
    if(writtenLength == leftDataLength) {
      onSendComplete();
    } else {
      leftDataLength -= writtenLength;
      sendingInProgress = true;
    }
  }
}
