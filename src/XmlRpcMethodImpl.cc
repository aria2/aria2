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

namespace aria2 {

namespace xmlrpc {

static BDE createGIDResponse(int32_t gid)
{
  BDE resParams = BDE::list();
  resParams << BDE(Util::itos(gid));
  return resParams;
}

BDE AddURIXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isList() || params[0].empty()) {
    throw DlAbortEx("URI is not provided.");
  }
  std::deque<std::string> uris;
  for(BDE::List::const_iterator i = params[0].listBegin();
      i != params[0].listEnd(); ++i) {
    if((*i).isString()) {
      uris.push_back((*i).s());
    }
  }

  SharedHandle<Option> requestOption(new Option(*e->option));
  if(params.size() > 1 && params[1].isDict()) {
    gatherRequestOption(requestOption, params[1]);
  }
  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForUri(result, requestOption, uris,
			   /* ignoreForceSeq = */ true,
			   /* ignoreNonURI = */ true);

  if(!result.empty()) {
    e->_requestGroupMan->addReservedGroup(result.front());
    return createGIDResponse(result.front()->getGID());
  } else {
    throw DlAbortEx("No URI to download.");
  }
}

BDE AddTorrentFileXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());
  if(params.empty() || !params[0].isString()) {
    throw DlAbortEx("Torrent data is not provided.");
  }
  
  // TODO should accept uris from xml rpc request

  SharedHandle<Option> requestOption(new Option(*e->option));
  if(params.size() > 2 && params[2].isDict()) {
    gatherRequestOption(requestOption, params[2]);
  }
  std::deque<SharedHandle<RequestGroup> > result;
  createRequestGroupForBitTorrent(result, requestOption,
				  std::deque<std::string>(),
				  params[0].s());

  if(!result.empty()) {
    e->_requestGroupMan->addReservedGroup(result.front());
    return createGIDResponse(result.front()->getGID());
  } else {
    throw DlAbortEx("No Torrent to download.");
  }
} 

BDE RemoveXmlRpcMethod::process(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DlAbortEx("GID is not provided.");
  }
  
  int32_t gid = Util::parseInt(params[0].s());

  SharedHandle<RequestGroup> group = e->_requestGroupMan->findRequestGroup(gid);

  if(group.isNull()) {
    group = e->_requestGroupMan->findReservedGroup(gid);
    if(group.isNull()) {
      throw DlAbortEx
	(StringFormat("Active Download not found for GID#%d", gid).str());
    }
    if(group->isDependencyResolved()) {
      e->_requestGroupMan->removeReservedGroup(gid);
    } else {
      throw DlAbortEx
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
  entryDict["gid"] = BDE(Util::itos(group->getGID()));
  entryDict["totalLength"] = BDE(Util::uitos(group->getTotalLength()));
  entryDict["completedLength"] = BDE(Util::uitos(group->getCompletedLength()));
  TransferStat stat = group->calculateStat();
  entryDict["downloadSpeed"] = BDE(Util::uitos(stat.getDownloadSpeed()));
  entryDict["uploadSpeed"] = BDE(Util::uitos(stat.getUploadSpeed()));
  entryDict["connections"] = group->getNumConnection();
  SharedHandle<PieceStorage> ps = group->getPieceStorage();
  if(!ps.isNull()) {
    if(ps->getBitfieldLength() > 0) {
      entryDict["bitfield"] = BDE(Util::toHex(ps->getBitfield(),
					      ps->getBitfieldLength()));
    }

    if(!ps->getDiskAdaptor().isNull()) {
      BDE files = BDE::list();
      const std::deque<SharedHandle<FileEntry> >& entries =
	ps->getDiskAdaptor()->getFileEntries();
      for(std::deque<SharedHandle<FileEntry> >::const_iterator i =
	    entries.begin(); i != entries.end(); ++i) {
	files << BDE((*i)->getPath());
      }
      entryDict["files"] = files;
    }
  }

  entryDict["pieceLength"] = 
    BDE(Util::uitos(group->getDownloadContext()->getPieceLength()));
  entryDict["numPieces"] =
    BDE(Util::uitos(group->getDownloadContext()->getNumPieces()));
}

static void gatherProgressBitTorrent
(BDE& entryDict, const SharedHandle<BtContext>& btctx)
{
  entryDict["infoHash"] = BDE(btctx->getInfoHashAsString());
}

static void gatherPeer(BDE& entryDict, const SharedHandle<PeerStorage>& ps)
{
  BDE peers = BDE::list();

  std::deque<SharedHandle<Peer> > activePeers;
  ps->getActivePeers(activePeers);
  for(std::deque<SharedHandle<Peer> >::const_iterator i =
	activePeers.begin(); i != activePeers.end(); ++i) {
    BDE peerEntry = BDE::dict();
    peerEntry["peerId"] = BDE(Util::torrentUrlencode((*i)->getPeerId(),
						     PEER_ID_LENGTH));
    peerEntry["ip"] = BDE((*i)->ipaddr);
    peerEntry["port"] = BDE(Util::uitos((*i)->port));
    peerEntry["bitfield"] = BDE(Util::toHex((*i)->getBitfield(),
					    (*i)->getBitfieldLength()));
    peers << peerEntry;
  }
  entryDict["peers"] = peers;
}

static void gatherProgress
(BDE& entryDict, const SharedHandle<RequestGroup>& group, DownloadEngine* e)
{
  gatherProgressCommon(entryDict, group);

  BDE uriList = BDE::list();
  std::deque<std::string> uris;
  group->getURIs(uris);
  for(std::deque<std::string>::const_iterator i = uris.begin(); i != uris.end();
      ++i) {
    uriList << *i;
  }
  entryDict["uris"] = uriList;

  SharedHandle<BtContext> btctx =
    dynamic_pointer_cast<BtContext>(group->getDownloadContext());
  if(!btctx.isNull()) {
    gatherProgressBitTorrent(entryDict, btctx);

    SharedHandle<BtRegistry> btreg = e->getBtRegistry();
    SharedHandle<PeerStorage> ps =
      btreg->getPeerStorage(btctx->getInfoHashAsString());
    if(!ps.isNull()) {
      gatherPeer(entryDict, ps);
    }
  }
}

static void gatherStoppedDownload
(BDE& entryDict, const SharedHandle<DownloadResult>& ds)
{
  entryDict["gid"] = ds->gid;
  if(ds->result == DownloadResult::IN_PROGRESS) {
    entryDict["status"] = BDE("removed");
  } else if(ds->result == DownloadResult::FINISHED) {
    entryDict["status"] = BDE("complete");
  } else {
    entryDict["status"] = BDE("error");
  }
}

BDE TellStatusXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  const BDE& params = req._params;
  assert(params.isList());

  if(params.empty() || !params[0].isString()) {
    throw DlAbortEx("GID is not provided.");
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
	throw DlAbortEx
	  (StringFormat("No such download for GID#%d", gid).str());
      }
      gatherStoppedDownload(entryDict, ds);
    } else {
      entryDict["status"] = BDE("waiting");
      gatherProgress(entryDict, group, e);
    }
  } else {
    entryDict["status"] = BDE("active");
    gatherProgress(entryDict, group, e);
  }

  BDE resParams = BDE::list();
  resParams << entryDict;
  return resParams;
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
    entryDict["status"] = BDE("active");
    gatherProgressCommon(entryDict, *i);

    SharedHandle<BtContext> btctx =
      dynamic_pointer_cast<BtContext>((*i)->getDownloadContext());
    if(!btctx.isNull()) {
      gatherProgressBitTorrent(entryDict, btctx);
    }
    list << entryDict;
  }
  BDE resParams = BDE::list();
  resParams << list;
  return resParams;
}

BDE NoSuchMethodXmlRpcMethod::process
(const XmlRpcRequest& req, DownloadEngine* e)
{
  throw DlAbortEx
    (StringFormat("No such method: %s", req._methodName.c_str()).str());
}

} // namespace xmlrpc

} // namespace aria2
