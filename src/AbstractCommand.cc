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

AbstractCommand::AbstractCommand(int cuid, Request* req, DownloadEngine* e, const Socket* s):
  Command(cuid), req(req), e(e), checkSocketIsReadable(false), checkSocketIsWritable(false) {

  if(s != NULL) {
    socket = new Socket(*s);
    setReadCheckSocket(socket);
  } else {
    socket = NULL;
  }
  timeout = this->e->option->getAsInt(PREF_TIMEOUT);
}

AbstractCommand::~AbstractCommand() {
  setReadCheckSocket(NULL);
  setWriteCheckSocket(NULL);
  if(socket != NULL) {
    delete(socket);
  }
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
      return true;
    } else {
      return prepareForRetry(e->option->getAsInt(PREF_RETRY_WAIT));
    }
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

void AbstractCommand::setReadCheckSocket(Socket* socket) {
  if(socket == NULL) {
    if(checkSocketIsReadable) {
      e->deleteSocketForReadCheck(readCheckTarget);
      checkSocketIsReadable = false;
      readCheckTarget = NULL;
    }
  } else {
    if(checkSocketIsReadable) {
      if(readCheckTarget != socket) {
	e->deleteSocketForReadCheck(readCheckTarget);
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

void AbstractCommand::setWriteCheckSocket(Socket* socket) {
  if(socket == NULL) {
    if(checkSocketIsWritable) {
      e->deleteSocketForWriteCheck(writeCheckTarget);
      checkSocketIsWritable = false;
      writeCheckTarget = NULL;
    }
  } else {
    if(checkSocketIsWritable) {
      if(writeCheckTarget != socket) {
	e->deleteSocketForWriteCheck(writeCheckTarget);
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
