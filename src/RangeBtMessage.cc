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
#include "RangeBtMessage.h"
#include "util.h"
#include "a2functional.h"
#include "bittorrent_helper.h"

namespace aria2 {

RangeBtMessage::RangeBtMessage(uint8_t id, const char* name, size_t index,
                               int32_t begin, int32_t length)
    : SimpleBtMessage(id, name), index_(index), begin_(begin), length_(length)
{
}

std::vector<unsigned char> RangeBtMessage::createMessage()
{
  /**
   * len --- 13, 4bytes
   * id --- ?, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * length -- length, 4bytes
   * total: 17bytes
   */
  auto msg = std::vector<unsigned char>(MESSAGE_LENGTH);
  bittorrent::createPeerMessageString(msg.data(), MESSAGE_LENGTH, 13, getId());
  bittorrent::setIntParam(&msg[5], index_);
  bittorrent::setIntParam(&msg[9], begin_);
  bittorrent::setIntParam(&msg[13], length_);
  return msg;
}

std::string RangeBtMessage::toString() const
{
  return fmt("%s index=%lu, begin=%d, length=%d", getName(),
             static_cast<unsigned long>(index_), begin_, length_);
}

} // namespace aria2
