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
#include "TorrentRequestInfo.h"
#include "DownloadEngineFactory.h"
#include "prefs.h"
#include "Util.h"

extern RequestInfo* requestInfo;
extern void setSignalHander(int signal, void (*handler)(int), int flags);

void torrentHandler(int signal) {
  ((TorrentDownloadEngine*)requestInfo->getDownloadEngine())->
    torrentMan->setHalt(true);
}

RequestInfo* TorrentRequestInfo::execute() {
  if(op->get(PREF_SHOW_FILES) == V_TRUE) {
    showFileEntry();
    return 0;
  }
  e = DownloadEngineFactory::newTorrentConsoleEngine(op,
						     torrentFile,
						     targetFiles);
  setSignalHander(SIGINT, torrentHandler, SA_RESETHAND);
  setSignalHander(SIGTERM, torrentHandler, SA_RESETHAND);
    
  try {
    e->run();
    if(e->torrentMan->downloadComplete()) {
      printDownloadCompeleteMessage();
    }
  } catch(Exception* e) {
    logger->error("Exception caught", e);
    delete e;
    fail = true;
  }
  setSignalHander(SIGINT, SIG_DFL, 0);
  setSignalHander(SIGTERM, SIG_DFL, 0);
  delete e;
  
  return 0;
}

// TODO should be const TorrentMan* torrentMan
void TorrentRequestInfo::showFileEntry()
{
  TorrentMan torrentMan;
  torrentMan.option = op;

  FileEntries fileEntries =
    torrentMan.readFileEntryFromMetaInfoFile(torrentFile);
  cout << _("Files:") << endl;
  cout << "idx|path/length" << endl;
  cout << "===+===========================================================================" << endl;
  int count = 1;
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); count++, itr++) {
    printf("%3d|%s\n   |%s Bytes\n", count, itr->path.c_str(),
	   Util::llitos(itr->length, true).c_str());
    cout << "---+---------------------------------------------------------------------------" << endl;
  }
}
