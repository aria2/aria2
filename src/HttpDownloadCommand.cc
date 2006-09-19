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
#include "HttpDownloadCommand.h"
#include "DlRetryEx.h"
#include "HttpRequestCommand.h"
#include "Util.h"
#include "ChunkedEncoding.h"
#include "message.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

HttpDownloadCommand::HttpDownloadCommand(int cuid, Request* req,
					 DownloadEngine* e,
					 const SocketHandle& socket)
  :DownloadCommand(cuid, req, e, socket)
{
  ChunkedEncoding* ce = new ChunkedEncoding();
  transferEncodings["chunked"] = ce;
}

HttpDownloadCommand::~HttpDownloadCommand() {
  for(map<string, TransferEncoding*>::iterator itr = transferEncodings.begin(); itr != transferEncodings.end(); itr++) {
    delete((*itr).second);
  }
}

TransferEncoding* HttpDownloadCommand::getTransferEncoding(const string& name) {
  return transferEncodings[name];
}

bool HttpDownloadCommand::prepareForNextSegment(const Segment& currentSegment) {
  if(e->segmentMan->finished()) {
    return true;
  } else {
    if(req->isKeepAlive()) {
      Command* command = new HttpRequestCommand(cuid, req, e, socket);
      e->commands.push_back(command);
      return true;
    } else {
      return DownloadCommand::prepareForNextSegment(currentSegment);
    }
  }
}
