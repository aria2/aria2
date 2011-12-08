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
#ifndef D_DHT_CONSTANTS_H
#define D_DHT_CONSTANTS_H

// Increment this if major improvements or bug fixes are made in DHT
// code. This is 2 bytes unsigned integer.
#define DHT_VERSION 3U

#define DHT_ID_LENGTH 20

#define DHT_TRANSACTION_ID_LENGTH 2

#define DHT_TOKEN_LENGTH 4

// See --dht-message-timeout option.
#define DHT_MESSAGE_TIMEOUT 10

#define DHT_NODE_CONTACT_INTERVAL (15*60)

#define DHT_BUCKET_REFRESH_INTERVAL (15*60)

#define DHT_BUCKET_REFRESH_CHECK_INTERVAL (5*60)

#define DHT_PEER_ANNOUNCE_PURGE_INTERVAL (30*60)

#define DHT_PEER_ANNOUNCE_INTERVAL (15*60)

#define DHT_PEER_ANNOUNCE_CHECK_INTERVAL (5*60)

#define DHT_TOKEN_UPDATE_INTERVAL (10*60)

#endif // D_DHT_CONSTANTS_H
