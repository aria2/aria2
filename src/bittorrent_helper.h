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
#ifndef _D_BITTORRENT_HELPER_H_
#define _D_BITTORRENT_HELPER_H_

#include "common.h"

#include <string>
#include <vector>
#include <utility>

#include "SharedHandle.h"
#include "AnnounceTier.h"
#include "util.h"
#include "TorrentAttribute.h"

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

extern const std::string CREATION_DATE;

extern const std::string COMMENT;

extern const std::string CREATED_BY;

extern const std::string SINGLE;

extern const std::string MULTI;

extern const std::string BITTORRENT;

extern const std::string METADATA_SIZE;

extern const std::string METADATA;

void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const std::string& overrideName = "");

void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const std::vector<std::string>& uris,
          const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
                    const SharedHandle<DownloadContext>& ctx,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

// Parses BitTorrent Magnet URI and returns
// SharedHandle<TorrentAttribute> which includes infoHash, name and
// announceList. If parsing operation failed, an RecoverableException
// will be thrown.  infoHash and name are string and announceList is a
// list of list of announce URI.
//
// magnet:?xt=urn:btih:<info-hash>&dn=<name>&tr=<tracker-url>
// <info-hash> comes in 2 flavors: 40bytes hexadecimal ascii string,
// or 32bytes Base32 encoded string.
SharedHandle<TorrentAttribute> parseMagnet(const std::string& magnet);

// Parses BitTorrent Magnet URI and set them in ctx as a
// bittorrent::BITTORRENT attibute. If parsing operation failed, an
// RecoverableException will be thrown.
void loadMagnet
(const std::string& magnet, const SharedHandle<DownloadContext>& ctx);

// Generates Peer ID. BitTorrent specification says Peer ID is 20-byte
// length.  This function uses peerIdPrefix as a Peer ID and it is
// less than 20bytes, random bytes are generated and appened to it. If
// peerIdPrefix is larger than 20bytes, first 20bytes are used.
std::string generatePeerId(const std::string& peerIdPrefix);

// Generates Peer ID and stores it in static variable. This function
// uses generatePeerId(peerIdPrefix) to produce Peer ID.
const std::string& generateStaticPeerId(const std::string& peerIdPrefix);

// Returns Peer ID statically stored by generateStaticPeerId().  If
// Peer ID is not stored yet, this function calls
// generateStaticPeerId("aria2-")
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

SharedHandle<TorrentAttribute> getTorrentAttrs
(const SharedHandle<DownloadContext>& dctx);

// Returns the value associated with INFO_HASH key in BITTORRENT
// attribute.
const unsigned char*
getInfoHash(const SharedHandle<DownloadContext>& downloadContext);

std::string
getInfoHashString(const SharedHandle<DownloadContext>& downloadContext);

// Returns 4bytes unsigned integer located at offset pos.  The integer
// in msg is network byte order. This function converts it into host
// byte order and returns it.
uint32_t getIntParam(const unsigned char* msg, size_t pos);

// Returns 2bytes unsigned integer located at offset pos.  The integer
// in msg is network byte order. This function converts it into host
// byte order and returns it.
uint16_t getShortIntParam(const unsigned char* msg, size_t pos);

// Put param at location pointed by dest. param is converted into
// network byte order.
void setIntParam(unsigned char* dest, uint32_t param);

// Put param at location pointed by dest. param is converted into
// network byte order.
void setShortIntParam(unsigned char* dest, uint16_t param);

// Returns message ID located at first byte:msg[0]
uint8_t getId(const unsigned char* msg);
  
void checkIndex(size_t index, size_t pieces);
void checkBegin(uint32_t begin, size_t pieceLength);
void checkLength(size_t length);
void checkRange(uint32_t begin, size_t length, size_t pieceLength);
void checkBitfield
(const unsigned char* bitfield, size_t bitfieldLength, size_t pieces);

// Initialize msg with 0 and set payloadLength and messageId.
void createPeerMessageString
(unsigned char* msg, size_t msgLength, size_t payloadLength, uint8_t messageId);

/**
 * Creates compact tracker format(6bytes for ipv4 address and port)
 * and stores the results in compact.
 * compact must be at least 6 bytes and pre-allocated.
 * Returns true if creation is successful, otherwise returns false.
 * The example of failure reason is that addr is not numbers-and-dots
 * notation.
 */
bool createcompact
(unsigned char* compact, const std::string& addr, uint16_t port);

// Unpack compact into pair of IPv4 address and port.
std::pair<std::string, uint16_t> unpackcompact(const unsigned char* compact);

// Throws exception if threshold >= actual
void assertPayloadLengthGreater
(size_t threshold, size_t actual, const std::string& msgName);

// Throws exception if expected != actual
void assertPayloadLengthEqual
(size_t expected, size_t actual, const std::string& msgName);

// Throws exception if expected is not equal to id from data.
void assertID
(uint8_t expected, const unsigned char* data, const std::string& msgName);

// Converts attrs into torrent data. This function does not guarantee
// the returned string is valid torrent data.
std::string metadata2Torrent
(const std::string& metadata, const SharedHandle<TorrentAttribute>& attrs);

// Constructs BitTorrent Magnet URI using attrs.
std::string torrent2Magnet(const SharedHandle<TorrentAttribute>& attrs);

} // namespace bittorrent

} // namespace aria2

#endif // _D_BITTORRENT_HELPER_H_
