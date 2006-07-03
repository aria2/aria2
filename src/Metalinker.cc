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
#include "Metalinker.h"
#include <algorithm>

Metalinker::Metalinker() {
}

Metalinker::~Metalinker() {
  for_each(entries.begin(), entries.end(), Deleter());
}

class EntryQuery {
private:
  string version;
  string language;
  string os;
public:
  EntryQuery(const string& version,
	     const string& language,
	     const string& os):version(version),
			       language(language),
			       os(os) {}
  bool operator()(const MetalinkEntry* entry) {
    if(!version.empty()) {
      if(version != entry->version) {
	return false;
      }
    }
    if(!language.empty()) {
      if(language != entry->language) {
	return false;
      }
    }
    if(!os.empty()) {
      if(os != entry->os) {
	return false;
      }
    }
    return true;
  }
};

MetalinkEntry* Metalinker::queryEntry(const string& version,
				      const string& language,
				      const string& os) const {
  MetalinkEntries::const_iterator itr =
    find_if(entries.begin(), entries.end(),
	    EntryQuery(version, language, os));
  if(itr == entries.end()) {
    return NULL;
  } else {
    return *itr;
  }
}
