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
#ifndef _D_TORRENT_CONSOLE_DOWNLOAD_ENGINE_H_
#define _D_TORRENT_CONSOLE_DOWNLOAD_ENGINE_H_

#include "TorrentDownloadEngine.h"

class TorrentConsoleDownloadEngine : public TorrentDownloadEngine {
private:
  struct timeval cp[2];
  long long int sessionDownloadLengthArray[2];
  long long int sessionUploadLengthArray[2];
  int currentCp;

  int downloadSpeed;
  int uploadSpeed;
  int lastElapsed;
  long long int selectedDownloadLengthDiff;
  long long int selectedTotalLength;
  // The time when startup
  struct timeval startup;
  // The number of bytes downloaded since startup
  long long int sessionDownloadLength;
  // The average speed(bytes per second) since startup
  int avgSpeed;
  // The estimated remaining time to complete the download.
  int eta;
  long long int downloadLength;
  long long int totalLength;

  void printStatistics();
  int calculateSpeed(long long int sessionLength, int elapsed);
protected:
  void initStatistics();
  void calculateStatistics();
  void onSelectiveDownloadingCompletes();
public:
  TorrentConsoleDownloadEngine();
  ~TorrentConsoleDownloadEngine();
};

#endif // _D_TORRENT_CONSOLE_DOWNLOAD_ENGINE_H_
