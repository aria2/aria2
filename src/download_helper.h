/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
#ifndef D_DOWNLOAD_HELPER_H
#define D_DOWNLOAD_HELPER_H

#include "common.h"

#include <string>
#include <vector>
#include <set>
#include <memory>

namespace aria2 {

class RequestGroup;
class Option;
class MetadataInfo;
class DownloadContext;
class UriListParser;
class ValueBase;
class GroupId;

#ifdef ENABLE_BITTORRENT
// Create RequestGroup object using torrent file specified by
// metaInfoUri, which is treated as local file path. If non-empty
// torrentData is specified, then it is used as a content of torrent
// file instead. If adjustAnnounceUri is true, announce URIs are
// adjusted using bittorrent::adjustAnnounceUri().  In this function,
// force-sequential is ignored.
void createRequestGroupForBitTorrent(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    const std::string& metaInfoUri, const std::string& torrentData = "",
    bool adjustAnnounceUri = true);

// Create RequestGroup object using already decoded torrent metainfo
// structure.  If adjustAnnounceUri is true, announce URIs are
// adjusted using bittorrent::adjustAnnounceUri().  In this function,
// force-sequential is ignored.
void createRequestGroupForBitTorrent(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    const std::string& metaInfoUri, const ValueBase* torrent,
    bool adjustAnnounceUri = true);

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
// Create RequestGroup objects using Metalink file specified by
// metalink-file option. If non-empty metalinkData is specified, it is
// used as a content of metalink file instead.
void createRequestGroupForMetalink(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option,
    const std::string& metalinkData = "");
#endif // ENABLE_METALINK

// Reads one entry from uriListParser and creates RequestGroups from
// it and store them in result. If the bad entry is found, this
// function just skips it and reads next entry. If at least one
// RequestGroup is created successfully, this function returns true
// and created RequestGroups are stored in result. If no RequestGroup
// is created and uriListParser reads all input, this function returns
// false. The option is used as a option template.
bool createRequestGroupFromUriListParser(
    std::vector<std::shared_ptr<RequestGroup>>& result, const Option* option,
    UriListParser* uriListParser);

// Creates UriListParser using given filename.  If filename is "-",
// then UriListParser is configured to read from standard input.
// Otherwise, this function first checks file denoted by filename
// exists.  If it does not exist, this function throws exception.
// This function returns std::shared_ptr<UriListParser> object if it
// succeeds.
std::shared_ptr<UriListParser> openUriListParser(const std::string& filename);

// Create RequestGroup objects from reading file specified by input-file option.
// If the value of input-file option is "-", stdin is used as a input source.
// Each line is treated as if it is provided in command-line argument.
// The additional out and dir options can be specified after each line of URIs.
// This optional line must start with white space(s).
void createRequestGroupForUriList(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option);

// Create RequestGroup object using provided uris.  If ignoreLocalPath
// is true, a path to torrent file and metalink file are ignored.  If
// throwOnError is true, exception will be thrown when Metalink
// Document or .torrent file cannot be parsed or unrecognized URI is
// given. If throwOnError is false, these errors are just logged as
// error.
void createRequestGroupForUri(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    bool ignoreForceSequential = false, bool ignoreLocalPath = false,
    bool throwOnError = false);

template <typename InputIterator>
void setMetadataInfo(InputIterator first, InputIterator last,
                     const std::shared_ptr<MetadataInfo>& mi)
{
  for (; first != last; ++first) {
    (*first)->setMetadataInfo(mi);
  }
}

std::shared_ptr<MetadataInfo> createMetadataInfoFromFirstFileEntry(
    const std::shared_ptr<GroupId>& gid,
    const std::shared_ptr<DownloadContext>& dctx);

// Removes option value which is only effective at the first
// construction time.
void removeOneshotOption(const std::shared_ptr<Option>& option);

} // namespace aria2

#endif // D_DOWNLOAD_HELPER_H
