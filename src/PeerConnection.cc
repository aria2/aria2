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
#include "PeerConnection.h"
#include "message.h"
#include "DlAbortEx.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "LogFactory.h"
#include <netinet/in.h>

PeerConnection::PeerConnection(int cuid,
			       const SocketHandle& socket,
			       const Option* op)
  :cuid(cuid),
   socket(socket),
   option(op),
   logger(logger),
   resbufLength(0),
   currentPayloadLength(0),
   lenbufLength(0) {
  logger = LogFactory::getInstance();
}

PeerConnection::~PeerConnection() {}

int PeerConnection::sendMessage(const char* msg, int length) {
  int writtenLength = 0;
  if(socket->isWritable(0)) {
    socket->writeData(msg, length);
    writtenLength += length;
  }
  return writtenLength;
}

bool PeerConnection::receiveMessage(char* msg, int& length) {
  if(!socket->isReadable(0)) {
    return false;
  }
  if(resbufLength == 0 && lenbufLength != 4) {
    // read payload size, 4-byte integer
    int remain = 4-lenbufLength;
    int temp = remain;
    socket->readData(lenbuf+lenbufLength, temp);
    if(temp == 0) {
      // we got EOF
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    if(remain != temp) {
      // still 4-temp bytes to go
      lenbufLength += temp;
      return false;
    }
    //payloadLen = ntohl(nPayloadLen);
    int payloadLength = ntohl(*((int*)lenbuf));
    if(payloadLength > MAX_PAYLOAD_LEN || payloadLength < 0) {
      throw new DlAbortEx("max payload length exceeded or invalid. length = %d",
			  payloadLength);
    }
    currentPayloadLength = payloadLength;
  }
  // we have currentPayloadLen-resbufLen bytes to read
  int remaining = currentPayloadLength-resbufLength;
  if(remaining > 0) {
    socket->readData(resbuf+resbufLength, remaining);
    if(remaining == 0) {
      // we got EOF
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    resbufLength += remaining;
    if(currentPayloadLength != resbufLength) {
      return false;
    }
  }
  // we got whole payload.
  resbufLength = 0;
  lenbufLength = 0;

  memcpy(msg, resbuf, currentPayloadLength);
  length = currentPayloadLength;
  return true;
}

bool PeerConnection::receiveHandshake(char* msg, int& length) {
  if(!socket->isReadable(0)) {
    length = 0;
    return false;
  }
  int remain = HANDSHAKE_MESSAGE_LENGTH-resbufLength;
  int temp = remain;
  socket->readData(resbuf+resbufLength, temp);
  if(temp == 0) {
    // we got EOF
    throw new DlAbortEx(EX_EOF_FROM_PEER);
  }
  bool retval;
  if(remain != temp) {
    retval = false;
  } else {
    retval = true;
  }
  resbufLength += temp;
  // we got whole handshake payload
  int writeLength = resbufLength > length ? length : resbufLength;
  memcpy(msg, resbuf, writeLength);
  length = writeLength;
  if(retval) {
    resbufLength = 0;
  }
  return retval;
}
