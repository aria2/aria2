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
#include "DHTTokenTracker.h"
#include "DHTUtil.h"
#include "PeerMessageUtil.h"
#include "DlAbortEx.h"
#include "DHTConstants.h"
#include "MessageDigestHelper.h"
#include <cstring>

namespace aria2 {

DHTTokenTracker::DHTTokenTracker()
{
  DHTUtil::generateRandomData(_secret[0], SECRET_SIZE);
  memcpy(_secret[1], _secret[0], SECRET_SIZE);
}

DHTTokenTracker::DHTTokenTracker(const char* initialSecret)
{
  memcpy(_secret[0], initialSecret, SECRET_SIZE);
  memcpy(_secret[1], initialSecret, SECRET_SIZE);
}

DHTTokenTracker::~DHTTokenTracker() {}

std::string DHTTokenTracker::generateToken(const unsigned char* infoHash,
					   const std::string& ipaddr, uint16_t port,
					   const char* secret) const
{
  char src[DHT_ID_LENGTH+6+SECRET_SIZE];
  if(!PeerMessageUtil::createcompact(src+DHT_ID_LENGTH, ipaddr, port)) {
    throw new DlAbortEx("Token generation failed: ipaddr=%s, port=%u",
			ipaddr.c_str(), port);
  }
  memcpy(src, infoHash, DHT_ID_LENGTH);
  memcpy(src+DHT_ID_LENGTH+6, secret, SECRET_SIZE);
  unsigned char md[20];
  MessageDigestHelper::digest(md, sizeof(md), "sha1", src, sizeof(src));
  return std::string(&md[0], &md[sizeof(md)]);
}

std::string DHTTokenTracker::generateToken(const unsigned char* infoHash,
					   const std::string& ipaddr, uint16_t port) const
{
  return generateToken(infoHash, ipaddr, port, _secret[0]);
}

bool DHTTokenTracker::validateToken(const std::string& token,
				    const unsigned char* infoHash,
				    const std::string& ipaddr, uint16_t port) const
{
  for(int i = 0; i < 2; ++i) {
    if(generateToken(infoHash, ipaddr, port, _secret[i]) == token) {
      return true;
    }
  }
  return false;
}

void DHTTokenTracker::updateTokenSecret()
{
  memcpy(_secret[1], _secret[0], SECRET_SIZE);
  DHTUtil::generateRandomData(_secret[0], SECRET_SIZE);
}

} // namespace aria2
