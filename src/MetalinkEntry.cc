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
#include "MetalinkEntry.h"
#include "Util.h"
#include <algorithm>

MetalinkEntry::MetalinkEntry() {}

MetalinkEntry::~MetalinkEntry() {
  for_each(resources.begin(), resources.end(), Deleter());
}

bool MetalinkEntry::check(const string& filename) const {
  unsigned char buf[20];
  int digestLength;
  const string* digestPtr;
  MessageDigestContext::HashAlgo algo;
  if(!sha1.empty()) {
    digestLength = 20;
    algo = MessageDigestContext::ALGO_SHA1;
    digestPtr = &sha1;
  } else if(!md5.empty()) {
    digestLength = 16;
    algo = MessageDigestContext::ALGO_MD5;
    digestPtr = &md5;
  } else {
    return true;
  }
  Util::fileChecksum(filename, buf, algo);
  return *digestPtr == Util::toHex(buf, digestLength);
}

class PrefOrder {
public:
  bool operator()(const MetalinkResource* res1, const MetalinkResource* res2) {
    return res1->preference > res2->preference;
  }
};

void MetalinkEntry::reorderResourcesByPreference() {
  random_shuffle(resources.begin(), resources.end());
  sort(resources.begin(), resources.end(), PrefOrder());
}

class Supported {
public:
  bool operator()(const MetalinkResource* res) {
    switch(res->type) {
    case MetalinkResource::TYPE_FTP:
    case MetalinkResource::TYPE_HTTP:
      return true;
    default:
      return false;
    }
  }
};

void MetalinkEntry::dropUnsupportedResource() {
  MetalinkResources::iterator split =
    partition(resources.begin(), resources.end(), Supported());
  resources.erase(split, resources.end());
}
