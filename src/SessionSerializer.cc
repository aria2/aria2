/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "SessionSerializer.h"

#include <cstdio>
#include <iterator>

#include "RequestGroupMan.h"
#include "a2functional.h"
#include "File.h"
#include "A2STR.h"
#include "download_helper.h"
#include "Option.h"
#include "DownloadResult.h"
#include "FileEntry.h"
#include "prefs.h"
#include "util.h"
#include "array_fun.h"
#include "BufferedFile.h"
#include "OptionParser.h"
#include "OptionHandler.h"

namespace aria2 {

SessionSerializer::SessionSerializer
(const SharedHandle<RequestGroupMan>& requestGroupMan):
  rgman_(requestGroupMan),
  saveError_(true),
  saveInProgress_(true),
  saveWaiting_(true) {}

bool SessionSerializer::save(const std::string& filename) const
{
  std::string tempFilename = filename;
  tempFilename += "__temp";
  {
    BufferedFile fp(tempFilename.c_str(), BufferedFile::WRITE);
    if(!fp) {
      return false;
    }
    if(!save(fp) || fp.close() == EOF) {
      return false;
    }
  }
  return File(tempFilename).renameTo(filename);
}

namespace {
bool writeOption(BufferedFile& fp, const SharedHandle<Option>& op)
{
  const SharedHandle<OptionParser>& oparser = OptionParser::getInstance();
  for(size_t i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if(h && h->getInitialOption() && op->defined(pref)) {
      if(h->getCumulative()) {
        const std::string& val = op->get(pref);
        std::vector<std::string> v;
        util::split(val.begin(), val.end(), std::back_inserter(v), '\n',
                    false, false);
        for(std::vector<std::string>::const_iterator j = v.begin(),
              eoj = v.end(); j != eoj; ++j) {
          if(fp.printf(" %s=%s\n", pref->k, (*j).c_str()) < 0) {
            return false;
          }
        }
      } else {
        if(fp.printf(" %s=%s\n", pref->k, op->get(pref).c_str()) < 0) {
          return false;
        }
      }
    }
  }
  return true;
}
} // namespace

// The downloads whose followedBy() is empty is persisited with its
// GID without no problem. For other cases, there are several patterns.
//
// 1. magnet URI
//  GID of metadata download is persisted.
// 2. URI to torrent file
//  GID of torrent file download is persisted.
// 3. URI to metalink file
//  GID of metalink file download is persisted.
// 4. local torrent file
//  GID of torrent download itself is persisted.
// 5. local metalink file
//  No GID is persisted. GID is saved but it is just a random GID.

namespace {
bool writeDownloadResult
(BufferedFile& fp, std::set<a2_gid_t>& metainfoCache,
 const SharedHandle<DownloadResult>& dr)
{
  const SharedHandle<MetadataInfo>& mi = dr->metadataInfo;
  if(dr->belongsTo != 0 || (mi && mi->dataOnly())) {
    return true;
  }
  if(!mi) {
    // With --force-save option, same gid may be saved twice. (e.g.,
    // Downloading .meta4 followed by its conent download. First
    // .meta4 download is saved and second content download is also
    // saved with the same gid.)
    if(metainfoCache.count(dr->gid->getNumericId()) != 0) {
      return true;
    } else {
      metainfoCache.insert(dr->gid->getNumericId());
    }
    // only save first file entry
    if(dr->fileEntries.empty()) {
      return true;
    }
    const SharedHandle<FileEntry>& file = dr->fileEntries[0];
    std::vector<std::string> uris;
    file->getUris(uris);
    if(uris.empty()) {
      return true;
    }
    for(std::vector<std::string>::const_iterator i = uris.begin(),
          eoi = uris.end(); i != eoi; ++i) {
      if(fp.printf("%s\t", (*i).c_str()) < 0) {
        return false;
      }
    }
    if(fp.write("\n", 1) != 1) {
      return false;
    }
    if(fp.printf(" gid=%s\n", dr->gid->toHex().c_str()) < 0) {
      return false;
    }
  } else {
    if(metainfoCache.count(mi->getGID()) != 0) {
      return true;
    } else {
      metainfoCache.insert(mi->getGID());
      if(fp.printf("%s\n", mi->getUri().c_str()) < 0) {
        return false;
      }
      // For downloads generated by metadata (e.g., BitTorrent,
      // Metalink), save gid of Metadata download.
      if(fp.printf(" gid=%s\n", GroupId::toHex(mi->getGID()).c_str()) < 0) {
        return false;
      }
    }
  }
  return writeOption(fp, dr->option);
}
} // namespace

bool SessionSerializer::save(BufferedFile& fp) const
{
  std::set<a2_gid_t> metainfoCache;
  const DownloadResultList& results = rgman_->getDownloadResults();
  for(DownloadResultList::SeqType::const_iterator itr = results.begin(),
        eoi = results.end(); itr != eoi; ++itr) {
    const SharedHandle<DownloadResult>& dr = (*itr).second;
    if(dr->result == error_code::FINISHED ||
       dr->result == error_code::REMOVED) {
      if(dr->option->getAsBool(PREF_FORCE_SAVE)) {
        if(!writeDownloadResult(fp, metainfoCache, dr)) {
          return false;
        }
      } else {
        continue;
      }
    } else if(dr->result == error_code::IN_PROGRESS) {
      if(saveInProgress_) {
        if(!writeDownloadResult(fp, metainfoCache, dr)) {
          return false;
        }
      }
    } else {
      // error download
      if(saveError_) {
        if(!writeDownloadResult(fp, metainfoCache, dr)) {
          return false;
        }
      }
    }
  }
  if(saveWaiting_) {
    const RequestGroupList& groups = rgman_->getReservedGroups();
    for(RequestGroupList::SeqType::const_iterator itr = groups.begin(),
          eoi = groups.end(); itr != eoi; ++itr) {
      const SharedHandle<RequestGroup>& rg = (*itr).second;
      SharedHandle<DownloadResult> result = rg->createDownloadResult();
      if(!writeDownloadResult(fp, metainfoCache, result)) {
        return false;
      }
      // PREF_PAUSE was removed from option, so save it here looking
      // property separately.
      if(rg->isPauseRequested()) {
        if(fp.printf(" %s=true\n", PREF_PAUSE->k) < 0) {
          return false;
        }
      }
    }
  }
  return true;
}

} // namespace aria2
