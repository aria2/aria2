/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_SHARE_RATIO_SEED_CRITERIA_H_
#define _D_SHARE_RATIO_SEED_CRITERIA_H_

#include "SeedCriteria.h"
#include "TorrentMan.h"

class ShareRatioSeedCriteria : public SeedCriteria {
private:
  double ratio;
  TorrentMan* torrentMan;
public:
  ShareRatioSeedCriteria(double ratio, TorrentMan* torrentMan)
    :ratio(ratio),
     torrentMan(torrentMan) {}
  virtual ~ShareRatioSeedCriteria() {}

  virtual void reset() {}

  virtual bool evaluate() {
    if(torrentMan->getDownloadLength() == 0) {
      return false;
    }
    return ratio <=
      ((double)torrentMan->getUploadLength())/torrentMan->getDownloadLength();
  }

  void setRatio(double ratio) {
    this->ratio = ratio;
  }

  double getRatio() const {
    return ratio;
  }
};

#endif // _D_SHARE_RATIO_SEED_CRITERIA_H_
