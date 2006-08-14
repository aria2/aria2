/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

AbstractCommand::AbstractCommand(int cuid, Request* req, DownloadEngine* e,
				 const SocketHandle& s):
  Command(cuid), req(req), e(e), socket(s),
  checkSocketIsReadable(false), checkSocketIsWritable(false) {
  
  setReadCheckSocket(socket);
  timeout = this->e->option->getAsInt(PREF_TIMEOUT);
}

AbstractCommand::~AbstractCommand() {
  disableReadCheckSocket();
  disableWriteCheckSocket();
}

bool AbstractCommand::execute() {
  try {
    if(checkSocketIsReadable && readCheckTarget->isReadable(0) ||
       checkSocketIsWritable && writeCheckTarget->isWritable(0) ||
       !checkSocketIsReadable && !checkSocketIsWritable) {
      checkPoint.reset();
      Segment seg = { 0, 0, 0, false };
      if(e->segmentMan->downloadStarted) {
	// get segment information in order to set Range header.
	if(!e->segmentMan->getSegment(seg, cuid)) {
	  // no segment available
	  logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
	  return true;
	}
      }
      return executeInternal(seg);
    } else {
      if(checkPoint.elapsed(timeout)) {
	throw new DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push_back(this);
      return false;
    }
  } catch(DlAbortEx* err) {
    logger->error(MSG_DOWNLOAD_ABORTED, err, cuid);
    onAbort(err);
    delete(err);
    req->resetUrl();
    e->segmentMan->errors++;
    tryReserved();
    return true;
  } catch(DlRetryEx* err) {
    logger->error(MSG_RESTARTING_DOWNLOAD, err, cuid);
    req->addTryCount();
    bool isAbort = e->option->getAsInt(PREF_MAX_TRIES) != 0 &&
      req->getTryCount() >= e->option->getAsInt(PREF_MAX_TRIES);
    if(isAbort) {
      onAbort(err);
    }
    delete(err);
    if(isAbort) {
      logger->error(MSG_MAX_TRY, cuid, req->getTryCount());
      e->segmentMan->errors++;
      tryReserved();
      return true;
    } else {
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
  }
}

void AbstractCommand::tryReserved() {
  if(!e->segmentMan->reserved.empty()) {
    Request* req = e->segmentMan->reserved.front();
    e->segmentMan->reserved.pop_front();
    Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e);
    e->commands.push_back(command);
  }
}

bool AbstractCommand::prepareForRetry(int wait) {
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e);
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
  e->segmentMan->unregisterId(cuid);
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

#ifdef HAVE_LIBARES
void AbstractCommand::setNameResolverCheck(const NameResolverHandle& resolver) {
  e->addNameResolverCheck(resolver, this);
}

void AbstractCommand::disableNameResolverCheck(const NameResolverHandle& resolver) {
  e->deleteNameResolverCheck(resolver, this);
}

bool AbstractCommand::resolveHostname(const string& hostname,
				      const NameResolverHandle& resolver) {
  switch(resolver->getStatus()) {
  case NameResolver::STATUS_READY:
    logger->info("CUID#%d - Resolving hostname %s", cuid, hostname.c_str());
    resolver->resolve(hostname);
    setNameResolverCheck(resolver);
    return false;
  case NameResolver::STATUS_SUCCESS:
    logger->info("CUID#%d - Name resolution complete: %s -> %s", cuid,
		 hostname.c_str(), resolver->getAddrString().c_str());
    return true;
    break;
  case NameResolver::STATUS_ERROR:
    throw new DlRetryEx("CUID#%d - Name resolution failed:%s", cuid,
			resolver->getError().c_str());
  default:
    return false;
  }
}
#endif // HAVE_LIBARES
