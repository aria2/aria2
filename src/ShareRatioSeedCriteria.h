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
#ifndef _D_SHARE_RATIO_SEED_CRITERIA_H_
#define _D_SHARE_RATIO_SEED_CRITERIA_H_

#include "SeedCriteria.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "BtRegistry.h"

namespace aria2 {

class ShareRatioSeedCriteria : public SeedCriteria {
private:
  double ratio;
  SharedHandle<BtContext> btContext;
  SharedHandle<PeerStorage> peerStorage;
  SharedHandle<PieceStorage> pieceStorage;
  SharedHandle<BtRuntime> btRuntime;
public:
  ShareRatioSeedCriteria(double ratio, const SharedHandle<BtContext>& btContext)
    :ratio(ratio),
     btContext(btContext),
     peerStorage(PEER_STORAGE(btContext)),
     pieceStorage(PIECE_STORAGE(btContext)),
     btRuntime(BT_RUNTIME(btContext)) {}

  virtual ~ShareRatioSeedCriteria() {}

  virtual void reset() {}

  virtual bool evaluate() {
    if(btContext->getTotalLength() == 0) {
      return false;
    }
    TransferStat stat = peerStorage->calculateStat();
    int64_t allTimeUploadLength =
      btRuntime->getUploadLengthAtStartup()+stat.getSessionUploadLength();
    return ratio <=
      ((double)allTimeUploadLength)/pieceStorage->getCompletedLength();
  }

  void setRatio(double ratio) {
    this->ratio = ratio;
  }

  double getRatio() const {
    return ratio;
  }
};

} // namespace aria2

#endif // _D_SHARE_RATIO_SEED_CRITERIA_H_
