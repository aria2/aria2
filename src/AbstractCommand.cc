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

#define TIMEOUT_SEC 5

AbstractCommand::AbstractCommand(int cuid, Request* req, DownloadEngine* e, Socket* s):
  Command(cuid), req(req), e(e), checkSocketIsReadable(false), checkSocketIsWritable(false) {
  if(s != NULL) {
    socket = new Socket(*s);
    e->addSocketForReadCheck(socket);
  } else {
    socket = NULL;
  }
  this->checkPoint.tv_sec = 0;
  this->checkPoint.tv_usec = 0;
}

AbstractCommand::~AbstractCommand() {
  e->deleteSocketForReadCheck(socket);
  e->deleteSocketForWriteCheck(socket);
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
    if(checkSocketIsReadable && !socket->isReadable(0)
       || checkSocketIsWritable && !socket->isWritable(0)) {
      if(isTimeoutDetected()) {
	throw new DlRetryEx(EX_TIME_OUT);
      }
      updateCheckPoint();
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
    e->logger->error(MSG_DOWNLOAD_ABORTED, err);
    onError(err);
    delete(err);
    req->resetUrl();
    return true;
  } catch(DlRetryEx* err) {
    e->logger->error(MSG_RESTARTING_DOWNLOAD, err);
    onError(err);
    delete(err);
    req->resetUrl();
    req->addRetryCount();
    if(req->noMoreRetry()) {
      e->logger->error(MSG_MAX_RETRY);
      return true;
    } else {
      return prepareForRetry();
    }
  }
}

bool AbstractCommand::prepareForRetry() {
  e->commands.push(InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e));
  return true;
}

void AbstractCommand::onError(Exception* e) {
}
