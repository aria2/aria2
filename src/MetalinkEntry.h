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
#ifndef _D_METALINK_ENTRY_H_
#define _D_METALINK_ENTRY_H_

#include "common.h"
#include "MetalinkResource.h"
#include <deque>

typedef deque<MetalinkResource*> MetalinkResources;

class MetalinkEntry {
public:
  string filename;
  string version;
  string language;
  string os;
  long long int size;
  string  md5;
  string  sha1;
public:
  MetalinkResources resources;
public:
  MetalinkEntry();
  ~MetalinkEntry();

  MetalinkEntry& operator=(const MetalinkEntry& metalinkEntry);

  bool check(const string& filename) const;

  void dropUnsupportedResource();

  void reorderResourcesByPreference();
};

#endif // _D_METALINK_ENTRY_H_
