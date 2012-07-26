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
#ifndef D_BITTORRENT_HELPER_H
#define D_BITTORRENT_HELPER_H

#include "common.h"

#include <cstring>
#include <string>
#include <vector>
#include <utility>

#include "SharedHandle.h"
#include "TorrentAttribute.h"
#include "a2netcompat.h"
#include "Peer.h"
#include "ValueBase.h"
#include "util.h"
#include "DownloadContext.h"
#include "TimeA2.h"

namespace aria2 {

class DownloadContext;
class Randomizer;
class Option;

namespace bittorrent {


extern const std::string SINGLE;

extern const std::string MULTI;

extern const std::string BITTORRENT;


void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const SharedHandle<Option>& option,
          const std::string& overrideName = "");

void load(const std::string& torrentFile,
          const SharedHandle<DownloadContext>& ctx,
          const SharedHandle<Option>& option,
          const std::vector<std::string>& uris,
          const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
                    const SharedHandle<DownloadContext>& ctx,
                    const SharedHandle<Option>& option,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const unsigned char* content, size_t length,
                    const SharedHandle<DownloadContext>& ctx,
                    const SharedHandle<Option>& option,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
                    const SharedHandle<DownloadContext>& ctx,
                    const SharedHandle<Option>& option,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const std::string& context,
                    const SharedHandle<DownloadContext>& ctx,
                    const SharedHandle<Option>& option,
                    const std::vector<std::string>& uris,
                    const std::string& defaultName,
                    const std::string& overrideName = "");

void loadFromMemory(const SharedHandle<ValueBase>& torrent,
                    const SharedHandle<DownloadContext>& ctx,
                    const SharedHandle<Option>& option,
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
void checkBegin(int32_t begin, int32_t pieceLength);
void checkLength(int32_t length);
void checkRange(int32_t begin, int32_t length, int32_t pieceLength);
void checkBitfield
(const unsigned char* bitfield, size_t bitfieldLength, size_t pieces);

// Initialize msg with 0 and set payloadLength and messageId.
void createPeerMessageString
(unsigned char* msg, size_t msgLength, size_t payloadLength, uint8_t messageId);

/**
 * Creates compact form(packed addresss + 2bytes port) and stores the
 * results in compact.  This function looks addr and if it is IPv4
 * address, it stores 6bytes in compact and if it is IPv6, it stores
 * 18bytes in compact.  So compact must be at least 18 bytes and
 * pre-allocated.  Returns the number of written bytes; for IPv4
 * address, it is 6 and for IPv6, it is 18. On failure, returns 0.
 */
int packcompact
(unsigned char* compact, const std::string& addr, uint16_t port);

/**
 * Unpack packed address and port in compact and returns address and
 * port pair.  family must be AF_INET or AF_INET6.  If family is
 * AF_INET, first 6 bytes from compact is used.  If family is
 * AF_INET6, first 18 bytes from compact is used.  On failure, returns
 * std::pair<std::string, uint16_t>().
 */
std::pair<std::string, uint16_t>
unpackcompact(const unsigned char* compact, int family);

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

// Removes announce URI in uris from attrs.  If uris contains '*', all
// announce URIs are removed.
void removeAnnounceUri
(const SharedHandle<TorrentAttribute>& attrs,
 const std::vector<std::string>& uris);

// Adds announce URI in uris to attrs. Each URI in uris creates its
// own tier.
void addAnnounceUri
(const SharedHandle<TorrentAttribute>& attrs,
 const std::vector<std::string>& uris);

// This helper function uses 2 option values: PREF_BT_TRACKER and
// PREF_BT_EXCLUDE_TRACKER. First, the value of
// PREF_BT_EXCLUDE_TRACKER is converted to std::vector<std::string>
// and call removeAnnounceUri(). Then the value of PREF_BT_TRACKER is
// converted to std::vector<std::string> and call addAnnounceUri().
void adjustAnnounceUri
(const SharedHandle<TorrentAttribute>& attrs,
 const SharedHandle<Option>& option);

template<typename OutputIterator>
void extractPeer(const ValueBase* peerData, int family, OutputIterator dest)
{
  class PeerListValueBaseVisitor:public ValueBaseVisitor {
  private:
    OutputIterator dest_;
    int family_;
  public:
    PeerListValueBaseVisitor(OutputIterator dest, int family):
      dest_(dest),
      family_(family) {}

    virtual ~PeerListValueBaseVisitor() {}

    virtual void visit(const String& peerData)
    {
      int unit = family_ == AF_INET?6:18;
      size_t length = peerData.s().size();
      if(length%unit == 0) {
        const unsigned char* base =
          reinterpret_cast<const unsigned char*>(peerData.s().data());
        const unsigned char* end = base+length;
        for(; base != end; base += unit) {
          std::pair<std::string, uint16_t> p = unpackcompact(base, family_);
          if(p.first.empty()) {
            continue;
          }
          *dest_++ = SharedHandle<Peer>(new Peer(p.first, p.second));
        }
      }
    }

    virtual void visit(const Integer& v) {}
    virtual void visit(const Bool& v) {}
    virtual void visit(const Null& v) {}

    virtual void visit(const List& peerData)
    {
      for(List::ValueType::const_iterator itr = peerData.begin(),
            eoi = peerData.end(); itr != eoi; ++itr) {
        const Dict* peerDict = downcast<Dict>(*itr);
        if(!peerDict) {
          continue;
        }
        static const std::string IP = "ip";
        static const std::string PORT = "port";
        const String* ip = downcast<String>(peerDict->get(IP));
        const Integer* port = downcast<Integer>(peerDict->get(PORT));
        if(!ip || !port || !(0 < port->i() && port->i() < 65536)) {
          continue;
        }
        *dest_ = SharedHandle<Peer>(new Peer(ip->s(), port->i()));
        ++dest_;
      }
    }

    virtual void visit(const Dict& v) {}
  };
  if(peerData) {
    PeerListValueBaseVisitor visitor(dest, family);
    peerData->accept(visitor);
  }
}

template<typename OutputIterator>
void extractPeer
(const SharedHandle<ValueBase>& peerData, int family, OutputIterator dest)
{
  return extractPeer(peerData.get(), family, dest);
}

int getCompactLength(int family);

// Writes the detailed information about torrent loaded in dctx.
template<typename Output>
void print(Output& o, const SharedHandle<DownloadContext>& dctx)
{
  SharedHandle<TorrentAttribute> torrentAttrs = getTorrentAttrs(dctx);
  o.write("*** BitTorrent File Information ***\n");
  if(!torrentAttrs->comment.empty()) {
    o.printf("Comment: %s\n", torrentAttrs->comment.c_str());
  }
  if(torrentAttrs->creationDate) {
    o.printf("Creation Date: %s\n",
             Time(torrentAttrs->creationDate).toHTTPDate().c_str());
  }
  if(!torrentAttrs->createdBy.empty()) {
    o.printf("Created By: %s\n", torrentAttrs->createdBy.c_str());
  }
  o.printf("Mode: %s\n", torrentAttrs->mode.c_str());
  o.write("Announce:\n");
  for(std::vector<std::vector<std::string> >::const_iterator tierIter =
        torrentAttrs->announceList.begin(),
        eoi = torrentAttrs->announceList.end(); tierIter != eoi; ++tierIter) {
    for(std::vector<std::string>::const_iterator i = (*tierIter).begin(),
          eoi2 = (*tierIter).end(); i != eoi2; ++i) {
      o.printf(" %s", (*i).c_str());
    }
    o.write("\n");
  }
  o.printf("Info Hash: %s\n", util::toHex(torrentAttrs->infoHash).c_str());
  o.printf("Piece Length: %sB\n",
           util::abbrevSize(dctx->getPieceLength()).c_str());
  o.printf("The Number of Pieces: %lu\n",
           static_cast<unsigned long>(dctx->getNumPieces()));
  o.printf("Total Length: %sB (%s)\n",
           util::abbrevSize(dctx->getTotalLength()).c_str(),
           util::uitos(dctx->getTotalLength(), true).c_str());
  if(!torrentAttrs->urlList.empty()) {
    o.write("URL List:\n");
    for(std::vector<std::string>::const_iterator i =
          torrentAttrs->urlList.begin(),
          eoi = torrentAttrs->urlList.end(); i != eoi; ++i) {
      o.printf(" %s\n", (*i).c_str());
    }
  }
  o.printf("Name: %s\n", torrentAttrs->name.c_str());
  o.printf("Magnet URI: %s\n", torrent2Magnet(torrentAttrs).c_str());
  util::toStream
    (dctx->getFileEntries().begin(), dctx->getFileEntries().end(), o);
}

} // namespace bittorrent

} // namespace aria2

#endif // D_BITTORRENT_HELPER_H
