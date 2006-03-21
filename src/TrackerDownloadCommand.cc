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
#include "TrackerDownloadCommand.h"
#include "TrackerUpdateCommand.h"
#include "ChunkedEncoding.h"
#include "DlRetryEx.h"
#include "message.h"
#include "MetaFileUtil.h"
#include "DlAbortEx.h"

TrackerDownloadCommand::TrackerDownloadCommand(int cuid, Request* req,
					       TorrentDownloadEngine* e,
					       Socket* s)
  :AbstractCommand(cuid, req, e, s), len(0) {
  resSize = 256;
  res = new char[resSize];
  ChunkedEncoding* ce = new ChunkedEncoding();
  transferEncodings["chunked"] = ce;
}

TrackerDownloadCommand::~TrackerDownloadCommand() {
  delete [] res;
  for(map<string, TransferEncoding*>::iterator itr = transferEncodings.begin(); itr != transferEncodings.end(); itr++) {
    delete((*itr).second);
  }
}

bool TrackerDownloadCommand::executeInternal(Segment seg) {
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
    if(len+infbufSize >= resSize) {
      expandBuffer(len+infbufSize);
    }
    memcpy(res+len, infbuf, infbufSize);
    len += infbufSize;
  } else {
    if(len+bufSize >= resSize) {
      expandBuffer(len+bufSize);
    }
    memcpy(res+len, buf, bufSize);
    len += bufSize;
  }
  if(e->segmentMan->totalSize != 0 && bufSize == 0) {
    throw new DlRetryEx(EX_GOT_EOF);
  }
  if(te != NULL && te->finished()
     || te == NULL && len == e->segmentMan->totalSize
     || bufSize == 0) {
    if(te != NULL) te->end();
    e->logger->info(MSG_DOWNLOAD_COMPLETED, cuid);
    MetaEntry* entry = MetaFileUtil::bdecoding(res, len);
    e->commands.push(new TrackerUpdateCommand(cuid, req, (TorrentDownloadEngine*)e, entry));
    return true;
  } else {
    e->commands.push(this);
    return false;
  }
}

TransferEncoding* TrackerDownloadCommand::getTransferEncoding(string name) {
  return transferEncodings[name];
}

void TrackerDownloadCommand::expandBuffer(int newSize) {
  char* newbuf = new char[newSize];
  memcpy(newbuf, res, len);
  delete [] res;
  res = newbuf;
  resSize = newSize;
}
