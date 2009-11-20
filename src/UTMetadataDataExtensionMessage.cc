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
#include "UTMetadataDataExtensionMessage.h"
#include "BDE.h"
#include "bencode.h"
#include "util.h"
#include "a2functional.h"

namespace aria2 {

UTMetadataDataExtensionMessage::UTMetadataDataExtensionMessage
(uint8_t extensionMessageID):UTMetadataExtensionMessage(extensionMessageID) {}

std::string UTMetadataDataExtensionMessage::getBencodedData()
{
  BDE list = BDE::list();

  BDE dict = BDE::dict();
  dict["msg_type"] = 1;
  dict["piece"] = _index;
  dict["total_size"] = _totalSize;

  BDE data = _data;

  list << dict;
  list << data;

  std::string encodedList = bencode::encode(list);
  // Remove first 'l' and last 'e' and return.
  return std::string(encodedList.begin()+1, encodedList.end()-1);
}

std::string UTMetadataDataExtensionMessage::toString() const
{
  return strconcat("ut_metadata data piece=", util::uitos(_index));
}

void UTMetadataDataExtensionMessage::doReceivedAction()
{
  // Update tracker

  // Write to pieceStorage
}

} // namespace aria2
