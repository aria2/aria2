/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "UDPTrackerRequest.h"

namespace aria2 {

UDPTrackerReply::UDPTrackerReply()
    : action(0), transactionId(0), interval(0), leechers(0), seeders(0)
{
}

UDPTrackerRequest::UDPTrackerRequest()
    : remotePort(0),
      connectionId(0),
      action(UDPT_ACT_CONNECT),
      transactionId(0),
      downloaded(0),
      left(0),
      uploaded(0),
      event(UDPT_EVT_NONE),
      ip(0),
      key(0),
      numWant(0),
      port(0),
      extensions(0),
      state(UDPT_STA_PENDING),
      error(UDPT_ERR_SUCCESS),
      dispatched(Timer::zero()),
      failCount(0),
      user_data(nullptr)
{
}

} // namespace aria2
