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
#include "DHTNode.h"

#include <cstring>

#include "util.h"
#include "a2functional.h"
#include "bittorrent_helper.h"
#include "wallclock.h"

namespace aria2 {

DHTNode::DHTNode():port_(0), rtt_(0), condition_(0), lastContact_(0)
{
  generateID();
}

DHTNode::DHTNode(const unsigned char* id):port_(0), rtt_(0), condition_(1), lastContact_(0)
{
  memcpy(id_, id, DHT_ID_LENGTH);
}

DHTNode::~DHTNode() {}

void DHTNode::generateID()
{
  util::generateRandomKey(id_);
}

bool DHTNode::operator==(const DHTNode& node) const
{
  return memcmp(id_, node.id_, DHT_ID_LENGTH) == 0;
}

bool DHTNode::operator<(const DHTNode& node) const
{
  for(size_t i = 0; i < DHT_ID_LENGTH; ++i) {
    if(id_[i] > node.id_[i]) {
      return false;
    } else if(id_[i] < node.id_[i]) {
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
  return condition_ >= BAD_CONDITION;
}

bool DHTNode::isQuestionable() const
{
  return !isBad() &&
    lastContact_.difference(global::wallclock()) >= DHT_NODE_CONTACT_INTERVAL;
}

void DHTNode::markGood()
{
  condition_ = 0;
}

void DHTNode::markBad()
{
  condition_ = BAD_CONDITION;
}

void DHTNode::updateLastContact()
{
  lastContact_ = global::wallclock();
}

void DHTNode::timeout()
{
  ++condition_;
}

std::string DHTNode::toString() const
{
  return fmt("DHTNode ID=%s, Host=%s(%u), Condition=%d, RTT=%d",
             util::toHex(id_, DHT_ID_LENGTH).c_str(),
             ipaddr_.c_str(),
             port_,
             condition_,
             rtt_);
}

void DHTNode::setID(const unsigned char* id)
{
  memcpy(id_, id, DHT_ID_LENGTH);
}

void DHTNode::setIPAddress(const std::string& ipaddr)
{
  ipaddr_ = ipaddr;
}

} // namespace aria2
