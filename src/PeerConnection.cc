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
#include "BtHandshakeMessage.h"
#include "a2netcompat.h"

PeerConnection::PeerConnection(int32_t cuid,
			       const SocketHandle& socket,
			       const Option* op)
  :cuid(cuid),
   socket(socket),
   option(op),
   logger(LogFactory::getInstance()),
   resbufLength(0),
   currentPayloadLength(0),
   lenbufLength(0)
{
  //logger->debug("PeerConnection::instantiated");
}

PeerConnection::~PeerConnection()
{
  //logger->debug("PeerConnection::deleted");
}

int32_t PeerConnection::sendMessage(const unsigned char* data, int32_t dataLength) {
  int32_t writtenLength = 0;
  if(socket->isWritable(0)) {
    // TODO fix this
    socket->writeData((const char*)data, dataLength);
    writtenLength += dataLength;
  }
  return writtenLength;
}

bool PeerConnection::receiveMessage(unsigned char* data, int32_t& dataLength) {
  if(resbufLength == 0 && 4 > lenbufLength) {
    if(!socket->isReadable(0)) {
      return false;
    }
    // read payload size, 32bit unsigned integer
    int32_t remaining = 4-lenbufLength;
    int32_t temp = remaining;
    socket->readData(lenbuf+lenbufLength, remaining);
    if(remaining == 0) {
      // we got EOF
      logger->debug("CUID#%d - In PeerConnection::receiveMessage(), remain=%d",
		    cuid, temp);
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    lenbufLength += remaining;
    if(4 > lenbufLength) {
      // still 4-lenbufLength bytes to go
      return false;
    }
    uint32_t payloadLength = ntohl(*(reinterpret_cast<uint32_t*>(lenbuf)));
    if(payloadLength > MAX_PAYLOAD_LEN) {
      throw new DlAbortEx(EX_TOO_LONG_PAYLOAD, payloadLength);
    }
    currentPayloadLength = payloadLength;
  }
  if(!socket->isReadable(0)) {
    return false;
  }
  // we have currentPayloadLen-resbufLen bytes to read
  int32_t remaining = currentPayloadLength-resbufLength;
  int32_t temp = remaining;
  if(remaining > 0) {
    socket->readData(resbuf+resbufLength, remaining);
    if(remaining == 0) {
      // we got EOF
      logger->debug("CUID#%d - In PeerConnection::receiveMessage(), payloadlen=%d, remaining=%d",
		    cuid, currentPayloadLength, temp);
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    resbufLength += remaining;
    if(currentPayloadLength > resbufLength) {
      return false;
    }
  }
  // we got whole payload.
  resbufLength = 0;
  lenbufLength = 0;

  memcpy(data, resbuf, currentPayloadLength);
  dataLength = currentPayloadLength;
  return true;
}

bool PeerConnection::receiveHandshake(unsigned char* data, int32_t& dataLength,
				      bool peek) {
  int32_t remaining = BtHandshakeMessage::MESSAGE_LENGTH-resbufLength;
  if(remaining > 0 && !socket->isReadable(0)) {
    dataLength = 0;
    return false;
  }
  bool retval = true;
  if(remaining > 0) {
    int32_t temp = remaining;
    socket->readData(resbuf+resbufLength, remaining);
    if(remaining == 0) {
      // we got EOF
      logger->debug("CUID#%d - In PeerConnection::receiveHandshake(), remain=%d",
		    cuid, temp);
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    resbufLength += remaining;
    if(BtHandshakeMessage::MESSAGE_LENGTH > resbufLength) {
      retval = false;
    }
  }
  int32_t writeLength = resbufLength > dataLength ? dataLength : resbufLength;
  memcpy(data, resbuf, writeLength);
  dataLength = writeLength;
  if(retval && !peek) {
    resbufLength = 0;
  }
  return retval;
}
