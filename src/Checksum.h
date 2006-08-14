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
#ifndef _D_CHECKSUM_H_
#define _D_CHECKSUM_H_

#include "common.h"
#include "messageDigest.h"

#ifdef ENABLE_MESSAGE_DIGEST
class Checksum {
private:
  string md;
  MessageDigestContext::DigestAlgo algo;
public:
  Checksum(const string& md, MessageDigestContext::DigestAlgo algo):
    md(md),
    algo(algo) {}
  Checksum():
    algo(DIGEST_ALGO_SHA1) {}
  ~Checksum() {}

  bool isEmpty() const {
    return md.size() == 0;
  }

  void setMessageDigest(const string& md) {
    this->md = md;
  }
  const string& getMessageDigest() const {
    return md;
  }
  
  void setDigestAlgo(MessageDigestContext::DigestAlgo algo) {
    this->algo = algo;
  }
  const MessageDigestContext::DigestAlgo& getDigestAlgo() const {
    return algo;
  }
};
#else
class Checksum {
};
#endif // ENABLE_MESSAGE_DIGEST

#endif // _D_CHECKSUM_H_
