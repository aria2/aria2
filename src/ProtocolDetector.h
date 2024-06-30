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
#ifndef D_PROTOCOL_DETECTOR_H
#define D_PROTOCOL_DETECTOR_H

#include "common.h"
#include <string>

namespace aria2 {

class ProtocolDetector {
public:
  ProtocolDetector();

  ~ProtocolDetector();

  // Returns true if uri is http(s)/ftp, otherwise returns false.
  bool isStreamProtocol(const std::string& uri) const;

  // Returns true if ProtocolDetector thinks uri is a path of BitTorrent
  // metainfo file, otherwise returns false.
  bool guessTorrentFile(const std::string& uri) const;

  // Returns true if ProtocolDetector thinks uri is BitTorrent Magnet link.
  // magnet:?xt=urn:btih:<info-hash>...
  bool guessTorrentMagnet(const std::string& uri) const;

  // Returns true if ProtocolDetector thinks uri is a path of Metalink XML
  // file, otherwise return false.
  bool guessMetalinkFile(const std::string& uri) const;

  // Returns true if ProtocolDetector thinks uri is a path to aria2 control
  // file, otherwise return false
  bool guessAria2ControlFile(const std::string& uri) const;
};

} // namespace aria2

#endif // D_PROTOCOL_DETECTOR_H
