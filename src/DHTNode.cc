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
#include "DHTNode.h"
#include "DHTUtil.h"
#include "Util.h"
#include <cstring>

namespace aria2 {

DHTNode::DHTNode():_port(0), _rtt(0), _condition(0), _lastContact(0)
{
  generateID();
}

DHTNode::DHTNode(const unsigned char* id):_port(0), _rtt(0), _condition(1), _lastContact(0)
{
  memcpy(_id, id, DHT_ID_LENGTH);
}

void DHTNode::generateID()
{
  DHTUtil::generateRandomKey(_id);
}

bool DHTNode::operator==(const DHTNode& node) const
{
  return memcmp(_id, node._id, DHT_ID_LENGTH) == 0;
}

bool DHTNode::operator<(const DHTNode& node) const
{
  for(size_t i = 0; i < DHT_ID_LENGTH; ++i) {
    if(_id[i] > node._id[i]) {
      return false;
    } else if(_id[i] < node._id[i]) {
      return true;
    }
  }
  return true;
}

bool DHTNode::isGood() const
{
  return !isBad() && !isQuestionable();
}

#define BAD_CONDITION 5

bool DHTNode::isBad() const
{
  return _condition >= BAD_CONDITION;
}

bool DHTNode::isQuestionable() const
{
  return !isBad() && _lastContact.elapsed(DHT_NODE_CONTACT_INTERVAL);
}

void DHTNode::markGood()
{
  _condition = 0;
}

void DHTNode::markBad()
{
  _condition = BAD_CONDITION;
}

void DHTNode::updateLastContact()
{
  _lastContact.reset();
}

void DHTNode::timeout()
{
  ++_condition;
}

std::string DHTNode::toString() const
{
  return "DHTNode ID="+Util::toHex(_id, DHT_ID_LENGTH)+
    ", Host="+_ipaddr+":"+Util::uitos(_port)+
    ", Condition="+Util::uitos(_condition)+
    ", RTT="+Util::itos(_rtt);
}

void DHTNode::setID(const unsigned char* id)
{
  memcpy(_id, id, DHT_ID_LENGTH);
}

} // namespace aria2
