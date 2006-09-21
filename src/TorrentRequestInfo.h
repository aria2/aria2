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
#ifndef _D_TORRENT_REQUEST_INFO_H_
#define _D_TORRENT_REQUEST_INFO_H_

#include "RequestInfo.h"
#include "TorrentDownloadEngine.h"

class TorrentRequestInfo : public RequestInfo {
private:
  string torrentFile;
  TorrentDownloadEngine* e;
  Strings targetFiles;

  void showFileEntry();
public:
  TorrentRequestInfo(const string& torrentFile, Option* op):
    RequestInfo(op),
    torrentFile(torrentFile),
    e(0) {}
  virtual ~TorrentRequestInfo() {}

  virtual RequestInfo* execute();

  void setTargetFiles(const Strings& targetFiles) {
    this->targetFiles = targetFiles;
  }
  virtual DownloadEngine* getDownloadEngine() { return e; }

};

#endif // _D_TORRENT_REQUEST_INFO_H_
