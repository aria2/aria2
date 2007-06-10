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
#include "AbstractCommand.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "InitiateConnectionCommandFactory.h"
#include "Util.h"
#include "message.h"
#include "SleepCommand.h"
#include "prefs.h"
#include "DNSCache.h"
#include "FatalException.h"

AbstractCommand::AbstractCommand(int cuid,
				 const RequestHandle& req,
				 RequestGroup* requestGroup,
				 DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), req(req), _requestGroup(requestGroup), e(e), socket(s),
  checkSocketIsReadable(false), checkSocketIsWritable(false),
  nameResolverCheck(false) {
  
  setReadCheckSocket(socket);
  timeout = this->e->option->getAsInt(PREF_TIMEOUT);
  ++_requestGroup->numConnection;
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
  --_requestGroup->numConnection;
}

bool AbstractCommand::execute() {
  try {
    if(_requestGroup->getSegmentMan()->finished()) {
      logger->debug("CUID#%d - finished.", cuid);
      return true;
    }
    PeerStatHandle peerStat = _requestGroup->getSegmentMan()->getPeerStat(cuid);
    if(peerStat.get()) {
      if(peerStat->getStatus() == PeerStat::REQUEST_IDLE) {
	logger->info("CUID#%d - Request idle.", cuid);
	onAbort(0);
	req->resetUrl();
	tryReserved();
	return true;
      }
    }
    if(checkSocketIsReadable && readCheckTarget->isReadable(0) ||
       checkSocketIsWritable && writeCheckTarget->isWritable(0) ||
#ifdef ENABLE_ASYNC_DNS
       nameResolverCheck && nameResolveFinished() ||
#endif // ENABLE_ASYNC_DNS
       !checkSocketIsReadable && !checkSocketIsWritable && !nameResolverCheck) {
      checkPoint.reset();
      if(_requestGroup->getSegmentMan()->downloadStarted) {
	// TODO Segment::isNull(), Change method name, it is very confusing.
	if(segment->isNull()) {
	  segment = _requestGroup->getSegmentMan()->getSegment(cuid);
	  if(segment.isNull()) {
	    logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
	    return prepareForRetry(1);
	  }
	}
      }
      return executeInternal();
    } else {

      if(checkPoint.elapsed(timeout)) {
	throw new DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push_back(this);
      return false;
    }
  } catch(FatalException* err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
    onAbort(err);
    delete(err);
    req->resetUrl();
    _requestGroup->getSegmentMan()->errors++;
    return true;    
  } catch(DlAbortEx* err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
    onAbort(err);
    delete(err);
    req->resetUrl();
    _requestGroup->getSegmentMan()->errors++;
    tryReserved();
    return true;
  } catch(DlRetryEx* err) {
    logger->info(MSG_RESTARTING_DOWNLOAD, err, cuid, req->getUrl().c_str());
    req->addTryCount();
    bool isAbort = e->option->getAsInt(PREF_MAX_TRIES) != 0 &&
      req->getTryCount() >= e->option->getAsInt(PREF_MAX_TRIES);
    if(isAbort) {
      onAbort(err);
    }
    if(isAbort) {
      logger->info(MSG_MAX_TRY, cuid, req->getTryCount());
      logger->error(MSG_DOWNLOAD_ABORTED, err, cuid, req->getUrl().c_str());
      delete(err);
      _requestGroup->getSegmentMan()->errors++;
      tryReserved();
      return true;
    } else {
      delete(err);
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
  }
}

void AbstractCommand::tryReserved() {
  Commands commands = _requestGroup->createNextCommand(e, 1);
  e->addCommand(commands);
}

bool AbstractCommand::prepareForRetry(int wait) {
  _requestGroup->getSegmentMan()->cancelSegment(cuid);
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, _requestGroup, e);
  if(wait == 0) {
    e->commands.push_back(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, command, wait);
    e->commands.push_back(scom);
  }
  return true;
}

void AbstractCommand::onAbort(Exception* ex) {
  logger->debug(MSG_UNREGISTER_CUID, cuid);
  //_segmentMan->unregisterId(cuid);
  _requestGroup->getSegmentMan()->cancelSegment(cuid);
}

void AbstractCommand::disableReadCheckSocket() {
  if(checkSocketIsReadable) {
    e->deleteSocketForReadCheck(readCheckTarget, this);
    checkSocketIsReadable = false;
    readCheckTarget = SocketHandle();
  }  
}

void AbstractCommand::setReadCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableReadCheckSocket();
  } else {
    if(checkSocketIsReadable) {
      if(readCheckTarget != socket) {
	e->deleteSocketForReadCheck(readCheckTarget, this);
	e->addSocketForReadCheck(socket, this);
	readCheckTarget = socket;
      }
    } else {
      e->addSocketForReadCheck(socket, this);
      checkSocketIsReadable = true;
      readCheckTarget = socket;
    }
  }
}

void AbstractCommand::disableWriteCheckSocket() {
  if(checkSocketIsWritable) {
    e->deleteSocketForWriteCheck(writeCheckTarget, this);
    checkSocketIsWritable = false;
    writeCheckTarget = SocketHandle();
  }
}

void AbstractCommand::setWriteCheckSocket(const SocketHandle& socket) {
  if(!socket->isOpen()) {
    disableWriteCheckSocket();
  } else {
    if(checkSocketIsWritable) {
      if(writeCheckTarget != socket) {
	e->deleteSocketForWriteCheck(writeCheckTarget, this);
	e->addSocketForWriteCheck(socket, this);
	writeCheckTarget = socket;
      }
    } else {
      e->addSocketForWriteCheck(socket, this);
      checkSocketIsWritable = true;
      writeCheckTarget = socket;
    }
  }
}

bool AbstractCommand::resolveHostname(const string& hostname,
				      const NameResolverHandle& resolver) {
  string ipaddr = DNSCacheSingletonHolder::instance()->find(hostname);
  if(ipaddr.empty()) {
#ifdef ENABLE_ASYNC_DNS
    switch(resolver->getStatus()) {
    case NameResolver::STATUS_READY:
      logger->info("CUID#%d - Resolving hostname %s", cuid, hostname.c_str());
      resolver->resolve(hostname);
      setNameResolverCheck(resolver);
      return false;
    case NameResolver::STATUS_SUCCESS:
      logger->info("CUID#%d - Name resolution complete: %s -> %s", cuid,
		   hostname.c_str(), resolver->getAddrString().c_str());
      DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
      return true;
      break;
    case NameResolver::STATUS_ERROR:
      throw new DlAbortEx("CUID#%d - Name resolution for %s failed:%s", cuid,
			  hostname.c_str(),
			  resolver->getError().c_str());
    default:
      return false;
    }
#else
    logger->info("CUID#%d - Resolving hostname %s", cuid, hostname.c_str());
    resolver->resolve(hostname);
    logger->info("CUID#%d - Name resolution complete: %s -> %s", cuid,
		 hostname.c_str(), resolver->getAddrString().c_str());
    DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
    return true;
#endif // ENABLE_ASYNC_DNS
  } else {
    logger->info("CUID#%d - DNS cache hit: %s -> %s", cuid,
		 hostname.c_str(), ipaddr.c_str());
    resolver->setAddr(ipaddr);
    return true;
  }
}

#ifdef ENABLE_ASYNC_DNS
void AbstractCommand::setNameResolverCheck(const NameResolverHandle& resolver) {
  nameResolverCheck = true;
  e->addNameResolverCheck(resolver, this);
}

void AbstractCommand::disableNameResolverCheck(const NameResolverHandle& resolver) {
  nameResolverCheck = false;
  e->deleteNameResolverCheck(resolver, this);
}

bool AbstractCommand::nameResolveFinished() const {
  return false;
}
#endif // ENABLE_ASYNC_DNS
