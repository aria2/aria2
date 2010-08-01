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
#include "download_helper.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "RequestGroup.h"
#include "Option.h"
#include "prefs.h"
#include "Metalink2RequestGroup.h"
#include "ProtocolDetector.h"
#include "ParameterizedStringParser.h"
#include "PStringBuildVisitor.h"
#include "UriListParser.h"
#include "DownloadContext.h"
#include "RecoverableException.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"
#include "FileEntry.h"
#include "LogFactory.h"
#include "File.h"
#include "util.h"
#include "array_fun.h"
#include "OptionHandler.h"
#include "ByteArrayDiskWriter.h"
#include "a2functional.h"
#include "ByteArrayDiskWriterFactory.h"
#include "MetadataInfo.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "BtConstants.h"
# include "UTMetadataPostDownloadHandler.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

const std::set<std::string>& listRequestOptions()
{
  static const std::string REQUEST_OPTIONS[] = {
    PREF_DIR,
    PREF_CHECK_INTEGRITY,
    PREF_CONTINUE,
    PREF_ALL_PROXY,
    PREF_ALL_PROXY_USER,
    PREF_ALL_PROXY_PASSWD,
    PREF_CONNECT_TIMEOUT,
    PREF_DRY_RUN,
    PREF_LOWEST_SPEED_LIMIT,
    PREF_MAX_FILE_NOT_FOUND,
    PREF_MAX_TRIES,
    PREF_NO_PROXY,
    PREF_OUT,
    PREF_PROXY_METHOD,
    PREF_REMOTE_TIME,
    PREF_SPLIT,
    PREF_TIMEOUT,
    PREF_HTTP_AUTH_CHALLENGE,
    PREF_HTTP_NO_CACHE,
    PREF_HTTP_USER,
    PREF_HTTP_PASSWD,
    PREF_HTTP_PROXY,
    PREF_HTTP_PROXY_USER,
    PREF_HTTP_PROXY_PASSWD,
    PREF_HTTPS_PROXY,
    PREF_HTTPS_PROXY_USER,
    PREF_HTTPS_PROXY_PASSWD,
    PREF_REFERER,
    PREF_ENABLE_HTTP_KEEP_ALIVE,
    PREF_ENABLE_HTTP_PIPELINING,
    PREF_HEADER,
    PREF_USE_HEAD,
    PREF_USER_AGENT,
    PREF_FTP_USER,
    PREF_FTP_PASSWD,
    PREF_FTP_PASV,
    PREF_FTP_PROXY,
    PREF_FTP_PROXY_USER,
    PREF_FTP_PROXY_PASSWD,
    PREF_FTP_TYPE,
    PREF_FTP_REUSE_CONNECTION,
    PREF_NO_NETRC,
    PREF_REUSE_URI,
    PREF_SELECT_FILE,
    PREF_BT_ENABLE_LPD,
    PREF_BT_EXTERNAL_IP,
    PREF_BT_HASH_CHECK_SEED,
    PREF_BT_MAX_OPEN_FILES,
    PREF_BT_MAX_PEERS,
    PREF_BT_METADATA_ONLY,
    PREF_BT_MIN_CRYPTO_LEVEL,
    PREF_BT_PRIORITIZE_PIECE,
    PREF_BT_REQUIRE_CRYPTO,
    PREF_BT_REQUEST_PEER_SPEED_LIMIT,
    PREF_BT_SAVE_METADATA,
    PREF_BT_SEED_UNVERIFIED,
    PREF_BT_STOP_TIMEOUT,
    PREF_BT_TRACKER_INTERVAL,
    PREF_BT_TRACKER_TIMEOUT,
    PREF_BT_TRACKER_CONNECT_TIMEOUT,
    PREF_ENABLE_PEER_EXCHANGE,
    PREF_FOLLOW_TORRENT,
    PREF_INDEX_OUT,
    PREF_MAX_UPLOAD_LIMIT,
    PREF_SEED_RATIO,
    PREF_SEED_TIME,
    PREF_FOLLOW_METALINK,
    PREF_METALINK_SERVERS,
    PREF_METALINK_LANGUAGE,
    PREF_METALINK_LOCATION,
    PREF_METALINK_OS,
    PREF_METALINK_VERSION,
    PREF_METALINK_PREFERRED_PROTOCOL,
    PREF_METALINK_ENABLE_UNIQUE_PROTOCOL,
    PREF_ALLOW_OVERWRITE,
    PREF_ALLOW_PIECE_LENGTH_CHANGE,
    PREF_ASYNC_DNS,
    PREF_AUTO_FILE_RENAMING,
    PREF_FILE_ALLOCATION,
    PREF_MAX_DOWNLOAD_LIMIT,
    PREF_NO_FILE_ALLOCATION_LIMIT,
    PREF_PARAMETERIZED_URI,
    PREF_REALTIME_CHUNK_CHECKSUM,
    PREF_REMOVE_CONTROL_FILE,
    PREF_ALWAYS_RESUME,
    PREF_MAX_RESUME_FAILURE_TRIES,
    PREF_HTTP_ACCEPT_GZIP,
    PREF_MAX_CONNECTION_PER_SERVER,
    PREF_MIN_SPLIT_SIZE,
    PREF_CONDITIONAL_GET,
    PREF_ENABLE_ASYNC_DNS6
  };
  static std::set<std::string> requestOptions
    (vbegin(REQUEST_OPTIONS), vend(REQUEST_OPTIONS));
  return requestOptions;
}

static void unfoldURI
(std::vector<std::string>& result, const std::vector<std::string>& args)
{
  ParameterizedStringParser p;
  PStringBuildVisitor v;
  for(std::vector<std::string>::const_iterator itr = args.begin(),
        eoi = args.end(); itr != eoi; ++itr) {
    v.reset();
    p.parse(*itr)->accept(v);
    result.insert(result.end(), v.getURIs().begin(), v.getURIs().end()); 
  }
}

template<typename InputIterator>
static void splitURI(std::vector<std::string>& result,
                     InputIterator begin,
                     InputIterator end,
                     size_t numSplit,
                     size_t maxIter)
{
  size_t numURIs = std::distance(begin, end);
  if(numURIs >= numSplit) {
    result.insert(result.end(), begin, end);
  } else if(numURIs > 0) {
    size_t num = std::min(numSplit/numURIs, maxIter);
    for(size_t i = 0; i < num; ++i) {
      result.insert(result.end(), begin, end);
    }
    if(num < maxIter) {
      result.insert(result.end(), begin, begin+(numSplit%numURIs));
    }
  }
}

static SharedHandle<RequestGroup> createRequestGroup
(const SharedHandle<Option>& option, const std::vector<std::string>& uris,
 bool useOutOption = false)
{
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx
    (new DownloadContext
     (option->getAsInt(PREF_SEGMENT_SIZE),
      0,
      useOutOption&&!option->blank(PREF_OUT)?
      util::applyDir(option->get(PREF_DIR), option->get(PREF_OUT)):A2STR::NIL));
  dctx->setDir(option->get(PREF_DIR));
  dctx->getFirstFileEntry()->setUris(uris);
  dctx->getFirstFileEntry()->setMaxConnectionPerServer
    (option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER));
  rg->setDownloadContext(dctx);
  return rg;
}

static SharedHandle<MetadataInfo> createMetadataInfo(const std::string& uri)
{
  return SharedHandle<MetadataInfo>(new MetadataInfo(uri));
}

static SharedHandle<MetadataInfo> createMetadataInfoDataOnly()
{
  return SharedHandle<MetadataInfo>(new MetadataInfo());
}

#ifdef ENABLE_BITTORRENT

static
SharedHandle<RequestGroup>
createBtRequestGroup(const std::string& torrentFilePath,
                     const SharedHandle<Option>& option,
                     const std::vector<std::string>& auxUris,
                     const std::string& torrentData = "")
{
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setDir(option->get(PREF_DIR));
  if(torrentData.empty()) {
    bittorrent::load(torrentFilePath, dctx, auxUris);// may throw exception
    rg->setMetadataInfo(createMetadataInfo(torrentFilePath));
  } else {
    bittorrent::loadFromMemory(torrentData, dctx, auxUris, "default"); // may
    // throw
    // exception
    rg->setMetadataInfo(createMetadataInfoDataOnly());
  }
  dctx->setFileFilter(util::parseIntRange(option->get(PREF_SELECT_FILE)));
  std::istringstream indexOutIn(option->get(PREF_INDEX_OUT));
  std::map<size_t, std::string> indexPathMap =
    util::createIndexPathMap(indexOutIn);
  for(std::map<size_t, std::string>::const_iterator i = indexPathMap.begin(),
        eoi = indexPathMap.end(); i != eoi; ++i) {
    dctx->setFilePathWithIndex
      ((*i).first, util::applyDir(dctx->getDir(), (*i).second));
  }
  rg->setDownloadContext(dctx);
  // Remove "metalink" from Accept Type list to avoid server from
  // responding Metalink file for web-seeding URIs.
  util::removeMetalinkContentTypes(rg);
  return rg;
}

static
SharedHandle<RequestGroup>
createBtMagnetRequestGroup(const std::string& magnetLink,
                           const SharedHandle<Option>& option,
                           const std::vector<std::string>& auxUris)
{
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, 0,
                         A2STR::NIL));
  dctx->setDir(A2STR::NIL);
  // We only know info hash. Total Length is unknown at this moment.
  dctx->markTotalLengthIsUnknown();
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  bittorrent::loadMagnet(magnetLink, dctx);
  SharedHandle<TorrentAttribute> torrentAttrs =
    bittorrent::getTorrentAttrs(dctx);
  dctx->getFirstFileEntry()->setPath(torrentAttrs->name);
  rg->setDownloadContext(dctx);
  rg->clearPostDownloadHandler();
  rg->addPostDownloadHandler
    (SharedHandle<UTMetadataPostDownloadHandler>
     (new UTMetadataPostDownloadHandler()));
  rg->setDiskWriterFactory
    (SharedHandle<DiskWriterFactory>(new ByteArrayDiskWriterFactory()));
  rg->setMetadataInfo(createMetadataInfo(magnetLink));
  return rg;
}

void createRequestGroupForBitTorrent
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::vector<std::string>& uris,
 const std::string& torrentData)
{
  std::vector<std::string> nargs;
  if(option->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  // we ignore -Z option here
  size_t numSplit = option->getAsInt(PREF_SPLIT);
  SharedHandle<RequestGroup> rg =
    createBtRequestGroup(option->get(PREF_TORRENT_FILE), option, nargs,
                         torrentData);
  rg->setNumConcurrentCommand(numSplit);
  result.push_back(rg);
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void createRequestGroupForMetalink
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::string& metalinkData)
{
  if(metalinkData.empty()) {
    Metalink2RequestGroup().generate(result,
                                     option->get(PREF_METALINK_FILE),
                                     option);
  } else {
    SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
    dw->setString(metalinkData);
    Metalink2RequestGroup().generate(result, dw, option);
  }
}
#endif // ENABLE_METALINK

class AccRequestGroup {
private:
  std::vector<SharedHandle<RequestGroup> >& requestGroups_;
  ProtocolDetector detector_;
  SharedHandle<Option> option_;
  bool ignoreLocalPath_;
public:
  AccRequestGroup(std::vector<SharedHandle<RequestGroup> >& requestGroups,
                  const SharedHandle<Option>& option,
                  bool ignoreLocalPath = false):
    requestGroups_(requestGroups), option_(option),
    ignoreLocalPath_(ignoreLocalPath) {}

  void
  operator()(const std::string& uri)
  {
    if(detector_.isStreamProtocol(uri)) {
      std::vector<std::string> streamURIs;
      size_t numIter = option_->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
      size_t numSplit = option_->getAsInt(PREF_SPLIT);
      size_t num = std::min(numIter, numSplit);
      for(size_t i = 0; i < num; ++i) {
        streamURIs.push_back(uri);
      }
      SharedHandle<RequestGroup> rg = createRequestGroup(option_, streamURIs);
      rg->setNumConcurrentCommand(numSplit);
      requestGroups_.push_back(rg);
    }
#ifdef ENABLE_BITTORRENT
    else if(detector_.guessTorrentMagnet(uri)) {
      try {
        SharedHandle<RequestGroup> group =
          createBtMagnetRequestGroup(uri, option_, std::vector<std::string>());
        requestGroups_.push_back(group);
      } catch(RecoverableException& e) {
        // error occurred while parsing torrent file.
        // We simply ignore it. 
        LogFactory::getInstance()->error(EX_EXCEPTION_CAUGHT, e);
      }
    } else if(!ignoreLocalPath_ && detector_.guessTorrentFile(uri)) {
      try {
        requestGroups_.push_back
          (createBtRequestGroup(uri, option_, std::vector<std::string>()));
      } catch(RecoverableException& e) {
        // error occurred while parsing torrent file.
        // We simply ignore it. 
        LogFactory::getInstance()->error(EX_EXCEPTION_CAUGHT, e);
      }
    } 
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    else if(!ignoreLocalPath_ && detector_.guessMetalinkFile(uri)) {
      try {
        Metalink2RequestGroup().generate(requestGroups_, uri, option_);
      } catch(RecoverableException& e) {
        // error occurred while parsing metalink file.
        // We simply ignore it.
        LogFactory::getInstance()->error(EX_EXCEPTION_CAUGHT, e);
      }
    }
#endif // ENABLE_METALINK
    else {
      LogFactory::getInstance()->error(MSG_UNRECOGNIZED_URI, (uri).c_str());
    }
  }
};

class StreamProtocolFilter {
private:
  ProtocolDetector detector_;
public:
  bool operator()(const std::string& uri) {
    return detector_.isStreamProtocol(uri);
  }
};

void createRequestGroupForUri
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::vector<std::string>& uris,
 bool ignoreForceSequential,
 bool ignoreLocalPath)
{
  std::vector<std::string> nargs;
  if(option->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  if(!ignoreForceSequential && option->get(PREF_FORCE_SEQUENTIAL) == V_TRUE) {
    std::for_each(nargs.begin(), nargs.end(),
                  AccRequestGroup(result, option, ignoreLocalPath));
  } else {
    std::vector<std::string>::iterator strmProtoEnd =
      std::stable_partition(nargs.begin(), nargs.end(), StreamProtocolFilter());
    // let's process http/ftp protocols first.
    if(nargs.begin() != strmProtoEnd) {
      size_t numIter = option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
      size_t numSplit = option->getAsInt(PREF_SPLIT);
      std::vector<std::string> streamURIs;
      splitURI(streamURIs, nargs.begin(), strmProtoEnd, numSplit, numIter);
      SharedHandle<RequestGroup> rg =
        createRequestGroup(option, streamURIs, true);
      rg->setNumConcurrentCommand(numSplit);
      result.push_back(rg);
    }
    // process remaining URIs(local metalink, BitTorrent files)
    std::for_each(strmProtoEnd, nargs.end(),
                  AccRequestGroup(result, option, ignoreLocalPath));
  }
}

static void createRequestGroupForUriList
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 std::istream& in)
{
  UriListParser p(in);
  while(p.hasNext()) {
    std::vector<std::string> uris;
    SharedHandle<Option> tempOption(new Option());
    p.parseNext(uris, *tempOption.get());
    if(uris.empty()) {
      continue;
    }

    SharedHandle<Option> requestOption(new Option(*option.get()));
    for(std::set<std::string>::const_iterator i =
          listRequestOptions().begin(), eoi = listRequestOptions().end();
        i != eoi; ++i) {
      if(tempOption->defined(*i)) {
        requestOption->put(*i, tempOption->get(*i));
      }
    }

    createRequestGroupForUri(result, requestOption, uris);
  }
}

void createRequestGroupForUriList
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option)
{
  if(option->get(PREF_INPUT_FILE) == "-") {
    createRequestGroupForUriList(result, option, std::cin);
  } else {
    if(!File(option->get(PREF_INPUT_FILE)).isFile()) {
      throw DL_ABORT_EX
        (StringFormat(EX_FILE_OPEN, option->get(PREF_INPUT_FILE).c_str(),
                      "No such file").str());
    }
    std::ifstream f(option->get(PREF_INPUT_FILE).c_str(), std::ios::binary);
    createRequestGroupForUriList(result, option, f);
  }
}

SharedHandle<MetadataInfo>
createMetadataInfoFromFirstFileEntry(const SharedHandle<DownloadContext>& dctx)
{
  if(dctx->getFileEntries().empty()) {
    return SharedHandle<MetadataInfo>();
  } else {
    std::vector<std::string> uris;
    dctx->getFileEntries()[0]->getUris(uris);
    if(uris.empty()) {
      return SharedHandle<MetadataInfo>();
    }
    return SharedHandle<MetadataInfo>(new MetadataInfo(uris[0]));
  }
}

} // namespace aria2
