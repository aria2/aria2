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
#ifndef _D_BITTORRENT_HELPER_H_
#define _D_BITTORRENT_HELPER_H_

#include "common.h"

#include <string>
#include <vector>
#include <deque>

#include "SharedHandle.h"
#include "AnnounceTier.h"
#include "BDE.h"
#include "Util.h"

namespace aria2 {

class DownloadContext;
class Randomizer;

namespace bittorrent {

extern const std::string INFO_HASH;

extern const std::string MODE;

extern const std::string PRIVATE;

extern const std::string ANNOUNCE_LIST;

extern const std::string NODES;

extern const std::string HOSTNAME;

extern const std::string PORT;

extern const std::string NAME;

extern const std::string URL_LIST;

extern const std::string SINGLE;

extern const std::string MULTI;

extern const std::string BITTORRENT;

void load(const std::string& torrentFile,
	  const SharedHandle<DownloadContext>& ctx,
	  const std::string& overrideName = "");

void load(const std::string& torrentFile,
	  const SharedHandle<DownloadContext>& ctx,
	  const std::deque<std::string>& uris,
	  const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::string& defaultName,
		    const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::deque<std::string>& uris,
		    const std::string& defaultName,
		    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::string& defaultName,
		    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
		    const SharedHandle<DownloadContext>& ctx,
		    const std::deque<std::string>& uris,
		    const std::string& defaultName,
		    const std::string& overrideName = "");

// Generates Peer ID. BitTorrent specification says Peer ID is 20-byte
// length.  This function uses peerIdPrefix as a Peer ID and it is
// less than 20bytes, random bytes are generated and appened to it. If
// peerIdPrefix is larger than 20bytes, first 20bytes are used.
std::string generatePeerId
(const std::string& peerIdPrefix, const SharedHandle<Randomizer>& randomizer);

// Generates Peer ID and stores it in static variable. This function
// uses generatePeerId(peerIdPrefix, randomizer) to produce Peer ID.
const std::string& generateStaticPeerId
(const std::string& peerIdPrefix, const SharedHandle<Randomizer>& randomizer);

// Returns Peer ID statically stored by generateStaticPeerId().  If
// Peer ID is not stored yet, this function calls
// generateStaticPeerId("-aria2-", randomizer)
const unsigned char* getStaticPeerId();

// Set newPeerId as a static Peer ID. newPeerId must be 20-byte
// length.
void setStaticPeerId(const std::string& newPeerId);

// Computes fast set index and stores them in fastset.
void computeFastSet
(std::vector<size_t>& fastSet, const std::string& ipaddr,
 size_t numPieces, const unsigned char* infoHash, size_t fastSetSize);

// Writes the detailed information about torrent loaded in dctx.
void print(std::ostream& o, const SharedHandle<DownloadContext>& dctx);

// Returns the value associated with INFO_HASH key in BITTORRENT
// attribute.
const unsigned char*
getInfoHash(const SharedHandle<DownloadContext>& downloadContext);

std::string
getInfoHashString(const SharedHandle<DownloadContext>& downloadContext);

} // namespace bittorrent

} // namespace aria2

#endif // _D_BITTORRENT_HELPER_H_
