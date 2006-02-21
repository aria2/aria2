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
#include <sys/time.h>
#include "Util.h"
#include "message.h"
#include "SleepCommand.h"

#define TIMEOUT_SEC 60

AbstractCommand::AbstractCommand(int cuid, Request* req, DownloadEngine* e, Socket* s):
  Command(cuid), req(req), e(e), checkSocketIsReadable(false), checkSocketIsWritable(false) {

  if(s != NULL) {
    socket = new Socket(*s);
    setReadCheckSocket(socket);
  } else {
    socket = NULL;
  }
  this->checkPoint.tv_sec = 0;
  this->checkPoint.tv_usec = 0;
}

AbstractCommand::~AbstractCommand() {
  setReadCheckSocket(NULL);
  setWriteCheckSocket(NULL);
  if(socket != NULL) {
    delete(socket);
  }
}

void AbstractCommand::updateCheckPoint() {
  gettimeofday(&checkPoint, NULL);
}

bool AbstractCommand::isTimeoutDetected() {
  struct timeval now;
  gettimeofday(&now, NULL);
  if(checkPoint.tv_sec == 0 && checkPoint.tv_usec == 0) {
    checkPoint = now;
    return false;
  } else {
    long long int elapsed = Util::difftv(now, checkPoint);
    if(elapsed >= TIMEOUT_SEC*1000000) {
      return true;
    } else {
      return false;
    }
  }
}

bool AbstractCommand::execute() {
  try {
    if(checkSocketIsReadable && !readCheckTarget->isReadable(0)
       || checkSocketIsWritable && !writeCheckTarget->isWritable(0)) {
      if(isTimeoutDetected()) {
	throw new DlRetryEx(EX_TIME_OUT);
      }
      e->commands.push(this);
      return false;
    }
    updateCheckPoint();
    Segment seg = { 0, 0, 0, false };
    if(e->segmentMan->downloadStarted) {
      // get segment information in order to set Range header.
      if(!e->segmentMan->getSegment(seg, cuid)) {
	// no segment available
	e->logger->info(MSG_NO_SEGMENT_AVAILABLE, cuid);
	return true;
      }
    }
    return executeInternal(seg);
  } catch(DlAbortEx* err) {
    e->logger->error(MSG_DOWNLOAD_ABORTED, err, cuid);
    onAbort(err);
    delete(err);
    req->resetUrl();
    return true;
  } catch(DlRetryEx* err) {
    e->logger->error(MSG_RESTARTING_DOWNLOAD, err, cuid);
    delete(err);
    //req->resetUrl();
    req->addTryCount();
    if(req->noMoreTry()) {
      e->logger->error(MSG_MAX_RETRY, cuid);
      return true;
    } else {
      return prepareForRetry(e->option->getAsInt("retry_wait"));
    }
  }
}

bool AbstractCommand::prepareForRetry(int wait) {
  Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e);
  if(wait == 0) {
    e->commands.push(command);
  } else {
    SleepCommand* scom = new SleepCommand(cuid, e, command, wait);
    e->commands.push(scom);
  }
  return true;
}

void AbstractCommand::onAbort(Exception* e) {
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
	e->addSocketForReadCheck(socket);
	readCheckTarget = socket;
      }
    } else {
      e->addSocketForReadCheck(socket);
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
	e->addSocketForWriteCheck(socket);
	writeCheckTarget = socket;
      }
    } else {
      e->addSocketForWriteCheck(socket);
      checkSocketIsWritable = true;
      writeCheckTarget = socket;
    }
  }
}
