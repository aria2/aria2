/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "XmlRpcMethodImpl.h"

#include <cassert>
#include <algorithm>

#include "Logger.h"
#include "BDE.h"
#include "DlAbortEx.h"
#include "Option.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "Util.h"
#include "RequestGroupMan.h"
#include "StringFormat.h"
#include "XmlRpcRequest.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "BtContext.h"
#include "BtRegistry.h"
#include "Peer.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "BtProgressInfoFile.h"
#include "BtRuntime.h"
#include "BtAnnounce.h"
#include "prefs.h"
#include "message.h"

namespace aria2 {

namespace xmlrpc {

static const BDE BDE_TRUE = BDE("true");
static const BDE BDE_FALSE = BDE("false");
static const BDE BDE_OK = BDE("OK");
static const BDE BDE_ACTIVE = BDE("active");
static const BDE BDE_WAITING = BDE("waiting");
static const BDE BDE_REMOVED = BDE("removed");
static const BDE BDE_ERROR = BDE("error");
static const BDE BDE_COMPLETE = BDE("complete");

static BDE createGIDResponse(int32_t gid)
{
  return BDE(Util::itos(gid));
}

static BDE addRequestGroup(const SharedHandle<RequestGroup>& group,
			   DownloadEngine* e,
			   bool posGiven, int pos)
{
  if(posGiven) {
    e->_requestGroupMan->insertReservedGroup(pos, group);
  } else {
    e->_requestGroupMan->addReservedGroup(group);
  }
  return createGIDResponse(group->getGID());
}

static bool hasDictParam(const BDE& params, size_t index)
{
  return params.size() > index && params[index].isDict();
}

static void getPosParam(const BDE& params, size_t posParamIndex,
			bool& posGiven, size_t& pos)
{
  if(params.size() > posParamIndex && params[posParamIndex].isInteger()) {
    pos = params[posParamIndex].i();
    posGiven = true;
  } else {
    posGiven = false;
  }
} 

BDE AddUriXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isList() || params[0].empty()) {
    throw DL_ABORT_EX("URI is not provided.");
  }
  std::deque<std::string> uris;
  for(BDE::List::const_iterator i = params[0].listBegin();
      i != params[0].listEnd(); ++i) {
    if((*i).isString()) {
      uris.push_back((*i).s());
    }
  }

  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 1)) {
    gatherRequestOption(requestOption, params[1]);
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 2, posGiven, pos);

  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, requestOption, uris,
			   /* ignoreForceSeq = */ true,
			   /* ignoreNonURI = */ true);

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No URI to download.");
  }
}

#ifdef ENABLE_BITTORRENT
BDE AddTorrentXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX("Torrent data is not provided.");
  }
  
  std::deque<std::string> uris;
  if(params.size() > 1 && params[1].isList()) {
    for(BDE::List::const_iterator i = params[1].listBegin();
	i != params[1].listEnd(); ++i) {
      if((*i).isString()) {
	uris.push_back((*i).s());
      }
    }
  }
  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 2)) {
    gatherRequestOption(requestOption, params[2]);
  }
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 3, posGiven, pos);

  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForBitTorrent(result, requestOption,
				  uris,
				  params[0].s());

  if(!result.empty()) {
    return addRequestGroup(result.front(), e, posGiven, pos);
  } else {
    throw DL_ABORT_EX("No Torrent to download.");
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
BDE AddMetalinkXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX("Metalink data is not provided.");
  }
  
  SharedHandle<Option> requestOption(new Option(*e->option));
  if(hasDictParam(params, 1)) {
    gatherRequestOption(requestOption, params[1]);
  };
  size_t pos = 0;
  bool posGiven = false;
  getPosParam(params, 2, posGiven, pos);

  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForMetalink(result, requestOption, params[0].s());
  if(!result.empty()) {
    if(posGiven) {
      e->_requestGroupMan->insertReservedGroup(pos, result);
    } else {
      e->_requestGroupMan->addReservedGroup(result);
    }
    BDE gids = BDE::list();
    for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
	  result.begin(); i != result.end(); ++i) {
      gids << BDE(Util::itos((*i)->getGID()));
    }
    return gids;
  } else {
    throw DL_ABORT_EX("No files to download.");
  }
} 
#endif // ENABLE_METALINK

BDE RemoveXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  if(group.isNull()) {
    group = e->_requestGroupMan->findReservedGroup(gid);
    if(group.isNull()) {
      throw DL_ABORT_EX
	(StringFormat("Active Download not found for GID#%d", gid).str());
    }
    if(group->isDependencyResolved()) {
      e->_requestGroupMan->removeReservedGroup(gid);
    } else {
      throw DL_ABORT_EX
	(StringFormat("GID#%d cannot be removed now", gid).str());
    }
  } else {
    group->setHaltRequested(true, RequestGroup::USER_REQUEST);
  }

  return createGIDResponse(gid);
}

static void gatherProgressCommon
(BDE& entryDict, const SharedHandle<RequestGroup>& group)
{
  entryDict["gid"] = Util::itos(group->getGID());
  // This is "filtered" total length if --select-file is used.
  entryDict["totalLength"] = Util::uitos(group->getTotalLength());
  // This is "filtered" total length if --select-file is used.
  entryDict["completedLength"] = Util::uitos(group->getCompletedLength());
  TransferStat stat = group->calculateStat();
  entryDict["downloadSpeed"] = Util::uitos(stat.getDownloadSpeed());
  entryDict["uploadSpeed"] = Util::uitos(stat.getUploadSpeed());
  entryDict["connections"] = Util::uitos(group->getNumConnection());
  SharedHandle<PieceStorage> ps = group->getPieceStorage();
  if(!ps.isNull()) {
    if(ps->getBitfieldLength() > 0) {
      entryDict["bitfield"] = Util::toHex(ps->getBitfield(),
					  ps->getBitfieldLength());
    }
  }
  entryDict["pieceLength"] = 
    Util::uitos(group->getDownloadContext()->getPieceLength());
  entryDict["numPieces"] =
    Util::uitos(group->getDownloadContext()->getNumPieces());
}

#ifdef ENABLE_BITTORRENT
static void gatherProgressBitTorrent
(BDE& entryDict, const SharedHandle<BtContext>& btctx)
{
  entryDict["infoHash"] = btctx->getInfoHashAsString();
}

static void gatherPeer(BDE& peers, const SharedHandle<PeerStorage>& ps)
{
  std::deque<SharedHandle<Peer> > activePeers;
  ps->getActivePeers(activePeers);
  for(std::deque<SharedHandle<Peer> >::const_iterator i =
	activePeers.begin(); i != activePeers.end(); ++i) {
    BDE peerEntry = BDE::dict();
    peerEntry["peerId"] = Util::torrentUrlencode((*i)->getPeerId(),
						 PEER_ID_LENGTH);
    peerEntry["ip"] = (*i)->ipaddr;
    peerEntry["port"] = Util::uitos((*i)->port);
    peerEntry["bitfield"] = Util::toHex((*i)->getBitfield(),
					(*i)->getBitfieldLength());
    peerEntry["amChoking"] = (*i)->amChoking()?BDE_TRUE:BDE_FALSE;
    peerEntry["peerChoking"] = (*i)->peerChoking()?BDE_TRUE:BDE_FALSE;
    TransferStat stat = ps->getTransferStatFor(*i);
    peerEntry["downloadSpeed"] = Util::uitos(stat.getDownloadSpeed());
    peerEntry["uploadSpeed"] = Util::uitos(stat.getUploadSpeed());
    peerEntry["seeder"] = (*i)->isSeeder()?BDE_TRUE:BDE_FALSE;
    peers << peerEntry;
  }
}
#endif // ENABLE_BITTORRENT

static void gatherProgress
(BDE& entryDict, const SharedHandle<RequestGroup>& group, DownloadEngine* e)
{
  gatherProgressCommon(entryDict, group);
#ifdef ENABLE_BITTORRENT
  SharedHandle<BtContext> btctx =
    dynamic_pointer_cast<BtContext>(group->getDownloadContext());
  if(!btctx.isNull()) {
    gatherProgressBitTorrent(entryDict, btctx);
  }
#endif // ENABLE_BITTORRENT
}

static void gatherStoppedDownload
(BDE& entryDict, const SharedHandle<DownloadResult>& ds)
{
  entryDict["gid"] = Util::itos(ds->gid);
  if(ds->result == DownloadResult::IN_PROGRESS) {
    entryDict["status"] = BDE_REMOVED;
  } else if(ds->result == DownloadResult::FINISHED) {
    entryDict["status"] = BDE_COMPLETE;
  } else {
    entryDict["status"] = BDE_ERROR;
  }
}

static
SharedHandle<RequestGroup>
findRequestGroup(const SharedHandle<RequestGroupMan>& rgman, int32_t gid)
{
  SharedHandle<RequestGroup> group = rgman->findRequestGroup(gid);
  if(group.isNull()) {
    group = rgman->findReservedGroup(gid);
  }
  return group;
}

template<typename InputIterator>
static void createFileEntry(BDE& files, InputIterator first, InputIterator last)
{
  size_t index = 1;
  for(; first != last; ++first, ++index) {
    BDE entry = BDE::dict();
    entry["index"] = Util::uitos(index);
    entry["path"] = (*first)->getPath();
    entry["selected"] = (*first)->isRequested()?BDE_TRUE:BDE_FALSE;
    entry["length"] = Util::uitos((*first)->getLength());
    files << entry;
  }
}

BDE GetFilesXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  BDE files = BDE::list();
  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    SharedHandle<DownloadResult> dr =
      e->_requestGroupMan->findDownloadResult(gid);
    if(dr.isNull()) {
      throw DL_ABORT_EX
	(StringFormat("No file data is available for GID#%d", gid).str());
    } else {
      createFileEntry(files, dr->fileEntries.begin(), dr->fileEntries.end());
    }
  } else {
    std::deque<SharedHandle<FileEntry> > fileEntries =
      group->getDownloadContext()->getFileEntries();
    createFileEntry(files, fileEntries.begin(), fileEntries.end());
  }
  return files;
}

BDE GetUrisXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No URI data is available for GID#%d", gid).str());
  }
  BDE uriList = BDE::list();
  std::deque<std::string> uris;
  group->getURIs(uris);
  for(std::deque<std::string>::const_iterator i = uris.begin(); i != uris.end();
      ++i) {
    BDE entry = BDE::dict();
    entry["uri"] = *i;
    uriList << entry;
  }
  return uriList;
}

#ifdef ENABLE_BITTORRENT
BDE GetPeersXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("No peer data is available for GID#%d", gid).str());
  }
  BDE peers = BDE::list();
  SharedHandle<BtContext> btctx =
    dynamic_pointer_cast<BtContext>(group->getDownloadContext());
  if(!btctx.isNull()) {
    SharedHandle<BtRegistry> btreg = e->getBtRegistry();
    SharedHandle<PeerStorage> ps =
      btreg->getPeerStorage(btctx->getInfoHashAsString());
    if(!ps.isNull()) {
      BDE entry = BDE::dict();
      gatherPeer(peers, ps);
    }
  }
  return peers;
}
#endif // ENABLE_BITTORRENT

BDE TellStatusXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  BDE entryDict = BDE::dict();
  if(group.isNull()) {
    group = e->_requestGroupMan->findReservedGroup(gid);
    if(group.isNull()) {
      SharedHandle<DownloadResult> ds =
	e->_requestGroupMan->findDownloadResult(gid);
      if(ds.isNull()) {
	throw DL_ABORT_EX
	  (StringFormat("No such download for GID#%d", gid).str());
      }
      gatherStoppedDownload(entryDict, ds);
    } else {
      entryDict["status"] = BDE_WAITING;
      gatherProgress(entryDict, group, e);
    }
  } else {
    entryDict["status"] = BDE_ACTIVE;
    gatherProgress(entryDict, group, e);
  }
  return entryDict;
}

BDE TellActiveXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  BDE list = BDE::list();
  const std::deque<SharedHandle<RequestGroup> >& groups =
    e->_requestGroupMan->getRequestGroups();
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
	groups.begin(); i != groups.end(); ++i) {
    BDE entryDict = BDE::dict();
    entryDict["status"] = BDE_ACTIVE;
    gatherProgress(entryDict, *i, e);
    list << entryDict;
  }
  return list;
}

BDE PurgeDownloadResultXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  e->_requestGroupMan->purgeDownloadResult();
  return BDE_OK;
}

BDE ChangeOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DL_ABORT_EX(MSG_GID_NOT_PROVIDED);
  }  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = findRequestGroup(e->_requestGroupMan, gid);
  if(group.isNull()) {
    throw DL_ABORT_EX
      (StringFormat("Cannot change option for GID#%d", gid).str());
  }
  SharedHandle<Option> option(new Option(*group->getOption().get()));
  if(params.size() > 1 && params[1].isDict()) {
    gatherChangeableOption(option, params[1]);
  }
  if(option->defined(PREF_MAX_DOWNLOAD_LIMIT)) {
    group->setMaxDownloadSpeedLimit(option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_UPLOAD_LIMIT)) {
    group->setMaxUploadSpeedLimit(option->getAsInt(PREF_MAX_UPLOAD_LIMIT));
  }
  return BDE_OK;
}

BDE ChangeGlobalOptionXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isDict()) {
    return BDE_OK;
  }
  SharedHandle<Option> option(new Option(*e->option));
  gatherChangeableGlobalOption(option, params[0]);
  if(option->defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)) {
    e->_requestGroupMan->setMaxOverallDownloadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_OVERALL_UPLOAD_LIMIT)) {
    e->_requestGroupMan->setMaxOverallUploadSpeedLimit
      (option->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT));
  }
  if(option->defined(PREF_MAX_CONCURRENT_DOWNLOADS)) {
    e->_requestGroupMan->setMaxSimultaneousDownloads
      (option->getAsInt(PREF_MAX_CONCURRENT_DOWNLOADS));
  }
  return BDE_OK;
}

BDE NoSuchMethodXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  throw DL_ABORT_EX
    (StringFormat("No such method: %s", req._methodName.c_str()).str());
}

} // namespace xmlrpc

} // namespace aria2
