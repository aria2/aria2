/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "RangeBtMessage.h"
#include "util.h"
#include "a2functional.h"
#include "bittorrent_helper.h"

namespace aria2 {

RangeBtMessage::RangeBtMessage(uint8_t id,
			       const std::string& name,
			       size_t index, uint32_t begin, size_t length)
  :SimpleBtMessage(id, name),
   _index(index),
   _begin(begin),
   _length(length),
   _msg(0) {}

RangeBtMessage::~RangeBtMessage()
{
  delete [] _msg;
}

const unsigned char* RangeBtMessage::getMessage()
{
  if(!_msg) {
    /**
     * len --- 13, 4bytes
     * id --- ?, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * length -- length, 4bytes
     * total: 17bytes
     */
    _msg = new unsigned char[MESSAGE_LENGTH];
    bittorrent::createPeerMessageString(_msg, MESSAGE_LENGTH, 13, getId());
    bittorrent::setIntParam(&_msg[5], _index);
    bittorrent::setIntParam(&_msg[9], _begin);
    bittorrent::setIntParam(&_msg[13], _length);
  }
  return _msg;
}

size_t RangeBtMessage::getMessageLength()
{
  return MESSAGE_LENGTH;
}

std::string RangeBtMessage::toString() const
{
  return strconcat(getName(), " index=", util::uitos(_index),
		   ", begin=", util::uitos(_begin),
		   ", length=", util::uitos(_length));
}

} // namespace aria2
