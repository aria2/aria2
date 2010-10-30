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

#include <fstream>
#include <iterator>

#include "RequestGroupMan.h"
#include "ServerStatMan.h"
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
    std::ofstream out(tempFilename.c_str(), std::ios::binary);
    if(!out) {
      return false;
    }
    save(out);
    out.flush();
    if(!out) {
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
void writeOption(std::ostream& out, const SharedHandle<Option>& op)
{
  const std::set<std::string>& requestOptions = listRequestOptions();
  for(std::set<std::string>::const_iterator itr = requestOptions.begin(),
        eoi = requestOptions.end(); itr != eoi; ++itr) {
    if(inCumulativeOpts(*itr)) {
      continue;
    }
    if(op->defined(*itr)) {
      out << " " << *itr << "=" << op->get(*itr) << "\n";
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
        out << " " << *opitr << "=" << *i << "\n";
      }
    }
  }
}
} // namespace

namespace {
void writeDownloadResult
(std::ostream& out, std::set<int64_t>& metainfoCache,
 const SharedHandle<DownloadResult>& dr)
{
  const SharedHandle<MetadataInfo>& mi = dr->metadataInfo;
  if(dr->belongsTo != 0 || (!mi.isNull() && mi->dataOnly())) {
    return;
  }
  if(mi.isNull()) {
    // only save first file entry
    if(dr->fileEntries.empty()) {
      return;
    }
    const SharedHandle<FileEntry>& file = dr->fileEntries[0];
    std::vector<std::string> uris;
    file->getUris(uris);
    if(uris.empty()) {
      return;
    }
    std::copy(uris.begin(), uris.end(),
              std::ostream_iterator<std::string>(out, "\t"));
    out << "\n";
  } else {
    if(metainfoCache.count(mi->getId()) != 0) {
      return;
    } else {
      metainfoCache.insert(mi->getId());
      out << mi->getUri() << "\n";
    }
  }
  writeOption(out, dr->option);
}
} // namespace

void SessionSerializer::save(std::ostream& out) const
{
  std::set<int64_t> metainfoCache;
  const std::deque<SharedHandle<DownloadResult> >& results =
    rgman_->getDownloadResults();
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr =
        results.begin(), eoi = results.end(); itr != eoi; ++itr) {
    if((*itr)->result == downloadresultcode::FINISHED) {
      continue;
    } else if((*itr)->result == downloadresultcode::IN_PROGRESS) {
      if(saveInProgress_) {
        writeDownloadResult(out, metainfoCache, *itr);
      }
    } else {
      // error download
      if(saveError_) {
        writeDownloadResult(out, metainfoCache, *itr);
      }
    }
  }
  if(saveInProgress_) {
    const std::deque<SharedHandle<RequestGroup> >& groups =
      rgman_->getRequestGroups();
    for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
          groups.begin(), eoi = groups.end(); itr != eoi; ++itr) {
      SharedHandle<DownloadResult> result = (*itr)->createDownloadResult();
      if(result->result == downloadresultcode::FINISHED) {
        continue;
      }
      writeDownloadResult(out, metainfoCache, result);
    }
  }
  if(saveWaiting_) {
    const std::deque<SharedHandle<RequestGroup> >& groups =
      rgman_->getReservedGroups();
    for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
          groups.begin(), eoi = groups.end(); itr != eoi; ++itr) {
      SharedHandle<DownloadResult> result = (*itr)->createDownloadResult();
      writeDownloadResult(out, metainfoCache, result);
    }
  }
}

} // namespace aria2


