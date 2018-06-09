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
#include "download_handlers.h"
#include "SimpleRandomizer.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#  include "BtConstants.h"
#  include "ValueBaseBencodeParser.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace {
void unfoldURI(std::vector<std::string>& result,
               const std::vector<std::string>& args)
{
  for (const auto& i : args) {
    paramed_string::expand(std::begin(i), std::end(i),
                           std::back_inserter(result));
  }
}
} // namespace

namespace {
template <typename InputIterator>
void splitURI(std::vector<std::string>& result, InputIterator begin,
              InputIterator end, size_t numSplit, size_t maxIter)
{
  size_t numURIs = std::distance(begin, end);
  if (numURIs >= numSplit) {
    result.insert(std::end(result), begin, end);
  }
  else if (numURIs > 0) {
    size_t num = std::min(numSplit / numURIs, maxIter);
    for (size_t i = 0; i < num; ++i) {
      result.insert(std::end(result), begin, end);
    }
    if (num < maxIter) {
      result.insert(std::end(result), begin, begin + (numSplit % numURIs));
    }
  }
}
} // namespace

namespace {
std::shared_ptr<GroupId> getGID(const std::shared_ptr<Option>& option)
{
  std::shared_ptr<GroupId> gid;
  if (option->defined(PREF_GID)) {
    a2_gid_t n;
    if (GroupId::toNumericId(n, option->get(PREF_GID).c_str()) != 0) {
      throw DL_ABORT_EX(
          fmt("%s is invalid for GID.", option->get(PREF_GID).c_str()));
    }
    gid = GroupId::import(n);
    if (!gid) {
      throw DL_ABORT_EX(
          fmt("GID %s is not unique.", option->get(PREF_GID).c_str()));
    }
  }
  else {
    gid = GroupId::create();
  }
  return gid;
}
} // namespace

namespace {
std::shared_ptr<RequestGroup>
createRequestGroup(const std::shared_ptr<Option>& optionTemplate,
                   const std::vector<std::string>& uris,
                   bool useOutOption = false)
{
  auto option = util::copy(optionTemplate);
  auto rg = std::make_shared<RequestGroup>(getGID(option), option);
  auto dctx = std::make_shared<DownloadContext>(
      option->getAsInt(PREF_PIECE_LENGTH), 0,
      useOutOption && !option->blank(PREF_OUT)
          ? util::applyDir(option->get(PREF_DIR), option->get(PREF_OUT))
          : A2STR::NIL);
  dctx->getFirstFileEntry()->setUris(uris);
  dctx->getFirstFileEntry()->setMaxConnectionPerServer(
      option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER));
  const std::string& checksum = option->get(PREF_CHECKSUM);
  if (!checksum.empty()) {
    auto p = util::divide(std::begin(checksum), std::end(checksum), '=');
    std::string hashType(p.first.first, p.first.second);
    std::string hexDigest(p.second.first, p.second.second);
    util::lowercase(hashType);
    dctx->setDigest(hashType,
                    util::fromHex(std::begin(hexDigest), std::end(hexDigest)));
  }
  rg->setDownloadContext(dctx);

  if (option->getAsBool(PREF_ENABLE_RPC)) {
    rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  }

  removeOneshotOption(option);
  return rg;
}
} // namespace

#if defined(ENABLE_BITTORRENT) || defined(ENABLE_METALINK)
namespace {
std::shared_ptr<MetadataInfo>
createMetadataInfo(const std::shared_ptr<GroupId>& gid, const std::string& uri)
{
  return std::make_shared<MetadataInfo>(gid, uri);
}
} // namespace

namespace {
std::shared_ptr<MetadataInfo> createMetadataInfoDataOnly()
{
  return std::make_shared<MetadataInfo>();
}
} // namespace
#endif // ENABLE_BITTORRENT || ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

namespace {
std::shared_ptr<RequestGroup>
createBtRequestGroup(const std::string& metaInfoUri,
                     const std::shared_ptr<Option>& optionTemplate,
                     const std::vector<std::string>& auxUris,
                     const ValueBase* torrent, bool adjustAnnounceUri = true)
{
  auto option = util::copy(optionTemplate);
  auto gid = getGID(option);
  auto rg = std::make_shared<RequestGroup>(gid, option);
  auto dctx = std::make_shared<DownloadContext>();
  // may throw exception
  bittorrent::loadFromMemory(torrent, dctx, option, auxUris,
                             metaInfoUri.empty() ? "default" : metaInfoUri);
  for (auto& fe : dctx->getFileEntries()) {
    auto& uris = fe->getRemainingUris();
    std::shuffle(std::begin(uris), std::end(uris),
                 *SimpleRandomizer::getInstance());
  }
  if (metaInfoUri.empty()) {
    rg->setMetadataInfo(createMetadataInfoDataOnly());
  }
  else {
    rg->setMetadataInfo(createMetadataInfo(gid, metaInfoUri));
  }
  if (adjustAnnounceUri) {
    bittorrent::adjustAnnounceUri(bittorrent::getTorrentAttrs(dctx), option);
  }
  auto sgl = util::parseIntSegments(option->get(PREF_SELECT_FILE));
  sgl.normalize();
  dctx->setFileFilter(std::move(sgl));
  std::istringstream indexOutIn(option->get(PREF_INDEX_OUT));
  auto indexPaths = util::createIndexPaths(indexOutIn);
  for (const auto& i : indexPaths) {
    dctx->setFilePathWithIndex(i.first,
                               util::applyDir(option->get(PREF_DIR), i.second));
  }
  rg->setDownloadContext(dctx);

  if (option->getAsBool(PREF_ENABLE_RPC)) {
    rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  }

  // Remove "metalink" from Accept Type list to avoid server from
  // responding Metalink file for web-seeding URIs.
  dctx->setAcceptMetalink(false);
  removeOneshotOption(option);
  return rg;
}
} // namespace

namespace {
std::shared_ptr<RequestGroup>
createBtMagnetRequestGroup(const std::string& magnetLink,
                           const std::shared_ptr<Option>& optionTemplate)
{
  auto dctx = std::make_shared<DownloadContext>(METADATA_PIECE_SIZE, 0);

  // We only know info hash. Total Length is unknown at this moment.
  dctx->markTotalLengthIsUnknown();

  bittorrent::loadMagnet(magnetLink, dctx);
  auto torrentAttrs = bittorrent::getTorrentAttrs(dctx);

  if (optionTemplate->getAsBool(PREF_BT_LOAD_SAVED_METADATA)) {
    // Try to read .torrent file saved by aria2 (see
    // UTMetadataPostDownloadHandler and --bt-save-metadata option).
    auto torrentFilename =
        util::applyDir(optionTemplate->get(PREF_DIR),
                       util::toHex(torrentAttrs->infoHash) + ".torrent");

    bittorrent::ValueBaseBencodeParser parser;
    auto torrent = parseFile(parser, torrentFilename);
    if (torrent) {
      auto rg = createBtRequestGroup(torrentFilename, optionTemplate, {},
                                     torrent.get());
      const auto& actualInfoHash =
          bittorrent::getTorrentAttrs(rg->getDownloadContext())->infoHash;

      if (torrentAttrs->infoHash == actualInfoHash) {
        A2_LOG_NOTICE(fmt("BitTorrent metadata was loaded from %s",
                          torrentFilename.c_str()));
        rg->setMetadataInfo(createMetadataInfo(rg->getGroupId(), magnetLink));
        return rg;
      }

      A2_LOG_WARN(
          fmt("BitTorrent metadata loaded from %s has unexpected infohash %s\n",
              torrentFilename.c_str(), util::toHex(actualInfoHash).c_str()));
    }
  }

  auto option = util::copy(optionTemplate);
  bittorrent::adjustAnnounceUri(torrentAttrs, option);
  // torrentAttrs->name may contain "/", but we use basename of
  // FileEntry::getPath() to print out in-memory download entry.
  // Since "/" is treated as separator, we replace it with "-".
  dctx->getFirstFileEntry()->setPath(
      util::replace(torrentAttrs->name, "/", "-"));

  auto gid = getGID(option);
  auto rg = std::make_shared<RequestGroup>(gid, option);
  rg->setFileAllocationEnabled(false);
  rg->setPreLocalFileCheckEnabled(false);
  rg->setDownloadContext(dctx);
  rg->clearPostDownloadHandler();
  rg->addPostDownloadHandler(
      download_handlers::getUTMetadataPostDownloadHandler());
  rg->setDiskWriterFactory(std::make_shared<ByteArrayDiskWriterFactory>());
  rg->setMetadataInfo(createMetadataInfo(gid, magnetLink));
  rg->markInMemoryDownload();

  if (option->getAsBool(PREF_ENABLE_RPC)) {
    rg->setPauseRequested(option->getAsBool(PREF_PAUSE));
  }

  removeOneshotOption(option);
  return rg;
}
} // namespace

void createRequestGroupForBitTorrent(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    const std::string& metaInfoUri, const std::string& torrentData,
    bool adjustAnnounceUri)
{
  std::unique_ptr<ValueBase> torrent;
  bittorrent::ValueBaseBencodeParser parser;
  if (torrentData.empty()) {
    torrent = parseFile(parser, metaInfoUri);
  }
  else {
    ssize_t error;
    torrent = parser.parseFinal(torrentData.c_str(), torrentData.size(), error);
  }
  if (!torrent) {
    throw DL_ABORT_EX2("Bencode decoding failed",
                       error_code::BENCODE_PARSE_ERROR);
  }
  createRequestGroupForBitTorrent(result, option, uris, metaInfoUri,
                                  torrent.get(), adjustAnnounceUri);
}

void createRequestGroupForBitTorrent(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    const std::string& metaInfoUri, const ValueBase* torrent,
    bool adjustAnnounceUri)
{
  std::vector<std::string> nargs;
  if (option->get(PREF_PARAMETERIZED_URI) == A2_V_TRUE) {
    unfoldURI(nargs, uris);
  }
  else {
    nargs = uris;
  }
  // we ignore -Z option here
  size_t numSplit = option->getAsInt(PREF_SPLIT);
  auto rg = createBtRequestGroup(metaInfoUri, option, nargs, torrent,
                                 adjustAnnounceUri);
  rg->setNumConcurrentCommand(numSplit);
  result.push_back(rg);
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void createRequestGroupForMetalink(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::string& metalinkData)
{
  if (metalinkData.empty()) {
    Metalink2RequestGroup().generate(result, option->get(PREF_METALINK_FILE),
                                     option,
                                     option->get(PREF_METALINK_BASE_URI));
  }
  else {
    auto dw = std::make_shared<ByteArrayDiskWriter>();
    dw->setString(metalinkData);
    Metalink2RequestGroup().generate(result, dw, option,
                                     option->get(PREF_METALINK_BASE_URI));
  }
}
#endif // ENABLE_METALINK

namespace {
class AccRequestGroup {
private:
  std::vector<std::shared_ptr<RequestGroup>>& requestGroups_;
  ProtocolDetector detector_;
  std::shared_ptr<Option> option_;
  bool ignoreLocalPath_;
  bool throwOnError_;

public:
  AccRequestGroup(std::vector<std::shared_ptr<RequestGroup>>& requestGroups,
                  std::shared_ptr<Option> option, bool ignoreLocalPath = false,
                  bool throwOnError = false)
      : requestGroups_(requestGroups),
        option_(std::move(option)),
        ignoreLocalPath_(ignoreLocalPath),
        throwOnError_(throwOnError)
  {
  }

  void operator()(const std::string& uri)
  {
    if (detector_.isStreamProtocol(uri)) {
      std::vector<std::string> streamURIs;
      size_t numIter = option_->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
      size_t numSplit = option_->getAsInt(PREF_SPLIT);
      size_t num = std::min(numIter, numSplit);
      for (size_t i = 0; i < num; ++i) {
        streamURIs.push_back(uri);
      }
      auto rg = createRequestGroup(option_, streamURIs);
      rg->setNumConcurrentCommand(numSplit);
      requestGroups_.push_back(rg);
    }
#ifdef ENABLE_BITTORRENT
    else if (detector_.guessTorrentMagnet(uri)) {
      requestGroups_.push_back(createBtMagnetRequestGroup(uri, option_));
    }
    else if (!ignoreLocalPath_ && detector_.guessTorrentFile(uri)) {
      try {
        bittorrent::ValueBaseBencodeParser parser;
        auto torrent = parseFile(parser, uri);
        if (!torrent) {
          throw DL_ABORT_EX2("Bencode decoding failed",
                             error_code::BENCODE_PARSE_ERROR);
        }
        requestGroups_.push_back(
            createBtRequestGroup(uri, option_, {}, torrent.get()));
      }
      catch (RecoverableException& e) {
        if (throwOnError_) {
          throw;
        }
        else {
          // error occurred while parsing torrent file.
          // We simply ignore it.
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
    }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    else if (!ignoreLocalPath_ && detector_.guessMetalinkFile(uri)) {
      try {
        Metalink2RequestGroup().generate(requestGroups_, uri, option_,
                                         option_->get(PREF_METALINK_BASE_URI));
      }
      catch (RecoverableException& e) {
        if (throwOnError_) {
          throw;
        }
        else {
          // error occurred while parsing metalink file.
          // We simply ignore it.
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
    }
#endif // ENABLE_METALINK
    else {
      if (throwOnError_) {
        throw DL_ABORT_EX(fmt(MSG_UNRECOGNIZED_URI, uri.c_str()));
      }
      else {
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
  bool operator()(const std::string& uri)
  {
    return detector_.isStreamProtocol(uri);
  }
};
} // namespace

void createRequestGroupForUri(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option, const std::vector<std::string>& uris,
    bool ignoreForceSequential, bool ignoreLocalPath, bool throwOnError)
{
  std::vector<std::string> nargs;
  if (option->get(PREF_PARAMETERIZED_URI) == A2_V_TRUE) {
    unfoldURI(nargs, uris);
  }
  else {
    nargs = uris;
  }
  if (!ignoreForceSequential &&
      option->get(PREF_FORCE_SEQUENTIAL) == A2_V_TRUE) {
    std::for_each(
        std::begin(nargs), std::end(nargs),
        AccRequestGroup(result, option, ignoreLocalPath, throwOnError));
  }
  else {
    auto strmProtoEnd = std::stable_partition(
        std::begin(nargs), std::end(nargs), StreamProtocolFilter());
    // let's process http/ftp protocols first.
    if (std::begin(nargs) != strmProtoEnd) {
      size_t numIter = option->getAsInt(PREF_MAX_CONNECTION_PER_SERVER);
      size_t numSplit = option->getAsInt(PREF_SPLIT);
      std::vector<std::string> streamURIs;
      splitURI(streamURIs, std::begin(nargs), strmProtoEnd, numSplit, numIter);
      try {
        auto rg = createRequestGroup(option, streamURIs, true);
        rg->setNumConcurrentCommand(numSplit);
        result.push_back(rg);
      }
      catch (RecoverableException& e) {
        if (throwOnError) {
          throw;
        }
        else {
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
      }
    }
    // process remaining URIs(local metalink, BitTorrent files)
    std::for_each(
        strmProtoEnd, std::end(nargs),
        AccRequestGroup(result, option, ignoreLocalPath, throwOnError));
  }
}

bool createRequestGroupFromUriListParser(
    std::vector<std::shared_ptr<RequestGroup>>& result, const Option* option,
    UriListParser* uriListParser)
{
  // Since result already contains some entries, we cache the size of
  // it. Later, we use this value to determine RequestGroup is
  // actually created.
  size_t num = result.size();
  while (uriListParser->hasNext()) {
    std::vector<std::string> uris;
    Option tempOption;
    uriListParser->parseNext(uris, tempOption);
    if (uris.empty()) {
      continue;
    }
    auto requestOption = std::make_shared<Option>(*option);
    requestOption->remove(PREF_OUT);
    const auto& oparser = OptionParser::getInstance();
    for (size_t i = 1, len = option::countOption(); i < len; ++i) {
      auto pref = option::i2p(i);
      auto h = oparser->find(pref);
      if (h && h->getInitialOption() && tempOption.defined(pref)) {
        requestOption->put(pref, tempOption.get(pref));
      }
    }
    // This does not throw exception because throwOnError = false.
    createRequestGroupForUri(result, requestOption, uris);
    if (num < result.size()) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<UriListParser> openUriListParser(const std::string& filename)
{
  std::string listPath;

  auto f = File(filename);
  if (!f.exists() || f.isDir()) {
    throw DL_ABORT_EX(fmt(EX_FILE_OPEN, filename.c_str(),
                          "File not found or it is a directory"));
  }
  listPath = filename;

  return std::make_shared<UriListParser>(listPath);
}

void createRequestGroupForUriList(
    std::vector<std::shared_ptr<RequestGroup>>& result,
    const std::shared_ptr<Option>& option)
{
  auto uriListParser = openUriListParser(option->get(PREF_INPUT_FILE));
  while (createRequestGroupFromUriListParser(result, option.get(),
                                             uriListParser.get()))
    ;
}

std::shared_ptr<MetadataInfo> createMetadataInfoFromFirstFileEntry(
    const std::shared_ptr<GroupId>& gid,
    const std::shared_ptr<DownloadContext>& dctx)
{
  if (dctx->getFileEntries().empty()) {
    return nullptr;
  }
  else {
    auto uris = dctx->getFileEntries()[0]->getUris();
    if (uris.empty()) {
      return nullptr;
    }
    return std::make_shared<MetadataInfo>(gid, uris[0]);
  }
}

void removeOneshotOption(const std::shared_ptr<Option>& option)
{
  option->remove(PREF_PAUSE);
  option->remove(PREF_GID);
}

} // namespace aria2
