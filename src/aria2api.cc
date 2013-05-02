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

namespace aria2 {

Session::Session(const KeyVals& options)
  : context(new Context(false, 0, 0, options))
{}

Session::~Session()
{}

namespace {
Platform* platform = 0;
} // namespace

int libraryInit()
{
  platform = new Platform();
  return 0;
}

int libraryDeinit()
{
  delete platform;
  return 0;
}

Session* sessionNew(const KeyVals& options)
{
  int rv;
  Session* session;
  try {
    session = new Session(options);
  } catch(RecoverableException& e) {
    return 0;
  }
  if(session->context->reqinfo) {
    rv = session->context->reqinfo->prepare();
    if(rv != 0) {
      delete session;
      session = 0;
    }
  } else {
    delete session;
    session = 0;
  }
  return session;
}

int sessionFinal(Session* session)
{
  error_code::Value rv = session->context->reqinfo->getResult();
  delete session;
  return rv;
}

int sessionConfigSetKeepRunning(Session* session, bool flag)
{
  session->context->reqinfo->getDownloadEngine()->getRequestGroupMan()
    ->setKeepRunning(flag);
  return 0;
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
    if(!pref) {
      throw DL_ABORT_EX
        (fmt("We don't know how to deal with %s option", optionName.c_str()));
    }
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
           A2Gid& gid,
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
    gid = result.front()->getGID();
    addRequestGroup(result.front(), e, position);
  }
  return 0;
}

int addMetalink(Session* session,
                std::vector<A2Gid>& gids,
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
    for(std::vector<SharedHandle<RequestGroup> >::const_iterator i =
          result.begin(), eoi = result.end(); i != eoi; ++i) {
      gids.push_back((*i)->getGID());
    }
  }
  return 0;
#else // !ENABLE_METALINK
  return -1;
#endif // !ENABLE_METALINK
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
template<typename OutputIterator, typename InputIterator>
void createFileEntry
(OutputIterator out,
 InputIterator first, InputIterator last,
 const BitfieldMan* bf)
{
  size_t index = 1;
  for(; first != last; ++first) {
    FileData file;
    file.index = index++;
    file.path = (*first)->getPath();
    file.length = (*first)->getLength();
    file.completedLength = bf->getOffsetCompletedLength
      ((*first)->getOffset(), (*first)->getLength());
    file.selected = (*first)->isRequested();
    createUriEntry(std::back_inserter(file.uris), *first);
    out++ = file;
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
  virtual size_t getNumPieces()
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
  virtual size_t getNumPieces()
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
