/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "aria2api.h"

#include <functional>

#include "Platform.h"
#include "Context.h"
#include "DownloadEngine.h"
#include "OptionParser.h"
#include "Option.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "OptionHandler.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "MultiUrlRequestInfo.h"
#include "prefs.h"
#include "download_helper.h"
#include "LogFactory.h"
#include "PieceStorage.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "BitfieldMan.h"
#include "DownloadContext.h"
#include "RpcMethodImpl.h"
#include "console.h"
#include "KeepRunningCommand.h"
#include "A2STR.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#include "ApiCallbackDownloadEventListener.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

Session::Session(const KeyVals& options)
    : context(std::make_shared<Context>(false, 0, nullptr, options))
{
}

Session::~Session() = default;

SessionConfig::SessionConfig()
    : keepRunning(false),
      useSignalHandler(true),
      downloadEventCallback(nullptr),
      userData(nullptr)
{
}

namespace {
Platform* platform = nullptr;
} // namespace

int libraryInit()
{
  global::initConsole(true);
  try {
    platform = new Platform();
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  LogFactory::setConsoleOutput(false);
  return 0;
}

int libraryDeinit()
{
  delete platform;
  return 0;
}

Session* sessionNew(const KeyVals& options, const SessionConfig& config)
{
  int rv;
  std::unique_ptr<Session> session;
  try {
    session = make_unique<Session>(options);
  }
  catch (RecoverableException& e) {
    return nullptr;
  }
  if (session->context->reqinfo) {
    if (!config.useSignalHandler) {
      session->context->reqinfo->setUseSignalHandler(false);
    }
    rv = session->context->reqinfo->prepare();
    if (rv != 0) {
      return nullptr;
    }
    auto& e = session->context->reqinfo->getDownloadEngine();
    if (config.keepRunning) {
      e->getRequestGroupMan()->setKeepRunning(true);
      // Add command to make aria2 keep event polling
      e->addCommand(make_unique<KeepRunningCommand>(e->newCUID(), e.get()));
    }
    if (config.downloadEventCallback) {
      session->listener = make_unique<ApiCallbackDownloadEventListener>(
          session.get(), config.downloadEventCallback, config.userData);
      SingletonHolder<Notifier>::instance()->addDownloadEventListener(
          session->listener.get());
    }
  }
  else {
    return nullptr;
  }
  return session.release();
}

int sessionFinal(Session* session)
{
  error_code::Value rv = session->context->reqinfo->getResult();
  delete session;
  return rv;
}

int run(Session* session, RUN_MODE mode)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  return e->run(mode == RUN_ONCE);
}

int shutdown(Session* session, bool force)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  if (force) {
    e->requestForceHalt();
  }
  else {
    e->requestHalt();
  }
  // Skip next polling timeout. This avoids 1 second delay when there
  // is no Command other than KeepRunningCommand in the queue.
  e->setNoWait(true);
  return 0;
}

std::string gidToHex(A2Gid gid) { return GroupId::toHex(gid); }

A2Gid hexToGid(const std::string& hex)
{
  A2Gid gid;
  if (GroupId::toNumericId(gid, hex.c_str()) == 0) {
    return gid;
  }
  else {
    return 0;
  }
}

bool isNull(A2Gid gid) { return gid == 0; }

namespace {
template <typename InputIterator, typename Pred>
void apiGatherOption(InputIterator first, InputIterator last, Pred pred,
                     Option* option,
                     const std::shared_ptr<OptionParser>& optionParser)
{
  for (; first != last; ++first) {
    const std::string& optionName = (*first).first;
    PrefPtr pref = option::k2p(optionName);
    const OptionHandler* handler = optionParser->find(pref);
    if (!handler || !pred(handler)) {
      // Just ignore the unacceptable options in this context.
      continue;
    }
    handler->parse(*option, (*first).second);
  }
}
} // namespace

namespace {
void apiGatherRequestOption(Option* option, const KeyVals& options,
                            const std::shared_ptr<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fn(&OptionHandler::getInitialOption), option,
                  optionParser);
}
} // namespace

namespace {
void apiGatherChangeableOption(
    Option* option, const KeyVals& options,
    const std::shared_ptr<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fn(&OptionHandler::getChangeOption), option,
                  optionParser);
}
} // namespace

namespace {
void apiGatherChangeableOptionForReserved(
    Option* option, const KeyVals& options,
    const std::shared_ptr<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fn(&OptionHandler::getChangeOptionForReserved),
                  option, optionParser);
}
} // namespace

namespace {
void apiGatherChangeableGlobalOption(
    Option* option, const KeyVals& options,
    const std::shared_ptr<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fn(&OptionHandler::getChangeGlobalOption), option,
                  optionParser);
}
} // namespace

namespace {
void addRequestGroup(const std::shared_ptr<RequestGroup>& group,
                     DownloadEngine* e, int position)
{
  if (position >= 0) {
    e->getRequestGroupMan()->insertReservedGroup(position, group);
  }
  else {
    e->getRequestGroupMan()->addReservedGroup(group);
  }
}
} // namespace

int addUri(Session* session, A2Gid* gid, const std::vector<std::string>& uris,
           const KeyVals& options, int position)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  auto requestOption = std::make_shared<Option>(*e->getOption());
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  std::vector<std::shared_ptr<RequestGroup>> result;
  createRequestGroupForUri(result, requestOption, uris,
                           /* ignoreForceSeq = */ true,
                           /* ignoreLocalPath = */ true);
  if (!result.empty()) {
    addRequestGroup(result.front(), e.get(), position);
    if (gid) {
      *gid = result.front()->getGID();
    }
  }
  return 0;
}

int addMetalink(Session* session, std::vector<A2Gid>* gids,
                const std::string& metalinkFile, const KeyVals& options,
                int position)
{
#ifdef ENABLE_METALINK
  auto& e = session->context->reqinfo->getDownloadEngine();
  auto requestOption = std::make_shared<Option>(*e->getOption());
  std::vector<std::shared_ptr<RequestGroup>> result;
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
    requestOption->put(PREF_METALINK_FILE, metalinkFile);
    createRequestGroupForMetalink(result, requestOption);
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  if (!result.empty()) {
    if (position >= 0) {
      e->getRequestGroupMan()->insertReservedGroup(position, result);
    }
    else {
      e->getRequestGroupMan()->addReservedGroup(result);
    }
    if (gids) {
      for (std::vector<std::shared_ptr<RequestGroup>>::const_iterator
               i = result.begin(),
               eoi = result.end();
           i != eoi; ++i) {
        (*gids).push_back((*i)->getGID());
      }
    }
  }
  return 0;
#else  // !ENABLE_METALINK
  return -1;
#endif // !ENABLE_METALINK
}

int addTorrent(Session* session, A2Gid* gid, const std::string& torrentFile,
               const std::vector<std::string>& webSeedUris,
               const KeyVals& options, int position)
{
#ifdef ENABLE_BITTORRENT
  auto& e = session->context->reqinfo->getDownloadEngine();
  auto requestOption = std::make_shared<Option>(*e->getOption());
  std::vector<std::shared_ptr<RequestGroup>> result;
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
    requestOption->put(PREF_TORRENT_FILE, torrentFile);
    createRequestGroupForBitTorrent(result, requestOption, webSeedUris,
                                    torrentFile);
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  if (!result.empty()) {
    addRequestGroup(result.front(), e.get(), position);
    if (gid) {
      *gid = result.front()->getGID();
    }
  }
  return 0;
#else  // !ENABLE_BITTORRENT
  return -1;
#endif // !ENABLE_BITTORRENT
}

int addTorrent(Session* session, A2Gid* gid, const std::string& torrentFile,
               const KeyVals& options, int position)
{
  return addTorrent(session, gid, torrentFile, std::vector<std::string>(),
                    options, position);
}

int removeDownload(Session* session, A2Gid gid, bool force)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  std::shared_ptr<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    if (group->getState() == RequestGroup::STATE_ACTIVE) {
      if (force) {
        group->setForceHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      else {
        group->setHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      e->setRefreshInterval(std::chrono::milliseconds(0));
    }
    else {
      if (group->isDependencyResolved()) {
        e->getRequestGroupMan()->removeReservedGroup(gid);
      }
      else {
        return -1;
      }
    }
  }
  else {
    return -1;
  }
  return 0;
}

int pauseDownload(Session* session, A2Gid gid, bool force)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  std::shared_ptr<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    bool reserved = group->getState() == RequestGroup::STATE_WAITING;
    if (pauseRequestGroup(group, reserved, force)) {
      e->setRefreshInterval(std::chrono::milliseconds(0));
      return 0;
    }
  }
  return -1;
}

int unpauseDownload(Session* session, A2Gid gid)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  std::shared_ptr<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if (!group || group->getState() != RequestGroup::STATE_WAITING ||
      !group->isPauseRequested()) {
    return -1;
  }
  else {
    group->setPauseRequested(false);
    e->getRequestGroupMan()->requestQueueCheck();
  }
  return 0;
}

int changePosition(Session* session, A2Gid gid, int pos, OffsetMode how)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  try {
    return e->getRequestGroupMan()->changeReservedGroupPosition(gid, pos, how);
  }
  catch (RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
}

int changeOption(Session* session, A2Gid gid, const KeyVals& options)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  std::shared_ptr<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if (group) {
    Option option;
    try {
      if (group->getState() == RequestGroup::STATE_ACTIVE) {
        apiGatherChangeableOption(&option, options,
                                  OptionParser::getInstance());
      }
      else {
        apiGatherChangeableOptionForReserved(&option, options,
                                             OptionParser::getInstance());
      }
    }
    catch (RecoverableException& err) {
      A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, err);
      return -1;
    }
    changeOption(group, option, e.get());
    return 0;
  }
  else {
    return -1;
  }
}

const std::string& getGlobalOption(Session* session, const std::string& name)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  PrefPtr pref = option::k2p(name);
  if (OptionParser::getInstance()->find(pref)) {
    return e->getOption()->get(pref);
  }
  else {
    return A2STR::NIL;
  }
}

KeyVals getGlobalOptions(Session* session)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  const std::shared_ptr<OptionParser>& optionParser =
      OptionParser::getInstance();
  const Option* option = e->getOption();
  KeyVals options;
  for (size_t i = 1, len = option::countOption(); i < len; ++i) {
    PrefPtr pref = option::i2p(i);
    if (option->defined(pref) && optionParser->find(pref)) {
      options.push_back(KeyVals::value_type(pref->k, option->get(pref)));
    }
  }
  return options;
}

int changeGlobalOption(Session* session, const KeyVals& options)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  Option option;
  try {
    apiGatherChangeableGlobalOption(&option, options,
                                    OptionParser::getInstance());
  }
  catch (RecoverableException& err) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, err);
    return -1;
  }
  changeGlobalOption(option, e.get());
  return 0;
}

GlobalStat getGlobalStat(Session* session)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  auto& rgman = e->getRequestGroupMan();
  TransferStat ts = rgman->calculateStat();
  GlobalStat res;
  res.downloadSpeed = ts.downloadSpeed;
  res.uploadSpeed = ts.uploadSpeed;
  res.numActive = rgman->getRequestGroups().size();
  res.numWaiting = rgman->getReservedGroups().size();
  res.numStopped = rgman->getDownloadResults().size();
  return res;
}

std::vector<A2Gid> getActiveDownload(Session* session)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  const RequestGroupList& groups = e->getRequestGroupMan()->getRequestGroups();
  std::vector<A2Gid> res;
  for (const auto& group : groups) {
    res.push_back(group->getGID());
  }
  return res;
}

namespace {
template <typename OutputIterator, typename InputIterator>
void createUriEntry(OutputIterator out, InputIterator first, InputIterator last,
                    UriStatus status)
{
  for (; first != last; ++first) {
    UriData uriData;
    uriData.uri = *first;
    uriData.status = status;
    out++ = uriData;
  }
}
} // namespace

namespace {
template <typename OutputIterator>
void createUriEntry(OutputIterator out, const std::shared_ptr<FileEntry>& file)
{
  createUriEntry(out, file->getSpentUris().begin(), file->getSpentUris().end(),
                 URI_USED);
  createUriEntry(out, file->getRemainingUris().begin(),
                 file->getRemainingUris().end(), URI_WAITING);
}
} // namespace

namespace {
FileData createFileData(const std::shared_ptr<FileEntry>& fe, int index,
                        const BitfieldMan* bf)
{
  FileData file;
  file.index = index;
  file.path = fe->getPath();
  file.length = fe->getLength();
  file.completedLength =
      bf->getOffsetCompletedLength(fe->getOffset(), fe->getLength());
  file.selected = fe->isRequested();
  createUriEntry(std::back_inserter(file.uris), fe);
  return file;
}
} // namespace

namespace {
template <typename OutputIterator, typename InputIterator>
void createFileEntry(OutputIterator out, InputIterator first,
                     InputIterator last, const BitfieldMan* bf)
{
  size_t index = 1;
  for (; first != last; ++first) {
    out++ = createFileData(*first, index++, bf);
  }
}
} // namespace

namespace {
template <typename OutputIterator, typename InputIterator>
void createFileEntry(OutputIterator out, InputIterator first,
                     InputIterator last, int64_t totalLength,
                     int32_t pieceLength, const std::string& bitfield)
{
  BitfieldMan bf(pieceLength, totalLength);
  bf.setBitfield(reinterpret_cast<const unsigned char*>(bitfield.data()),
                 bitfield.size());
  createFileEntry(out, first, last, &bf);
}
} // namespace

namespace {
template <typename OutputIterator, typename InputIterator>
void createFileEntry(OutputIterator out, InputIterator first,
                     InputIterator last, int64_t totalLength,
                     int32_t pieceLength,
                     const std::shared_ptr<PieceStorage>& ps)
{
  BitfieldMan bf(pieceLength, totalLength);
  if (ps) {
    bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
  }
  createFileEntry(out, first, last, &bf);
}
} // namespace

namespace {
template <typename OutputIterator>
void pushRequestOption(OutputIterator out,
                       const std::shared_ptr<Option>& option,
                       const std::shared_ptr<OptionParser>& oparser)
{
  for (size_t i = 1, len = option::countOption(); i < len; ++i) {
    PrefPtr pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if (h && h->getInitialOption() && option->defined(pref)) {
      out++ = KeyVals::value_type(pref->k, option->get(pref));
    }
  }
}
} // namespace

namespace {
const std::string& getRequestOption(const std::shared_ptr<Option>& option,
                                    const std::string& name)
{
  PrefPtr pref = option::k2p(name);
  if (OptionParser::getInstance()->find(pref)) {
    return option->get(pref);
  }
  else {
    return A2STR::NIL;
  }
}
} // namespace

namespace {
KeyVals getRequestOptions(const std::shared_ptr<Option>& option)
{
  KeyVals res;
  pushRequestOption(std::back_inserter(res), option,
                    OptionParser::getInstance());
  return res;
}
} // namespace

namespace {
struct RequestGroupDH : public DownloadHandle {
  RequestGroupDH(const std::shared_ptr<RequestGroup>& group)
      : group(group), ts(group->calculateStat())
  {
  }
  virtual ~RequestGroupDH() = default;
  virtual DownloadStatus getStatus() CXX11_OVERRIDE
  {
    if (group->getState() == RequestGroup::STATE_ACTIVE) {
      return DOWNLOAD_ACTIVE;
    }
    else {
      if (group->isPauseRequested()) {
        return DOWNLOAD_PAUSED;
      }
      else {
        return DOWNLOAD_WAITING;
      }
    }
  }
  virtual int64_t getTotalLength() CXX11_OVERRIDE
  {
    return group->getTotalLength();
  }
  virtual int64_t getCompletedLength() CXX11_OVERRIDE
  {
    return group->getCompletedLength();
  }
  virtual int64_t getUploadLength() CXX11_OVERRIDE
  {
    return ts.allTimeUploadLength;
  }
  virtual std::string getBitfield() CXX11_OVERRIDE
  {
    const std::shared_ptr<PieceStorage>& ps = group->getPieceStorage();
    if (ps) {
      return std::string(reinterpret_cast<const char*>(ps->getBitfield()),
                         ps->getBitfieldLength());
    }
    else {
      return "";
    }
  }
  virtual int getDownloadSpeed() CXX11_OVERRIDE { return ts.downloadSpeed; }
  virtual int getUploadSpeed() CXX11_OVERRIDE { return ts.uploadSpeed; }
  virtual const std::string& getInfoHash() CXX11_OVERRIDE
  {
#ifdef ENABLE_BITTORRENT
    if (group->getDownloadContext()->hasAttribute(CTX_ATTR_BT)) {
      return bittorrent::getTorrentAttrs(group->getDownloadContext())->infoHash;
    }
#endif // ENABLE_BITTORRENT
    return A2STR::NIL;
  }
  virtual size_t getPieceLength() CXX11_OVERRIDE
  {
    const std::shared_ptr<DownloadContext>& dctx = group->getDownloadContext();
    return dctx->getPieceLength();
  }
  virtual int getNumPieces() CXX11_OVERRIDE
  {
    return group->getDownloadContext()->getNumPieces();
  }
  virtual int getConnections() CXX11_OVERRIDE
  {
    return group->getNumConnection();
  }
  virtual int getErrorCode() CXX11_OVERRIDE
  {
    return group->getLastErrorCode();
  }
  virtual const std::vector<A2Gid>& getFollowedBy() CXX11_OVERRIDE
  {
    return group->followedBy();
  }
  virtual A2Gid getFollowing() CXX11_OVERRIDE { return group->following(); }
  virtual A2Gid getBelongsTo() CXX11_OVERRIDE { return group->belongsTo(); }
  virtual const std::string& getDir() CXX11_OVERRIDE
  {
    return group->getOption()->get(PREF_DIR);
  }
  virtual std::vector<FileData> getFiles() CXX11_OVERRIDE
  {
    std::vector<FileData> res;
    const std::shared_ptr<DownloadContext>& dctx = group->getDownloadContext();
    createFileEntry(std::back_inserter(res), dctx->getFileEntries().begin(),
                    dctx->getFileEntries().end(), dctx->getTotalLength(),
                    dctx->getPieceLength(), group->getPieceStorage());
    return res;
  }
  virtual int getNumFiles() CXX11_OVERRIDE
  {
    const std::shared_ptr<DownloadContext>& dctx = group->getDownloadContext();
    return dctx->getFileEntries().size();
  }
  virtual FileData getFile(int index) CXX11_OVERRIDE
  {
    const std::shared_ptr<DownloadContext>& dctx = group->getDownloadContext();
    BitfieldMan bf(dctx->getPieceLength(), dctx->getTotalLength());
    const std::shared_ptr<PieceStorage>& ps = group->getPieceStorage();
    if (ps) {
      bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
    }
    return createFileData(dctx->getFileEntries()[index - 1], index, &bf);
  }
  virtual BtMetaInfoData getBtMetaInfo() CXX11_OVERRIDE
  {
    BtMetaInfoData res;
#ifdef ENABLE_BITTORRENT
    if (group->getDownloadContext()->hasAttribute(CTX_ATTR_BT)) {
      auto torrentAttrs =
          bittorrent::getTorrentAttrs(group->getDownloadContext());
      res.announceList = torrentAttrs->announceList;
      res.comment = torrentAttrs->comment;
      res.creationDate = torrentAttrs->creationDate;
      res.mode = torrentAttrs->mode;
      if (!torrentAttrs->metadata.empty()) {
        res.name = torrentAttrs->name;
      }
    }
    else
#endif // ENABLE_BITTORRENT
    {
      res.creationDate = 0;
      res.mode = BT_FILE_MODE_NONE;
    }
    return res;
  }
  virtual const std::string& getOption(const std::string& name) CXX11_OVERRIDE
  {
    return getRequestOption(group->getOption(), name);
  }
  virtual KeyVals getOptions() CXX11_OVERRIDE
  {
    return getRequestOptions(group->getOption());
  }
  std::shared_ptr<RequestGroup> group;
  TransferStat ts;
};
} // namespace

namespace {
struct DownloadResultDH : public DownloadHandle {
  DownloadResultDH(std::shared_ptr<DownloadResult> dr) : dr(std::move(dr)) {}
  virtual ~DownloadResultDH() = default;
  virtual DownloadStatus getStatus() CXX11_OVERRIDE
  {
    switch (dr->result) {
    case error_code::FINISHED:
      return DOWNLOAD_COMPLETE;
    case error_code::REMOVED:
      return DOWNLOAD_REMOVED;
    default:
      return DOWNLOAD_ERROR;
    }
  }
  virtual int64_t getTotalLength() CXX11_OVERRIDE { return dr->totalLength; }
  virtual int64_t getCompletedLength() CXX11_OVERRIDE
  {
    return dr->completedLength;
  }
  virtual int64_t getUploadLength() CXX11_OVERRIDE { return dr->uploadLength; }
  virtual std::string getBitfield() CXX11_OVERRIDE { return dr->bitfield; }
  virtual int getDownloadSpeed() CXX11_OVERRIDE { return 0; }
  virtual int getUploadSpeed() CXX11_OVERRIDE { return 0; }
  virtual const std::string& getInfoHash() CXX11_OVERRIDE
  {
    return dr->infoHash;
  }
  virtual size_t getPieceLength() CXX11_OVERRIDE { return dr->pieceLength; }
  virtual int getNumPieces() CXX11_OVERRIDE { return dr->numPieces; }
  virtual int getConnections() CXX11_OVERRIDE { return 0; }
  virtual int getErrorCode() CXX11_OVERRIDE { return dr->result; }
  virtual const std::vector<A2Gid>& getFollowedBy() CXX11_OVERRIDE
  {
    return dr->followedBy;
  }
  virtual A2Gid getFollowing() CXX11_OVERRIDE { return dr->following; }
  virtual A2Gid getBelongsTo() CXX11_OVERRIDE { return dr->belongsTo; }
  virtual const std::string& getDir() CXX11_OVERRIDE { return dr->dir; }
  virtual std::vector<FileData> getFiles() CXX11_OVERRIDE
  {
    std::vector<FileData> res;
    createFileEntry(std::back_inserter(res), dr->fileEntries.begin(),
                    dr->fileEntries.end(), dr->totalLength, dr->pieceLength,
                    dr->bitfield);
    return res;
  }
  virtual int getNumFiles() CXX11_OVERRIDE { return dr->fileEntries.size(); }
  virtual FileData getFile(int index) CXX11_OVERRIDE
  {
    BitfieldMan bf(dr->pieceLength, dr->totalLength);
    bf.setBitfield(reinterpret_cast<const unsigned char*>(dr->bitfield.data()),
                   dr->bitfield.size());
    return createFileData(dr->fileEntries[index - 1], index, &bf);
  }
  virtual BtMetaInfoData getBtMetaInfo() CXX11_OVERRIDE
  {
    return BtMetaInfoData();
  }
  virtual const std::string& getOption(const std::string& name) CXX11_OVERRIDE
  {
    return getRequestOption(dr->option, name);
  }
  virtual KeyVals getOptions() CXX11_OVERRIDE
  {
    return getRequestOptions(dr->option);
  }
  std::shared_ptr<DownloadResult> dr;
};
} // namespace

DownloadHandle* getDownloadHandle(Session* session, A2Gid gid)
{
  auto& e = session->context->reqinfo->getDownloadEngine();
  auto& rgman = e->getRequestGroupMan();
  std::shared_ptr<RequestGroup> group = rgman->findGroup(gid);
  if (group) {
    return new RequestGroupDH(group);
  }
  else {
    std::shared_ptr<DownloadResult> ds = rgman->findDownloadResult(gid);
    if (ds) {
      return new DownloadResultDH(ds);
    }
  }
  return nullptr;
}

void deleteDownloadHandle(DownloadHandle* dh) { delete dh; }

} // namespace aria2
