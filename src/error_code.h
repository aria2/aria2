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
#ifndef D_ERROR_CODE_H
#define D_ERROR_CODE_H

#include "common.h"

namespace aria2 {

namespace error_code {

enum Value {
  UNDEFINED = -1,
  FINISHED = 0,
  UNKNOWN_ERROR = 1,
  TIME_OUT = 2,
  RESOURCE_NOT_FOUND = 3,
  MAX_FILE_NOT_FOUND = 4,
  TOO_SLOW_DOWNLOAD_SPEED = 5,
  NETWORK_PROBLEM = 6,
  IN_PROGRESS = 7,
  CANNOT_RESUME = 8,
  NOT_ENOUGH_DISK_SPACE = 9,
  PIECE_LENGTH_CHANGED = 10,
  DUPLICATE_DOWNLOAD = 11,
  DUPLICATE_INFO_HASH = 12,
  FILE_ALREADY_EXISTS = 13,
  FILE_RENAMING_FAILED = 14,
  FILE_OPEN_ERROR = 15,
  FILE_CREATE_ERROR = 16,
  FILE_IO_ERROR = 17,
  DIR_CREATE_ERROR = 18,
  NAME_RESOLVE_ERROR = 19,
  METALINK_PARSE_ERROR = 20,
  FTP_PROTOCOL_ERROR = 21,
  HTTP_PROTOCOL_ERROR = 22,
  HTTP_TOO_MANY_REDIRECTS = 23,
  HTTP_AUTH_FAILED = 24,
  BENCODE_PARSE_ERROR = 25,
  BITTORRENT_PARSE_ERROR = 26,
  MAGNET_PARSE_ERROR = 27,
  OPTION_ERROR = 28,
  HTTP_SERVICE_UNAVAILABLE = 29,
  JSON_PARSE_ERROR = 30,
  REMOVED = 31,
  CHECKSUM_ERROR = 32,
};

} // namespace error_code

} // namespace aria2

#endif // D_ERROR_CODE_H
