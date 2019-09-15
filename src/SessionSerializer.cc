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
#include <cassert>
#include <iterator>
#include <set>

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
#include "SHA1IOFile.h"

#if HAVE_ZLIB
#  include "GZipFile.h"
#endif

namespace aria2 {

SessionSerializer::SessionSerializer(RequestGroupMan* requestGroupMan)
    : rgman_{requestGroupMan},
      saveError_{true},
      saveInProgress_{true},
      saveWaiting_{true}
{
}

bool SessionSerializer::save(const std::string& filename) const
{
  std::string tempFilename = filename;
  tempFilename += "__temp";
  {
    std::unique_ptr<IOFile> fp;
#if HAVE_ZLIB
    if (util::endsWith(filename, ".gz")) {
      fp = make_unique<GZipFile>(tempFilename.c_str(), IOFile::WRITE);
    }
    else
#endif
    {
      fp = make_unique<BufferedFile>(tempFilename.c_str(), IOFile::WRITE);
    }
    if (!*fp) {
      return false;
    }
    if (!save(*fp) || fp->close() == EOF) {
      return false;
    }
  }
  return File(tempFilename).renameTo(filename);
}

namespace {
// Write 1 line of option name/value pair. This function returns true
// if it succeeds, or false.
bool writeOptionLine(IOFile& fp, PrefPtr pref, const std::string& val)
{
  size_t prefLen = strlen(pref->k);
  return fp.write(" ", 1) == 1 && fp.write(pref->k, prefLen) == prefLen &&
         fp.write("=", 1) == 1 &&
         fp.write(val.c_str(), val.size()) == val.size() &&
         fp.write("\n", 1) == 1;
}
} // namespace

namespace {
bool writeOption(IOFile& fp, const std::shared_ptr<Option>& op)
{
  const std::shared_ptr<OptionParser>& oparser = OptionParser::getInstance();
  for (size_t i = 1, len = option::countOption(); i < len; ++i) {
    PrefPtr pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if (h && h->getInitialOption() && op->definedLocal(pref)) {
      if (h->getCumulative()) {
        const std::string& val = op->get(pref);
        std::vector<std::string> v;
        util::split(val.begin(), val.end(), std::back_inserter(v), '\n', false,
                    false);
        for (std::vector<std::string>::const_iterator j = v.begin(),
                                                      eoj = v.end();
             j != eoj; ++j) {
          if (!writeOptionLine(fp, pref, *j)) {
            return false;
          }
        }
      }
      else {
        if (!writeOptionLine(fp, pref, op->get(pref))) {
          return false;
        }
      }
    }
  }
  return true;
}
} // namespace

namespace {
template <typename T> class Unique {
  typedef T type;
  struct PointerCmp {
    inline bool operator()(const type* x, const type* y) const
    {
      return *x < *y;
    }
  };
  std::set<const type*, PointerCmp> known;

public:
  inline bool operator()(const type& v) { return known.insert(&v).second; }
};

bool writeUri(IOFile& fp, const std::string& uri)
{
  return fp.write(uri.c_str(), uri.size()) == uri.size() &&
         fp.write("\t", 1) == 1;
}

template <typename InputIterator, class UnaryPredicate>
bool writeUri(IOFile& fp, InputIterator first, InputIterator last,
              UnaryPredicate& filter)
{
  for (; first != last; ++first) {
    if (!filter(*first)) {
      continue;
    }
    if (!writeUri(fp, *first)) {
      return false;
    }
  }
  return true;
}
} // namespace

// The downloads whose followedBy() is empty is persisted with its
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
bool writeDownloadResult(IOFile& fp, std::set<a2_gid_t>& metainfoCache,
                         const std::shared_ptr<DownloadResult>& dr,
                         bool pauseRequested)
{
  const std::shared_ptr<MetadataInfo>& mi = dr->metadataInfo;
  if (dr->belongsTo != 0 || (mi && mi->dataOnly()) || !dr->followedBy.empty()) {
    return true;
  }
  if (!mi) {
    // With --force-save option, same gid may be saved twice. (e.g.,
    // Downloading .meta4 followed by its content download. First
    // .meta4 download is saved and second content download is also
    // saved with the same gid.)
    if (metainfoCache.count(dr->gid->getNumericId()) != 0) {
      return true;
    }
    else {
      metainfoCache.insert(dr->gid->getNumericId());
    }
    // only save first file entry
    if (dr->fileEntries.empty()) {
      return true;
    }
    const std::shared_ptr<FileEntry>& file = dr->fileEntries[0];
    // Don't save download if there are no URIs.
    const bool hasRemaining = !file->getRemainingUris().empty();
    const bool hasSpent = !file->getSpentUris().empty();
    if (!hasRemaining && !hasSpent) {
      return true;
    }

    // Save spent URIs + remaining URIs. Remove URI in spent URI which
    // also exists in remaining URIs.
    {
      Unique<std::string> unique;
      if (hasRemaining && !writeUri(fp, file->getRemainingUris().begin(),
                                    file->getRemainingUris().end(), unique)) {
        return false;
      }
      if (hasSpent && !writeUri(fp, file->getSpentUris().begin(),
                                file->getSpentUris().end(), unique)) {
        return false;
      }
    }
    if (fp.write("\n", 1) != 1) {
      return false;
    }
    if (!writeOptionLine(fp, PREF_GID, dr->gid->toHex())) {
      return false;
    }
  }
  else {
    if (metainfoCache.count(mi->getGID()) != 0) {
      return true;
    }
    else {
      metainfoCache.insert(mi->getGID());
      if (fp.write(mi->getUri().c_str(), mi->getUri().size()) !=
              mi->getUri().size() ||
          fp.write("\n", 1) != 1) {
        return false;
      }
      // For downloads generated by metadata (e.g., BitTorrent,
      // Metalink), save gid of Metadata download.
      if (!writeOptionLine(fp, PREF_GID, GroupId::toHex(mi->getGID()))) {
        return false;
      }
    }
  }

  // PREF_PAUSE was removed from option, so save it here looking
  // property separately.
  if (pauseRequested) {
    if (!writeOptionLine(fp, PREF_PAUSE, A2_V_TRUE)) {
      return false;
    }
  }

  return writeOption(fp, dr->option);
}
} // namespace

namespace {
template <typename InputIt>
bool saveDownloadResult(IOFile& fp, std::set<a2_gid_t>& metainfoCache,
                        InputIt first, InputIt last, bool saveInProgress,
                        bool saveError)
{
  for (; first != last; ++first) {
    const auto& dr = *first;
    auto save = false;
    switch (dr->result) {
    case error_code::FINISHED:
    case error_code::REMOVED:
      save = dr->option->getAsBool(PREF_FORCE_SAVE);
      break;
    case error_code::IN_PROGRESS:
      save = saveInProgress;
      break;
    case error_code::RESOURCE_NOT_FOUND:
    case error_code::MAX_FILE_NOT_FOUND:
      save = saveError && dr->option->getAsBool(PREF_SAVE_NOT_FOUND);
      break;
    default:
      save = saveError;
      break;
    }
    if (save && !writeDownloadResult(fp, metainfoCache, dr, false)) {
      return false;
    }
  }
  return true;
}
} // namespace

bool SessionSerializer::save(IOFile& fp) const
{
  std::set<a2_gid_t> metainfoCache;

  const auto& unfinishedResults = rgman_->getUnfinishedDownloadResult();
  if (!saveDownloadResult(fp, metainfoCache, std::begin(unfinishedResults),
                          std::end(unfinishedResults), saveInProgress_,
                          saveError_)) {
    return false;
  }

  const auto& results = rgman_->getDownloadResults();
  if (!saveDownloadResult(fp, metainfoCache, std::begin(results),
                          std::end(results), saveInProgress_, saveError_)) {
    return false;
  }

  {
    // Save active downloads.
    const RequestGroupList& groups = rgman_->getRequestGroups();
    for (const auto& rg : groups) {
      auto dr = rg->createDownloadResult();
      bool stopped = dr->result == error_code::FINISHED ||
                     dr->result == error_code::REMOVED;
      if ((!stopped && saveInProgress_) ||
          (stopped && dr->option->getAsBool(PREF_FORCE_SAVE))) {
        if (!writeDownloadResult(fp, metainfoCache, dr,
                                 rg->isPauseRequested())) {
          return false;
        }
      }
    }
  }
  if (saveWaiting_) {
    const auto& groups = rgman_->getReservedGroups();
    for (const auto& rg : groups) {
      auto result = rg->createDownloadResult();
      if (!writeDownloadResult(fp, metainfoCache, result,
                               rg->isPauseRequested())) {
        return false;
      }
    }
  }
  return true;
}

std::string SessionSerializer::calculateHash() const
{
  SHA1IOFile sha1io;

  auto rv = save(sha1io);

  if (!rv) {
    return "";
  }

  return sha1io.digest();
}

} // namespace aria2
