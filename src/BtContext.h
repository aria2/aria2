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

#include "common.h"
#include "FileEntry.h"
#include "AnnounceTier.h"

#define INFO_HASH_LENGTH 20
#define MAX_PEER_ERROR 5
#define MAX_PEERS 55

typedef deque<AnnounceTierHandle> AnnounceTiers;

class BtContext {
public:
  virtual ~BtContext() {}

  enum FILE_MODE {
    SINGLE,
    MULTI
  };

  virtual const unsigned char* getInfoHash() const = 0;

  virtual int getInfoHashLength() const = 0;

  virtual string getInfoHashAsString() const = 0;

  virtual string getPieceHash(int index) const = 0;
  
  virtual const Strings& getPieceHashes() const = 0;

  virtual long long int getTotalLength() const = 0;

  virtual FILE_MODE getFileMode() const = 0;

  virtual FileEntries getFileEntries() const = 0;

  virtual AnnounceTiers getAnnounceTiers() const = 0;

  virtual void load(const string& torrentFile) = 0;

  virtual string getName() const = 0;
  
  virtual int getPieceLength() const = 0;

  virtual int getNumPieces() const = 0;

  /**
   * Returns the peer id of localhost, 20 byte length
   */
  virtual const unsigned char* getPeerId() = 0;
};

typedef SharedHandle<BtContext> BtContextHandle;

#endif // _D_BT_CONTEXT_H_
