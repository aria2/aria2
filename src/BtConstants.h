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
#ifndef D_BT_CONSTANTS_H
#define D_BT_CONSTANTS_H

#include "common.h"
#include <map>
#include <string>

typedef std::map<std::string, uint8_t> Extensions;

#define INFO_HASH_LENGTH 20

#define PIECE_HASH_LENGTH 20

#define PEER_ID_LENGTH 20

#define INFO_HASH_LENGTH 20

#define MAX_BLOCK_LENGTH (16*1024)

#define DEFAULT_MAX_OUTSTANDING_REQUEST 6

#define OUTSTANDING_REQUEST_STEP 6

// Upper Bound of the number of outstanding request
#define UB_MAX_OUTSTANDING_REQUEST 256

#define METADATA_PIECE_SIZE (16*1024)

#define LPD_MULTICAST_ADDR "239.192.152.143"

#define LPD_MULTICAST_PORT 6771

#define COMPACT_LEN_IPV4 6

#define COMPACT_LEN_IPV6 18

#endif // D_BT_CONSTANTS_H
