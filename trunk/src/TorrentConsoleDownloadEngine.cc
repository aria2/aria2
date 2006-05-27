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
#include "TorrentConsoleDownloadEngine.h"
#include "Util.h"

TorrentConsoleDownloadEngine::TorrentConsoleDownloadEngine() {}

TorrentConsoleDownloadEngine::~TorrentConsoleDownloadEngine() {}

void TorrentConsoleDownloadEngine::onSelectiveDownloadingCompletes() {
  printf("\nDownload of selected files has completed.\n");
  fflush(stdout);
}

void TorrentConsoleDownloadEngine::sendStatistics() {
  printf("\r                                                                             ");
  printf("\r");
  if(torrentMan->downloadComplete()) {
    printf("Download Completed ");
  } else {
    printf("%s/%sB %d%% %s D:%.2f",
	   Util::llitos(downloadLength, true).c_str(),
	   Util::llitos(totalLength, true).c_str(),
	   (totalLength == 0 ?
	    0 : (int)((downloadLength*100)/totalLength)),
	   avgSpeed == 0 ? "-" : Util::secfmt(eta).c_str(),
	   downloadSpeed/1024.0);
  }
  printf(" U:%.2f(%s) %d peers",
	 uploadSpeed/1024.0,
	 Util::llitos(torrentMan->getUploadLength(), true).c_str(),
	 torrentMan->connections);
  fflush(stdout);	 
}
