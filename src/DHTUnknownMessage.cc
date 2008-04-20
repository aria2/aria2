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
#include "DHTUnknownMessage.h"
#include "DHTNode.h"
#include "Util.h"
#include <cstring>

namespace aria2 {

DHTUnknownMessage::DHTUnknownMessage(const SharedHandle<DHTNode>& localNode,
				     const unsigned char* data, size_t length,
				     const std::string& ipaddr, uint16_t port):
  DHTMessage(localNode, SharedHandle<DHTNode>()),
  _length(length),
  _ipaddr(ipaddr),
  _port(port)
{
  if(_length == 0) {
    _data = 0;
  } else {
    _data = new unsigned char[length];
    memcpy(_data, data, length);
  }
}

DHTUnknownMessage::~DHTUnknownMessage()
{
  delete [] _data;
}

void DHTUnknownMessage::doReceivedAction() {}

void DHTUnknownMessage::send() {}

bool DHTUnknownMessage::isReply() const
{
  return false;
}

void DHTUnknownMessage::validate() const {}
  
std::string DHTUnknownMessage::getMessageType() const
{
  return "unknown";
}

std::string DHTUnknownMessage::toString() const
{
  size_t sampleLength = 8;
  if(_length < sampleLength) {
    sampleLength = _length;
  }
  std::string sample(&_data[0], &_data[sampleLength]);

  return "dht unknown Remote:"+_ipaddr+":"+Util::uitos(_port)+" length="+
    Util::uitos(_length)+", first 8 bytes(hex)="+Util::toHex(sample);
}

} // namespace aria2
