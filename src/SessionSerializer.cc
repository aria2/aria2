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

namespace aria2 {

SessionSerializer::SessionSerializer
(const SharedHandle<RequestGroupMan>& requestGroupMan):
  rgman_(requestGroupMan),
  saveError_(true),
  saveInProgress_(true),
  saveWaiting_(true) {}

bool SessionSerializer::save(const std::string& filename) const
{
  std::string tempFilename = strconcat(filename, "__temp");
  {
    BufferedFile fp(tempFilename, BufferedFile::WRITE);
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
const std::vector<std::string>& getCumulativeOpts()
{
  static std::string cumulativeOpts[] = { PREF_INDEX_OUT, PREF_HEADER };
  static std::vector<std::string> opts
    (vbegin(cumulativeOpts), vend(cumulativeOpts));
  return opts;
}
} // namespace

namespace {
bool inCumulativeOpts(const std::string& opt)
{
  const std::vector<std::string>& cumopts = getCumulativeOpts();
  for(std::vector<std::string>::const_iterator itr = cumopts.begin(),
        eoi = cumopts.end(); itr != eoi; ++itr) {
    if(opt == *itr) {
      return true;
    }
  }
  return false;
}
} // namespace

namespace {
bool writeOption(BufferedFile& fp, const SharedHandle<Option>& op)
{
  const std::set<std::string>& requestOptions = listRequestOptions();
  for(std::set<std::string>::const_iterator itr = requestOptions.begin(),
        eoi = requestOptions.end(); itr != eoi; ++itr) {
    if(inCumulativeOpts(*itr)) {
      continue;
    }
    if(op->defined(*itr)) {
      if(fp.printf(" %s=%s\n", (*itr).c_str(), op->get(*itr).c_str()) < 0) {
        return false;
      }
    }
  }
  const std::vector<std::string>& cumopts = getCumulativeOpts();
  for(std::vector<std::string>::const_iterator opitr = cumopts.begin(),
        eoi = cumopts.end(); opitr != eoi; ++opitr) {
    if(op->defined(*opitr)) {
      std::vector<std::string> v;
      util::split(op->get(*opitr), std::back_inserter(v), "\n",
                  false, false);
      for(std::vector<std::string>::const_iterator i = v.begin(), eoi = v.end();
          i != eoi; ++i) {
        if(fp.printf(" %s=%s\n", (*opitr).c_str(), (*i).c_str()) < 0) {
          return false;
        }
      }
    }
  }
  return true;
}
} // namespace

namespace {
bool writeDownloadResult
(BufferedFile& fp, std::set<int64_t>& metainfoCache,
 const SharedHandle<DownloadResult>& dr)
{
  const SharedHandle<MetadataInfo>& mi = dr->metadataInfo;
  if(dr->belongsTo != 0 || (mi && mi->dataOnly())) {
    return true;
  }
  if(!mi) {
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
  } else {
    if(metainfoCache.count(mi->getId()) != 0) {
      return true;
    } else {
      metainfoCache.insert(mi->getId());
      if(fp.printf("%s\n", mi->getUri().c_str()) < 0) {
        return false;
      }
    }
  }
  return writeOption(fp, dr->option);
}
} // namespace

bool SessionSerializer::save(BufferedFile& fp) const
{
  std::set<int64_t> metainfoCache;
  const std::deque<SharedHandle<DownloadResult> >& results =
    rgman_->getDownloadResults();
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr =
        results.begin(), eoi = results.end(); itr != eoi; ++itr) {
    if((*itr)->result == error_code::FINISHED ||
       (*itr)->result == error_code::REMOVED) {
      continue;
    } else if((*itr)->result == error_code::IN_PROGRESS) {
      if(saveInProgress_) {
        if(!writeDownloadResult(fp, metainfoCache, *itr)) {
          return false;
        }
      }
    } else {
      // error download
      if(saveError_) {
        if(!writeDownloadResult(fp, metainfoCache, *itr)) {
          return false;
        }
      }
    }
  }
  if(saveWaiting_) {
    const std::deque<SharedHandle<RequestGroup> >& groups =
      rgman_->getReservedGroups();
    for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
          groups.begin(), eoi = groups.end(); itr != eoi; ++itr) {
      SharedHandle<DownloadResult> result = (*itr)->createDownloadResult();
      if(!writeDownloadResult(fp, metainfoCache, result)) {
        return false;
      }
      // PREF_PAUSE was removed from option, so save it here looking
      // property separately.
      if((*itr)->isPauseRequested()) {
        if(fp.printf(" %s=true\n", PREF_PAUSE.c_str()) < 0) {
          return false;
        }
      }
    }
  }
  return true;
}

} // namespace aria2
