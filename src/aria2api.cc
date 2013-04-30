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
  Session* session = new Session(options);
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

int run(Session* session, RUN_MODE mode)
{
  const SharedHandle<DownloadEngine>& e =
    session->context->reqinfo->getDownloadEngine();
  return e->run(mode == RUN_ONCE);
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
struct RequestGroupDH : public DownloadHandle {
  RequestGroupDH(const SharedHandle<RequestGroup>& group)
    : group(group),
      ts(group->calculateStat())
  {}
  virtual ~RequestGroupDH() {}
  virtual DOWNLOAD_STATUS getStatus()
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
  virtual DOWNLOAD_STATUS getStatus()
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

DOWNLOAD_STATUS downloadGetStatus(DownloadHandle* dh)
{
  return dh->getStatus();
}

int64_t downloadGetTotalLength(DownloadHandle* dh)
{
  return dh->getTotalLength();
}

int64_t downloadGetCompletedLength(DownloadHandle* dh)
{
  return dh->getCompletedLength();
}

int64_t downloadGetUploadLength(DownloadHandle* dh)
{
  return dh->getUploadLength();
}

std::string downloadGetBitfield(DownloadHandle* dh)
{
  return dh->getBitfield();
}

int downloadGetDownloadSpeed(DownloadHandle* dh)
{
  return dh->getDownloadSpeed();
}

int downloadGetUploadSpeed(DownloadHandle* dh)
{
  return dh->getUploadSpeed();
}

size_t downloadGetNumPieces(DownloadHandle* dh)
{
  return dh->getNumPieces();
}

int downloadGetConnections(DownloadHandle* dh)
{
  return dh->getConnections();
}

int downloadGetErrorCode(DownloadHandle* dh)
{
  return dh->getErrorCode();
}

const std::vector<A2Gid>& downloadGetFollowedBy(DownloadHandle* dh)
{
  return dh->getFollowedBy();
}

A2Gid downloadGetBelongsTo(DownloadHandle* dh)
{
  return dh->getBelongsTo();
}

const std::string& downloadGetDir(DownloadHandle* dh)
{
  return dh->getDir();
}

} // namespace aria2
