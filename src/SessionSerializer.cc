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

#if HAVE_ZLIB
#include "GZipFile.h"
#endif

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
    SharedHandle<IOFile> fp;
#if HAVE_ZLIB
    if (util::endsWith(filename, ".gz")) {
      fp.reset(new GZipFile(tempFilename.c_str(), IOFile::WRITE));
    }
    else
#endif
    {
     fp.reset(new BufferedFile(tempFilename.c_str(), IOFile::WRITE));
    }
    if(!*fp) {
      return false;
    }
    if(!save(*fp) || fp->close() == EOF) {
      return false;
    }
  }
  return File(tempFilename).renameTo(filename);
}

namespace {
// Write 1 line of option name/value pair. This function returns true
// if it succeeds, or false.
bool writeOptionLine(IOFile& fp, const Pref* pref,
                     const std::string& val)
{
  size_t prefLen = strlen(pref->k);
  return fp.write(" ", 1) == 1 &&
    fp.write(pref->k, prefLen) == prefLen &&
    fp.write("=", 1) == 1 &&
    fp.write(val.c_str(), val.size()) == val.size() &&
    fp.write("\n", 1) == 1;
}
} // namespace

namespace {
bool writeOption(IOFile& fp, const SharedHandle<Option>& op)
{
  const SharedHandle<OptionParser>& oparser = OptionParser::getInstance();
  for(size_t i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if(h && h->getInitialOption() && op->definedLocal(pref)) {
      if(h->getCumulative()) {
        const std::string& val = op->get(pref);
        std::vector<std::string> v;
        util::split(val.begin(), val.end(), std::back_inserter(v), '\n',
                    false, false);
        for(std::vector<std::string>::const_iterator j = v.begin(),
              eoj = v.end(); j != eoj; ++j) {
          if(!writeOptionLine(fp, pref, *j)) {
            return false;
          }
        }
      } else {
        if(!writeOptionLine(fp, pref, op->get(pref))) {
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
(IOFile& fp, std::set<a2_gid_t>& metainfoCache,
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
    if(file->getRemainingUris().empty()) {
      return true;
    }
    for(std::deque<std::string>::const_iterator i =
          file->getRemainingUris().begin(),
          eoi = file->getRemainingUris().end(); i != eoi; ++i) {
      if (fp.write((*i).c_str(), (*i).size()) != (*i).size() ||
          fp.write("\t", 1) != 1) {
        return false;
      }
    }
    if(fp.write("\n", 1) != 1) {
      return false;
    }
    if(!writeOptionLine(fp, PREF_GID, dr->gid->toHex())) {
      return false;
    }
  } else {
    if(metainfoCache.count(mi->getGID()) != 0) {
      return true;
    } else {
      metainfoCache.insert(mi->getGID());
      if (fp.write(mi->getUri().c_str(),
                   mi->getUri().size()) != mi->getUri().size() ||
          fp.write("\n", 1) != 1) {
        return false;
      }
      // For downloads generated by metadata (e.g., BitTorrent,
      // Metalink), save gid of Metadata download.
      if(!writeOptionLine(fp, PREF_GID, GroupId::toHex(mi->getGID()))) {
        return false;
      }
    }
  }
  return writeOption(fp, dr->option);
}
} // namespace

bool SessionSerializer::save(IOFile& fp) const
{
  std::set<a2_gid_t> metainfoCache;
  const DownloadResultList& results = rgman_->getDownloadResults();
  for(DownloadResultList::const_iterator itr = results.begin(),
        eoi = results.end(); itr != eoi; ++itr) {
    const SharedHandle<DownloadResult>& dr = *itr;
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
  {
    // Save active downloads.
    const RequestGroupList& groups = rgman_->getRequestGroups();
    for(RequestGroupList::const_iterator itr = groups.begin(),
          eoi = groups.end(); itr != eoi; ++itr) {
      const SharedHandle<RequestGroup>& rg = *itr;
      SharedHandle<DownloadResult> dr = rg->createDownloadResult();
      bool stopped = dr->result == error_code::FINISHED ||
        dr->result == error_code::REMOVED;
      if((!stopped && saveInProgress_) ||
         (stopped && dr->option->getAsBool(PREF_FORCE_SAVE))) {
        if(!writeDownloadResult(fp, metainfoCache, dr)) {
          return false;
        }
      }
    }
  }
  if(saveWaiting_) {
    const RequestGroupList& groups = rgman_->getReservedGroups();
    for(RequestGroupList::const_iterator itr = groups.begin(),
          eoi = groups.end(); itr != eoi; ++itr) {
      const SharedHandle<RequestGroup>& rg = *itr;
      SharedHandle<DownloadResult> result = rg->createDownloadResult();
      if(!writeDownloadResult(fp, metainfoCache, result)) {
        return false;
      }
      // PREF_PAUSE was removed from option, so save it here looking
      // property separately.
      if(rg->isPauseRequested()) {
        if(!writeOptionLine(fp, PREF_PAUSE, A2_V_TRUE)) {
          return false;
        }
      }
    }
  }
  return true;
}

} // namespace aria2
