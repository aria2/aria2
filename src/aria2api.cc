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
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

Session::Session(const KeyVals& options)
  : context(new Context(false, 0, 0, options))
{}

Session::~Session()
{}

SessionConfig::SessionConfig()
  : keepRunning(false),
    useSignalHandler(true),
    downloadEventCallback(0),
    userData(0)
{}

namespace {
Platform* platform = 0;
} // namespace

int libraryInit()
{
  global::initConsole(true);
  try {
    platform = new Platform();
  } catch(RecoverableException& e) {
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
  Session* session;
  try {
    session = new Session(options);
  } catch(RecoverableException& e) {
    return 0;
  }
  if(session->context->reqinfo) {
    if(!config.useSignalHandler) {
      session->context->reqinfo->setUseSignalHandler(false);
    }
    rv = session->context->reqinfo->prepare();
    if(rv != 0) {
      delete session;
      return 0;
    }
    const SharedHandle<DownloadEngine>& e =
      session->context->reqinfo->getDownloadEngine();
    if(config.keepRunning) {
      e->getRequestGroupMan()->setKeepRunning(true);
      // Add command to make aria2 keep event polling
      e->addCommand(new KeepRunningCommand(e->newCUID(), e.get()));
    }
    if(config.downloadEventCallback) {
      SharedHandle<DownloadEventListener> listener
        (new ApiCallbackDownloadEventListener(session,
                                              config.downloadEventCallback,
                                              config.userData));
      SingletonHolder<Notifier>::instance()
        ->addDownloadEventListener(listener);
    }
  } else {
    delete session;
    return 0;
  }
  return session;
}

int sessionFinal(Session* session)
{
  error_code::Value rv = session->context->reqinfo->getResult();
  delete session;
  return rv;
}

int run(Session* session, RUN_MODE mode)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  return e->run(mode == RUN_ONCE);
}

int shutdown(Session* session, bool force)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  if(force) {
    e->requestForceHalt();
  } else {
    e->requestHalt();
  }
  // Skip next polling timeout. This avoids 1 second delay when there
  // is no Command other than KeepRunningCommand in the queue.
  e->setNoWait(true);
  return 0;
}

std::string gidToHex(const A2Gid& gid)
{
  return GroupId::toHex(gid);
}

A2Gid hexToGid(const std::string& hex)
{
  A2Gid gid;
  if(GroupId::toNumericId(gid, hex.c_str()) == 0) {
    return gid;
  } else {
    return 0;
  }
}

bool isNull(const A2Gid& gid)
{
  return gid == 0;
}

namespace {
template<typename InputIterator, typename Pred>
void apiGatherOption
(InputIterator first, InputIterator last,
 Pred pred,
 Option* option,
 const SharedHandle<OptionParser>& optionParser)
{
  for(; first != last; ++first) {
    const std::string& optionName = (*first).first;
    const Pref* pref = option::k2p(optionName);
    const OptionHandler* handler = optionParser->find(pref);
    if(!handler || !pred(handler)) {
      // Just ignore the unacceptable options in this context.
      continue;
    }
    handler->parse(*option, (*first).second);
  }
}
} // namespace

namespace {
void apiGatherRequestOption(Option* option, const KeyVals& options,
                            const SharedHandle<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fun(&OptionHandler::getInitialOption),
                  option, optionParser);
}
} // namespace

namespace {
void apiGatherChangeableOption(Option* option, const KeyVals& options,
                               const SharedHandle<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fun(&OptionHandler::getChangeOption),
                  option, optionParser);
}
} // namespace

namespace {
void apiGatherChangeableOptionForReserved
(Option* option, const KeyVals& options,
 const SharedHandle<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fun(&OptionHandler::getChangeOptionForReserved),
                  option, optionParser);
}
} // namespace

namespace {
void apiGatherChangeableGlobalOption
(Option* option, const KeyVals& options,
 const SharedHandle<OptionParser>& optionParser)
{
  apiGatherOption(options.begin(), options.end(),
                  std::mem_fun(&OptionHandler::getChangeGlobalOption),
                  option, optionParser);
}
} // namespace

namespace {
void addRequestGroup(const SharedHandle<RequestGroup>& group,
                     const SharedHandle<DownloadEngine>& e,
                     int position)
{
  if(position >= 0) {
    e->getRequestGroupMan()->insertReservedGroup(position, group);
  } else {
    e->getRequestGroupMan()->addReservedGroup(group);
  }
}
} // namespace

int addUri(Session* session,
           A2Gid* gid,
           const std::vector<std::string>& uris,
           const KeyVals& options,
           int position)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  std::vector<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, requestOption, uris,
                           /* ignoreForceSeq = */ true,
                           /* ignoreLocalPath = */ true);
  if(!result.empty()) {
    addRequestGroup(result.front(), e, position);
    if(gid) {
      *gid = result.front()->getGID();
    }
  }
  return 0;
}

int addMetalink(Session* session,
                std::vector<A2Gid>* gids,
                const std::string& metalinkFile,
                const KeyVals& options,
                int position)
{
#ifdef ENABLE_METALINK
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  std::vector<SharedHandle<RequestGroup> > result;
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
    requestOption->put(PREF_METALINK_FILE, metalinkFile);
    createRequestGroupForMetalink(result, requestOption);
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  if(!result.empty()) {
    if(position >= 0) {
      e->getRequestGroupMan()->insertReservedGroup(position, result);
    } else {
      e->getRequestGroupMan()->addReservedGroup(result);
    }
    if(gids) {
      for(std::vector<SharedHandle<RequestGroup> >::const_iterator i =
            result.begin(), eoi = result.end(); i != eoi; ++i) {
        (*gids).push_back((*i)->getGID());
      }
    }
  }
  return 0;
#else // !ENABLE_METALINK
  return -1;
#endif // !ENABLE_METALINK
}

int addTorrent(Session* session,
               A2Gid* gid,
               const std::string& torrentFile,
               const std::vector<std::string>& webSeedUris,
               const KeyVals& options,
               int position)
{
#ifdef ENABLE_BITTORRENT
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<Option> requestOption(new Option(*e->getOption()));
  std::vector<SharedHandle<RequestGroup> > result;
  try {
    apiGatherRequestOption(requestOption.get(), options,
                           OptionParser::getInstance());
    requestOption->put(PREF_TORRENT_FILE, torrentFile);
    createRequestGroupForBitTorrent(result, requestOption, webSeedUris,
                                    torrentFile);
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
  if(!result.empty()) {
    addRequestGroup(result.front(), e, position);
    if(gid) {
      *gid = result.front()->getGID();
    }
  }
  return 0;
#else // !ENABLE_BITTORRENT
  return -1;
#endif // !ENABLE_BITTORRENT
}

int addTorrent(Session* session,
               A2Gid* gid,
               const std::string& torrentFile,
               const KeyVals& options,
               int position)
{
  return addTorrent(session, gid, torrentFile, std::vector<std::string>(),
                    options, position);
}

int removeDownload(Session* session, const A2Gid& gid, bool force)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(group) {
    if(group->getState() == RequestGroup::STATE_ACTIVE) {
      if(force) {
        group->setForceHaltRequested(true, RequestGroup::USER_REQUEST);
      } else {
        group->setHaltRequested(true, RequestGroup::USER_REQUEST);
      }
      e->setRefreshInterval(0);
    } else {
      if(group->isDependencyResolved()) {
        e->getRequestGroupMan()->removeReservedGroup(gid);
      } else {
        return -1;
      }
    }
  } else {
    return -1;
  }
  return 0;
}

int pauseDownload(Session* session, const A2Gid& gid, bool force)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(group) {
    bool reserved = group->getState() == RequestGroup::STATE_WAITING;
    if(pauseRequestGroup(group, reserved, force)) {
      e->setRefreshInterval(0);
      return 0;
    }
  }
  return -1;
}

int unpauseDownload(Session* session, const A2Gid& gid)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(!group ||
     group->getState() != RequestGroup::STATE_WAITING ||
     !group->isPauseRequested()) {
    return -1;
  } else {
    group->setPauseRequested(false);
    e->getRequestGroupMan()->requestQueueCheck();
  }
  return 0;
}

int changePosition(Session* session, const A2Gid& gid, int pos, OffsetMode how)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  try {
    return e->getRequestGroupMan()->changeReservedGroupPosition(gid, pos, how);
  } catch(RecoverableException& e) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
    return -1;
  }
}

int changeOption(Session* session, const A2Gid& gid, const KeyVals& options)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  SharedHandle<RequestGroup> group = e->getRequestGroupMan()->findGroup(gid);
  if(group) {
    Option option;
    try {
      if(group->getState() == RequestGroup::STATE_ACTIVE) {
        apiGatherChangeableOption(&option, options,
                                  OptionParser::getInstance());
      } else {
        apiGatherChangeableOptionForReserved(&option, options,
                                             OptionParser::getInstance());
      }
    } catch(RecoverableException& err) {
      A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, err);
      return -1;
    }
    changeOption(group, option, e.get());
    return 0;
  } else {
    return -1;
  }
}

const std::string& getGlobalOption(Session* session, const std::string& name)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  const Pref* pref = option::k2p(name);
  if(OptionParser::getInstance()->find(pref)) {
    return e->getOption()->get(pref);
  } else {
    return A2STR::NIL;
  }
}

KeyVals getGlobalOptions(Session* session)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  const SharedHandle<OptionParser>& optionParser = OptionParser::getInstance();
  const Option* option = e->getOption();
  KeyVals options;
  for(size_t i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    if(option->defined(pref) && optionParser->find(pref)) {
      options.push_back(KeyVals::value_type(pref->k, option->get(pref)));
    }
  }
  return options;
}

int changeGlobalOption(Session* session, const KeyVals& options)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  Option option;
  try {
    apiGatherChangeableGlobalOption(&option, options,
                                    OptionParser::getInstance());
  } catch(RecoverableException& err) {
    A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, err);
    return -1;
  }
  changeGlobalOption(option, e.get());
  return 0;
}

GlobalStat getGlobalStat(Session* session)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  const SharedHandle<RequestGroupMan>& rgman = e->getRequestGroupMan();
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
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  const RequestGroupList& groups = e->getRequestGroupMan()->getRequestGroups();
  std::vector<A2Gid> res;
  for(RequestGroupList::const_iterator i = groups.begin(),
        eoi = groups.end(); i != eoi; ++i) {
    res.push_back((*i)->getGID());
  }
  return res;
}

namespace {
template<typename OutputIterator, typename InputIterator>
void createUriEntry
(OutputIterator out,
 InputIterator first, InputIterator last,
 UriStatus status)
{
  for(; first != last; ++first) {
    UriData uriData;
    uriData.uri = *first;
    uriData.status = status;
    out++ = uriData;
  }
}
} // namespace

namespace {
template<typename OutputIterator>
void createUriEntry
(OutputIterator out, const SharedHandle<FileEntry>& file)
{
  createUriEntry(out,
                 file->getSpentUris().begin(),
                 file->getSpentUris().end(),
                 URI_USED);
  createUriEntry(out,
                 file->getRemainingUris().begin(),
                 file->getRemainingUris().end(),
                 URI_WAITING);
}
} // namespace

namespace {
FileData createFileData
(const SharedHandle<FileEntry>& fe, int index, const BitfieldMan* bf)
{
  FileData file;
  file.index = index;
  file.path = fe->getPath();
  file.length = fe->getLength();
  file.completedLength = bf->getOffsetCompletedLength
    (fe->getOffset(), fe->getLength());
  file.selected = fe->isRequested();
  createUriEntry(std::back_inserter(file.uris), fe);
  return file;
}
} // namespace

namespace {
template<typename OutputIterator, typename InputIterator>
void createFileEntry
(OutputIterator out,
 InputIterator first, InputIterator last,
 const BitfieldMan* bf)
{
  size_t index = 1;
  for(; first != last; ++first) {
    out++ = createFileData(*first, index++, bf);
  }
}
} // namespace

namespace {
template<typename OutputIterator, typename InputIterator>
void createFileEntry
(OutputIterator out,
 InputIterator first, InputIterator last,
 int64_t totalLength,
 int32_t pieceLength,
 const std::string& bitfield)
{
  BitfieldMan bf(pieceLength, totalLength);
  bf.setBitfield(reinterpret_cast<const unsigned char*>(bitfield.data()),
                 bitfield.size());
  createFileEntry(out, first, last, &bf);
}
} // namespace

namespace {
template<typename OutputIterator, typename InputIterator>
void createFileEntry
(OutputIterator out,
 InputIterator first, InputIterator last,
 int64_t totalLength,
 int32_t pieceLength,
 const SharedHandle<PieceStorage>& ps)
{
  BitfieldMan bf(pieceLength, totalLength);
  if(ps) {
    bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
  }
  createFileEntry(out, first, last, &bf);
}
} // namespace

namespace {
template<typename OutputIterator>
void pushRequestOption
(OutputIterator out,
 const SharedHandle<Option>& option,
 const SharedHandle<OptionParser>& oparser)
{
  for(size_t i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    const OptionHandler* h = oparser->find(pref);
    if(h && h->getInitialOption() && option->defined(pref)) {
      out++ = KeyVals::value_type(pref->k, option->get(pref));
    }
  }
}
} // namespace

namespace {
struct RequestGroupDH : public DownloadHandle {
  RequestGroupDH(const SharedHandle<RequestGroup>& group)
    : group(group),
      ts(group->calculateStat())
  {}
  virtual ~RequestGroupDH() {}
  virtual DownloadStatus getStatus()
  {
    if(group->getState() == RequestGroup::STATE_ACTIVE) {
      return DOWNLOAD_ACTIVE;
    } else {
      if(group->isPauseRequested()) {
        return DOWNLOAD_PAUSED;
      } else {
        return DOWNLOAD_WAITING;
      }
    }
  }
  virtual int64_t getTotalLength()
  {
    return group->getTotalLength();
  }
  virtual int64_t getCompletedLength()
  {
    return group->getCompletedLength();
  }
  virtual int64_t getUploadLength()
  {
    return ts.allTimeUploadLength;
  }
  virtual std::string getBitfield()
  {
    const SharedHandle<PieceStorage>& ps = group->getPieceStorage();
    if(ps) {
      return std::string(reinterpret_cast<const char*>(ps->getBitfield()),
                         ps->getBitfieldLength());
    } else {
      return "";
    }
  }
  virtual int getDownloadSpeed()
  {
    return ts.downloadSpeed;
  }
  virtual int getUploadSpeed()
  {
    return ts.uploadSpeed;
  }
  virtual const std::string& getInfoHash()
  {
#ifdef ENABLE_BITTORRENT
    if(group->getDownloadContext()->hasAttribute(CTX_ATTR_BT)) {
      SharedHandle<TorrentAttribute> torrentAttrs =
        bittorrent::getTorrentAttrs(group->getDownloadContext());
      return torrentAttrs->infoHash;
    }
#endif // ENABLE_BITTORRENT
    return A2STR::NIL;
  }
  virtual size_t getPieceLength()
  {
    const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
    return dctx->getPieceLength();
  }
  virtual int getNumPieces()
  {
    return group->getDownloadContext()->getNumPieces();
  }
  virtual int getConnections()
  {
    return group->getNumConnection();
  }
  virtual int getErrorCode()
  {
    return 0;
  }
  virtual const std::vector<A2Gid>& getFollowedBy()
  {
    return group->followedBy();
  }
  virtual A2Gid getBelongsTo()
  {
    return group->belongsTo();
  }
  virtual const std::string& getDir()
  {
    return group->getOption()->get(PREF_DIR);
  }
  virtual std::vector<FileData> getFiles()
  {
    std::vector<FileData> res;
    const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
    createFileEntry(std::back_inserter(res),
                    dctx->getFileEntries().begin(),
                    dctx->getFileEntries().end(),
                    dctx->getTotalLength(), dctx->getPieceLength(),
                    group->getPieceStorage());
    return res;
  }
  virtual int getNumFiles()
  {
    const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
    return dctx->getFileEntries().size();
  }
  virtual FileData getFile(int index)
  {
    const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
    BitfieldMan bf(dctx->getPieceLength(), dctx->getTotalLength());
    const SharedHandle<PieceStorage>& ps = group->getPieceStorage();
    if(ps) {
      bf.setBitfield(ps->getBitfield(), ps->getBitfieldLength());
    }
    return createFileData(dctx->getFileEntries()[index-1], index, &bf);
  }
  virtual BtMetaInfoData getBtMetaInfo()
  {
    BtMetaInfoData res;
#ifdef ENABLE_BITTORRENT
    if(group->getDownloadContext()->hasAttribute(CTX_ATTR_BT)) {
      SharedHandle<TorrentAttribute> torrentAttrs =
        bittorrent::getTorrentAttrs(group->getDownloadContext());
      res.announceList = torrentAttrs->announceList;
      res.comment = torrentAttrs->comment;
      res.creationDate = torrentAttrs->creationDate;
      res.mode = torrentAttrs->mode;
      if(!torrentAttrs->metadata.empty()) {
        res.name = torrentAttrs->name;
      }
    } else
#endif // ENABLE_BITTORRENT
      {
        res.creationDate = 0;
        res.mode = BT_FILE_MODE_NONE;
      }
    return res;
  }
  virtual const std::string& getOption(const std::string& name)
  {
    const Pref* pref = option::k2p(name);
    if(OptionParser::getInstance()->find(pref)) {
      return group->getOption()->get(pref);
    } else {
      return A2STR::NIL;
    }
  }
  virtual KeyVals getOptions()
  {
    KeyVals res;
    pushRequestOption(std::back_inserter(res), group->getOption(),
                      OptionParser::getInstance());
    return res;
  }
  SharedHandle<RequestGroup> group;
  TransferStat ts;
};
} // namespace

namespace {
struct DownloadResultDH : public DownloadHandle {
  DownloadResultDH(const SharedHandle<DownloadResult>& dr)
    : dr(dr)
  {}
  virtual ~DownloadResultDH() {}
  virtual DownloadStatus getStatus()
  {
    switch(dr->result) {
    case error_code::FINISHED:
      return DOWNLOAD_COMPLETE;
    case error_code::REMOVED:
      return DOWNLOAD_REMOVED;
    default:
      return DOWNLOAD_ERROR;
    }
  }
  virtual int64_t getTotalLength()
  {
    return dr->totalLength;
  }
  virtual int64_t getCompletedLength()
  {
    return dr->completedLength;
  }
  virtual int64_t getUploadLength()
  {
    return dr->uploadLength;
  }
  virtual std::string getBitfield()
  {
    return dr->bitfield;
  }
  virtual int getDownloadSpeed()
  {
    return 0;
  }
  virtual int getUploadSpeed()
  {
    return 0;
  }
  virtual const std::string& getInfoHash()
  {
    return dr->infoHash;
  }
  virtual size_t getPieceLength()
  {
    return dr->pieceLength;
  }
  virtual int getNumPieces()
  {
    return dr->numPieces;
  }
  virtual int getConnections()
  {
    return 0;
  }
  virtual int getErrorCode()
  {
    return dr->result;
  }
  virtual const std::vector<A2Gid>& getFollowedBy()
  {
    return dr->followedBy;
  }
  virtual A2Gid getBelongsTo()
  {
    return dr->belongsTo;
  }
  virtual const std::string& getDir()
  {
    return dr->dir;
  }
  virtual std::vector<FileData> getFiles()
  {
    std::vector<FileData> res;
    createFileEntry(std::back_inserter(res),
                    dr->fileEntries.begin(), dr->fileEntries.end(),
                    dr->totalLength, dr->pieceLength, dr->bitfield);
    return res;
  }
  virtual int getNumFiles()
  {
    return dr->fileEntries.size();
  }
  virtual FileData getFile(int index)
  {
    BitfieldMan bf(dr->pieceLength, dr->totalLength);
    bf.setBitfield(reinterpret_cast<const unsigned char*>(dr->bitfield.data()),
                   dr->bitfield.size());
    return createFileData(dr->fileEntries[index-1], index, &bf);
  }
  virtual BtMetaInfoData getBtMetaInfo()
  {
    return BtMetaInfoData();
  }
  virtual const std::string& getOption(const std::string& name)
  {
    return A2STR::NIL;
  }
  virtual KeyVals getOptions()
  {
    return KeyVals();
  }
  SharedHandle<DownloadResult> dr;
};
} // namespace

DownloadHandle* getDownloadHandle(Session* session, const A2Gid& gid)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  const SharedHandle<RequestGroupMan>& rgman = e->getRequestGroupMan();
  SharedHandle<RequestGroup> group = rgman->findGroup(gid);
  if(group) {
    return new RequestGroupDH(group);
  } else {
    SharedHandle<DownloadResult> ds = rgman->findDownloadResult(gid);
    if(ds) {
      return new DownloadResultDH(ds);
    }
  }
  return 0;
}

void deleteDownloadHandle(DownloadHandle* dh)
{
  delete dh;
}

} // namespace aria2
