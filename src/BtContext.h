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
#ifndef _D_BT_CONTEXT_H_
#define _D_BT_CONTEXT_H_

#include "DownloadContext.h"

#include <utility>
#include <deque>

#include "IntSequence.h"

namespace aria2 {

class AnnounceTier;
class RequestGroup;

class BtContext:public DownloadContext {
protected:
  bool _private;
public:
  BtContext():_private(false) {}
  
  virtual ~BtContext() {}

  virtual const unsigned char* getInfoHash() const = 0;

  virtual size_t getInfoHashLength() const = 0;

  virtual const std::string& getInfoHashAsString() const = 0;

  virtual const std::deque<SharedHandle<AnnounceTier> >&
  getAnnounceTiers() const = 0;

  virtual void load(const std::string& torrentFile,
		    const std::string& overrideName = "") = 0;

  /**
   * Returns the peer id of localhost, 20 byte length
   */
  virtual const unsigned char* getPeerId() = 0;

  bool isPrivate() const
  {
    return _private;
  }

  virtual void computeFastSet
  (std::deque<size_t>& fastSet, const std::string& ipaddr, size_t fastSetSize) = 0;
  
  virtual RequestGroup* getOwnerRequestGroup() = 0;

  virtual std::deque<std::pair<std::string, uint16_t> >& getNodes() = 0;

  virtual const std::string& getName() const = 0;

  virtual void setFileFilter(IntSequence seq) = 0;

  static const std::string C_NAME;

  static const std::string C_FILES;

  static const std::string C_LENGTH;

  static const std::string C_PATH;

  static const std::string C_INFO;

  static const std::string C_PIECES;

  static const std::string C_PIECE_LENGTH;

  static const std::string C_PRIVATE;

  // This is just a string "1". Used as a value of "private" flag.
  static const std::string C_PRIVATE_ON;

  static const std::string C_URL_LIST;

  static const std::string C_ANNOUNCE;

  static const std::string C_ANNOUNCE_LIST;

  static const std::string C_NODES;

};

class BtContext;
typedef SharedHandle<BtContext> BtContextHandle;

} // namespace aria2

#endif // _D_BT_CONTEXT_H_
