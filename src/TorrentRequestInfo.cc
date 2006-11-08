/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "TorrentRequestInfo.h"
#include "DownloadEngineFactory.h"
#include "prefs.h"
#include "Util.h"
#include "BtRegistry.h"
#include "DefaultBtContext.h"

extern RequestInfo* requestInfo;
extern void setSignalHander(int signal, void (*handler)(int), int flags);
extern bool timeoutSpecified;
extern volatile sig_atomic_t btHaltRequested;

void torrentHandler(int signal) {
  btHaltRequested = 1;
}

RequestInfos TorrentRequestInfo::execute() {
  BtContextHandle btContext(new DefaultBtContext());
  btContext->load(torrentFile);
  
  if(op->get(PREF_SHOW_FILES) == V_TRUE) {
    showFileEntry(btContext);
    return RequestInfos();
  }
  if(!timeoutSpecified) {
    op->put(PREF_TIMEOUT, "180");
  }
  // set max_tries to 1. AnnounceList handles retries.
  op->put(PREF_MAX_TRIES, "1");
  SharedHandle<TorrentDownloadEngine>
    e(DownloadEngineFactory::newTorrentConsoleEngine(btContext,
						     op,
						     targetFiles));

  setSignalHander(SIGINT, torrentHandler, SA_RESETHAND);
  setSignalHander(SIGTERM, torrentHandler, SA_RESETHAND);
    
  try {
    e->run();
    if(PIECE_STORAGE(btContext)->downloadFinished()) {
      printDownloadCompeleteMessage();
    }
  } catch(Exception* ex) {
    logger->error("Exception caught", ex);
    fail = true;
    delete ex;
  }
  setSignalHander(SIGINT, SIG_DFL, 0);
  setSignalHander(SIGTERM, SIG_DFL, 0);
  
  return RequestInfos();
}

void TorrentRequestInfo::showFileEntry(const BtContextHandle& btContext)
{
  FileEntries fileEntries = btContext->getFileEntries();
  cout << _("Files:") << endl;
  cout << "idx|path/length" << endl;
  cout << "===+===========================================================================" << endl;
  int count = 1;
  for(FileEntries::const_iterator itr = fileEntries.begin();
      itr != fileEntries.end(); count++, itr++) {
    printf("%3d|%s\n   |%s Bytes\n", count,
	   (*itr)->getPath().c_str(),
	   Util::llitos((*itr)->getLength(), true).c_str());
    cout << "---+---------------------------------------------------------------------------" << endl;
  }
}
