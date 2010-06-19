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
#ifndef _D_PEER_LIST_PROCESSOR_H_
#define _D_PEER_LIST_PROCESSOR_H_

#include "common.h"

#include <cstring>

#include "a2netcompat.h"
#include "Peer.h"
#include "ValueBase.h"

namespace aria2 {

class PeerListProcessor {
public:
  template<typename OutputIterator>
  void extractPeer(const ValueBase* peerData, OutputIterator dest)
  {
    class PeerListValueBaseVisitor:public ValueBaseVisitor {
    private:
      OutputIterator dest_;
    public:
      PeerListValueBaseVisitor(OutputIterator dest):dest_(dest) {}

      virtual ~PeerListValueBaseVisitor() {}

      virtual void visit(const String& peerData)
      {
        size_t length = peerData.s().size();
        if(length%6 == 0) {
          const char* base = peerData.s().data();
          for(size_t i = 0; i < length; i += 6) {
            struct in_addr in;
            memcpy(&in.s_addr, base+i, sizeof(uint32_t));
            std::string ipaddr = inet_ntoa(in);
            uint16_t port_nworder;
            memcpy(&port_nworder, base+i+4, sizeof(uint16_t));
            uint16_t port = ntohs(port_nworder);
            *dest_ = SharedHandle<Peer>(new Peer(ipaddr, port));
            ++dest_;
          }
        }
      }

      virtual void visit(const Integer& v) {}

      virtual void visit(const List& peerData)
      {
        for(List::ValueType::const_iterator itr = peerData.begin(),
              eoi = peerData.end(); itr != eoi; ++itr) {
          const Dict* peerDict = asDict(*itr);
          if(!peerDict) {
            continue;
          }
          static const std::string IP = "ip";
          static const std::string PORT = "port";
          const String* ip = asString(peerDict->get(IP));
          const Integer* port = asInteger(peerDict->get(PORT));
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
      PeerListValueBaseVisitor visitor(dest);
      peerData->accept(visitor);
    }
  }

  template<typename OutputIterator>
  void extractPeer(const SharedHandle<ValueBase>& peerData, OutputIterator dest)
  {
    return extractPeer(peerData.get(), dest);
  }
};

} // namespace aria2

#endif // _D_PEER_LIST_PROCESSOR_H_
