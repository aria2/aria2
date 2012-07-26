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

#include <algorithm>
#include <sstream>

#include "RequestGroup.h"
#include "Option.h"
#include "prefs.h"
#include "Metalink2RequestGroup.h"
#include "ProtocolDetector.h"
#include "paramed_string.h"
#include "UriListParser.h"
#include "DownloadContext.h"
#include "RecoverableException.h"
#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"
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
#include "OptionParser.h"
#include "SegList.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#  include "BtConstants.h"
#  include "UTMetadataPostDownloadHandler.h"
#  include "ValueBaseBencodeParser.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace {
void unfoldURI
(std::vector<std::string>& result, const std::vector<std::string>& args)
{
  for(std::vector<std::string>::const_iterator i = args.begin(),
        eoi = args.end(); i != eoi; ++i) {
    paramed_string::expand((*i).begin(), (*i).end(),
                           std::back_inserter(result));
  }
}
} // namespace

namespace {
template<typename InputIterator>
void splitURI(std::vector<std::string>& result,
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
} // namespace

namespace {
SharedHandle<RequestGroup> createRequestGroup
(const SharedHandle<Option>& optionTemplate,
 const std::vector<std::string>& uris,
 bool useOutOption = false)
{
  SharedHandle<Option> option = util::copy(optionTemplate);
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx
    (new DownloadContext
     (option->getAsInt(PREF_PIECE_LENGTH),
      0,
      useOutOption&&!option->blank(PREF_OUT)?
      util::applyDir(option->get(PREF_DIR), option->get(PREF_OUT)):A2STR::NIL));
  dctx->getFirstFileEntry()->setUris(uris);
  dctx->getFirstFileEntry()->setMaxConnectionPerServer
    (option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER));
#ifdef ENABLE_MESSAGE_DIGEST
  const std::string& checksum = option->get(PREF_CHECKSUM);
  if(!checksum.empty()) {
    std::pair<Scip, Scip> p;
    util::divide(p, checksum.begin(), checksum.end(), '=');
    std::string hashType(p.first.first, p.first.second);
    std::string hexDigest(p.second.first, p.second.second);
    util::lowercase(hashType);
    dctx->setDigest(hashType,
                    util::fromHex(hexDigest.begin(), hexDigest.end()));
  }
#endif // ENABLE_MESSAGE_DIGEST
  rg->setDownloadContext(dctx);
  rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  removeOneshotOption(option);
  return rg;
}
} // namespace

#if defined ENABLE_BITTORRENT || ENABLE_METALINK
namespace {
SharedHandle<MetadataInfo> createMetadataInfo(const std::string& uri)
{
  return SharedHandle<MetadataInfo>(new MetadataInfo(uri));
}
} // namespace

namespace {
SharedHandle<MetadataInfo> createMetadataInfoDataOnly()
{
  return SharedHandle<MetadataInfo>(new MetadataInfo());
}
} // namespace
#endif // ENABLE_BITTORRENT || ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

namespace {
SharedHandle<RequestGroup>
createBtRequestGroup(const std::string& metaInfoUri,
                     const SharedHandle<Option>& optionTemplate,
                     const std::vector<std::string>& auxUris,
                     const SharedHandle<ValueBase>& torrent,
                     bool adjustAnnounceUri = true)
{
  SharedHandle<Option> option = util::copy(optionTemplate);
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  // may throw exception
  bittorrent::loadFromMemory(torrent, dctx, option, auxUris,
                             metaInfoUri.empty() ? "default" : metaInfoUri);
  if(metaInfoUri.empty()) {
    rg->setMetadataInfo(createMetadataInfoDataOnly());
  } else {
    rg->setMetadataInfo(createMetadataInfo(metaInfoUri));
  }
  if(adjustAnnounceUri) {
    bittorrent::adjustAnnounceUri(bittorrent::getTorrentAttrs(dctx), option);
  }
  SegList<int> sgl;
  util::parseIntSegments(sgl, option->get(PREF_SELECT_FILE));
  sgl.normalize();
  dctx->setFileFilter(sgl);
  std::istringstream indexOutIn(option->get(PREF_INDEX_OUT));
  std::vector<std::pair<size_t, std::string> > indexPaths =
    util::createIndexPaths(indexOutIn);
  for(std::vector<std::pair<size_t, std::string> >::const_iterator i =
        indexPaths.begin(), eoi = indexPaths.end(); i != eoi; ++i) {
    dctx->setFilePathWithIndex
      ((*i).first, util::applyDir(option->get(PREF_DIR), (*i).second));
  }
  rg->setDownloadContext(dctx);
  rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  // Remove "metalink" from Accept Type list to avoid server from
  // responding Metalink file for web-seeding URIs.
  util::removeMetalinkContentTypes(rg);
  removeOneshotOption(option);
  return rg;
}
} // namespace

namespace {
SharedHandle<RequestGroup>
createBtMagnetRequestGroup
(const std::string& magnetLink,
 const SharedHandle<Option>& optionTemplate)
{
  SharedHandle<Option> option = util::copy(optionTemplate);
  SharedHandle<RequestGroup> rg(new RequestGroup(option));
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, 0,
                         A2STR::NIL));
  // We only know info hash. Total Length is unknown at this moment.
  dctx->markTotalLengthIsUnknown();
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  bittorrent::loadMagnet(magnetLink, dctx);
  SharedHandle<TorrentAttribute> torrentAttrs =
    bittorrent::getTorrentAttrs(dctx);
  bittorrent::adjustAnnounceUri(torrentAttrs, option);
  dctx->getFirstFileEntry()->setPath(torrentAttrs->name);
  rg->setDownloadContext(dctx);
  rg->clearPostDownloadHandler();
  SharedHandle<UTMetadataPostDownloadHandler> utMetadataPostHandler
    (new UTMetadataPostDownloadHandler());
  rg->addPostDownloadHandler(utMetadataPostHandler);
  rg->setDiskWriterFactory
    (SharedHandle<DiskWriterFactory>(new ByteArrayDiskWriterFactory()));
  rg->setMetadataInfo(createMetadataInfo(magnetLink));
  rg->markInMemoryDownload();
  rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  removeOneshotOption(option);
  return rg;
}
} // namespace

void createRequestGroupForBitTorrent
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::vector<std::string>& uris,
 const std::string& metaInfoUri,
 const std::string& torrentData,
 bool adjustAnnounceUri)
{
  SharedHandle<ValueBase> torrent;
  bittorrent::ValueBaseBencodeParser parser;
  if(torrentData.empty()) {
    torrent = parseFile(parser, metaInfoUri);
  } else {
    ssize_t error;
    torrent = parser.parseFinal(torrentData.c_str(), torrentData.size(),
                                error);
  }
  if(!torrent) {
    throw DL_ABORT_EX2("Bencode decoding failed",
                       error_code::BENCODE_PARSE_ERROR);
  }
  createRequestGroupForBitTorrent(result, option, uris, metaInfoUri, torrent);
}

void createRequestGroupForBitTorrent
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::vector<std::string>& uris,
 const std::string& metaInfoUri,
 const SharedHandle<ValueBase>& torrent,
 bool adjustAnnounceUri)
{
  std::vector<std::string> nargs;
  if(option->get(PREF_PARAMETERIZED_URI) == A2_V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  // we ignore -Z option here
  size_t numSplit = option->getAsInt(PREF_SPLIT);
  SharedHandle<RequestGroup> rg =
    createBtRequestGroup(metaInfoUri, option, nargs,
                         torrent, adjustAnnounceUri);
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
                                     option,
                                     option->get(PREF_METALINK_BASE_URI));
  } else {
    SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
    dw->setString(metalinkData);
    Metalink2RequestGroup().generate(result, dw, option,
                                     option->get(PREF_METALINK_BASE_URI));
  }
}
#endif // ENABLE_METALINK

namespace {
class AccRequestGroup {
private:
  std::vector<SharedHandle<RequestGroup> >& requestGroups_;
  ProtocolDetector detector_;
  SharedHandle<Option> option_;
  bool ignoreLocalPath_;
  bool throwOnError_;
public:
  AccRequestGroup(std::vector<SharedHandle<RequestGroup> >& requestGroups,
                  const SharedHandle<Option>& option,
                  bool ignoreLocalPath = false,
                  bool throwOnError = false):
    requestGroups_(requestGroups), option_(option),
    ignoreLocalPath_(ignoreLocalPath),
    throwOnError_(throwOnError)
  {}

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
      SharedHandle<RequestGroup> group =
        createBtMagnetRequestGroup(uri, option_);
      requestGroups_.push_back(group);
    } else if(!ignoreLocalPath_ && detector_.guessTorrentFile(uri)) {
      try {
        bittorrent::ValueBaseBencodeParser parser;
        SharedHandle<ValueBase> torrent = parseFile(parser, uri);
        if(!torrent) {
          throw DL_ABORT_EX2("Bencode decoding failed",
                             error_code::BENCODE_PARSE_ERROR);
        }
        requestGroups_.push_back
          (createBtRequestGroup(uri, option_, std::vector<std::string>(),
                                torrent));
      } catch(RecoverableException& e) {
        if(throwOnError_) {
          throw;
        } else {
          // error occurred while parsing torrent file.
          // We simply ignore it.
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
    }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    else if(!ignoreLocalPath_ && detector_.guessMetalinkFile(uri)) {
      try {
        Metalink2RequestGroup().generate(requestGroups_, uri, option_,
                                         option_->get(PREF_METALINK_BASE_URI));
      } catch(RecoverableException& e) {
        if(throwOnError_) {
          throw;
        } else {
          // error occurred while parsing metalink file.
          // We simply ignore it.
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
    }
#endif // ENABLE_METALINK
    else {
      if(throwOnError_) {
        throw DL_ABORT_EX(fmt(MSG_UNRECOGNIZED_URI, uri.c_str()));
      } else {
        A2_LOG_ERROR(fmt(MSG_UNRECOGNIZED_URI, uri.c_str()));
      }
    }
  }
};
} // namespace

namespace {
class StreamProtocolFilter {
private:
  ProtocolDetector detector_;
public:
  bool operator()(const std::string& uri) {
    return detector_.isStreamProtocol(uri);
  }
};
} // namespace

void createRequestGroupForUri
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option,
 const std::vector<std::string>& uris,
 bool ignoreForceSequential,
 bool ignoreLocalPath,
 bool throwOnError)
{
  std::vector<std::string> nargs;
  if(option->get(PREF_PARAMETERIZED_URI) == A2_V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  if(!ignoreForceSequential && option->get(PREF_FORCE_SEQUENTIAL) == A2_V_TRUE) {
    std::for_each(nargs.begin(), nargs.end(),
                  AccRequestGroup(result, option, ignoreLocalPath,
                                  throwOnError));
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
                  AccRequestGroup(result, option, ignoreLocalPath,
                                  throwOnError));
  }
}

bool createRequestGroupFromUriListParser
(std::vector<SharedHandle<RequestGroup> >& result,
 const Option* option,
 UriListParser* uriListParser)
{
  // Since result already contains some entries, we cache the size of
  // it. Later, we use this value to determine RequestGroup is
  // actually created.
  size_t num = result.size();
  while(uriListParser->hasNext()) {
    std::vector<std::string> uris;
    Option tempOption;
    uriListParser->parseNext(uris, tempOption);
    if(uris.empty()) {
      continue;
    }
    SharedHandle<Option> requestOption(new Option(*option));
    requestOption->remove(PREF_OUT);
    const SharedHandle<OptionParser>& oparser = OptionParser::getInstance();
    for(size_t i = 1, len = option::countOption(); i < len; ++i) {
      const Pref* pref = option::i2p(i);
      const SharedHandle<OptionHandler>& h = oparser->find(pref);
      if(h && h->getInitialOption() && tempOption.defined(pref)) {
        requestOption->put(pref, tempOption.get(pref));
      }
    }
    // This does not throw exception because throwOnError = false.
    createRequestGroupForUri(result, requestOption, uris);
    if(num < result.size()) {
      return true;
    }
  }
  return false;
}

SharedHandle<UriListParser> openUriListParser(const std::string& filename)
{
  std::string listPath;
  if(filename == "-") {
    listPath = DEV_STDIN;
  } else {
    if(!File(filename).isFile()) {
      throw DL_ABORT_EX
        (fmt(EX_FILE_OPEN, filename.c_str(), "No such file"));
    }
    listPath = filename;
  }
  return SharedHandle<UriListParser>(new UriListParser(listPath));
}

void createRequestGroupForUriList
(std::vector<SharedHandle<RequestGroup> >& result,
 const SharedHandle<Option>& option)
{
  SharedHandle<UriListParser> uriListParser = openUriListParser
    (option->get(PREF_INPUT_FILE));
  while(createRequestGroupFromUriListParser(result, option.get(),
                                            uriListParser.get()));
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

void removeOneshotOption(const SharedHandle<Option>& option)
{
  option->remove(PREF_PAUSE);
}

} // namespace aria2
