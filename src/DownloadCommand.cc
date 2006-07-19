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
#include "DownloadCommand.h"
#include <sys/time.h>
#include "Util.h"
#include "DlRetryEx.h"
#include "HttpInitiateConnectionCommand.h"
#include "InitiateConnectionCommandFactory.h"
#include "message.h"

DownloadCommand::DownloadCommand(int cuid, Request* req, DownloadEngine* e,
				 const SocketHandle& s):
  AbstractCommand(cuid, req, e, s), lastSize(0) {
}

DownloadCommand::~DownloadCommand() {}

bool DownloadCommand::executeInternal(Segment seg) {
  TransferEncoding* te = NULL;
  if(transferEncoding.size()) {
    te = getTransferEncoding(transferEncoding);
    assert(te != NULL);
  }
  int bufSize = 4096;
  char buf[bufSize];
  socket->readData(buf, bufSize);
  if(te != NULL) {
    int infbufSize = 4096;
    char infbuf[infbufSize];
    te->inflate(infbuf, infbufSize, buf, bufSize);
    e->segmentMan->diskWriter->writeData(infbuf, infbufSize, seg.sp+seg.ds);
    seg.ds += infbufSize;
  } else {
    e->segmentMan->diskWriter->writeData(buf, bufSize, seg.sp+seg.ds);
    seg.ds += bufSize;
  }
  // calculate downloading speed
  int diff = sw.difference();
  if(diff >= 1) {
    seg.speed = (int)((seg.ds-lastSize)/(diff*1.0));
    sw.reset();
    lastSize = seg.ds;
  }
  if(e->segmentMan->totalSize != 0 && bufSize == 0) {
    throw new DlRetryEx(EX_GOT_EOF);
  }
  if(te != NULL && te->finished()
     || te == NULL && seg.ds >= seg.ep-seg.sp+1
     || bufSize == 0) {
    if(te != NULL) te->end();
    logger->info(MSG_DOWNLOAD_COMPLETED, cuid);
    seg.ds = seg.ep-seg.sp+1;
    seg.finish = true;
    e->segmentMan->updateSegment(seg);
    // this unit is going to download another segment.
    return prepareForNextSegment();
  } else {
    e->segmentMan->updateSegment(seg);
    e->commands.push_back(this);
    return false;
  }
  
}

bool DownloadCommand::prepareForRetry(int wait) {
  //req->resetUrl();
  return AbstractCommand::prepareForRetry(wait);
}

bool DownloadCommand::prepareForNextSegment() {
  req->resetUrl();
  if(!e->segmentMan->finished()) {
    return AbstractCommand::prepareForRetry(0);
  } else {
    return true;
  }
}
