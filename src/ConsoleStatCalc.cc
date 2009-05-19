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
#include "ConsoleStatCalc.h"

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif // HAVE_TERMIOS_H
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif // HAVE_SYS_IOCTL_H
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iterator>

#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "FileAllocationMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityMan.h"
#include "CheckIntegrityEntry.h"
#include "Util.h"
#ifdef ENABLE_BITTORRENT
# include "BtContext.h"
# include "Peer.h"
# include "PeerStorage.h"
# include "BtRegistry.h"
# include "BtProgressInfoFile.h"
# include "BtRuntime.h"
# include "BtAnnounce.h"
# include "PieceStorage.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

static void printProgress
(std::ostream& o, const SharedHandle<RequestGroup>& rg, const DownloadEngine* e)
{
  TransferStat stat = rg->calculateStat();
  unsigned int eta = 0;
  if(rg->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
    eta = (rg->getTotalLength()-rg->getCompletedLength())/stat.getDownloadSpeed();
  }

  o << "["
    << "#" << rg->getGID() << " ";

#ifdef ENABLE_BITTORRENT
  SharedHandle<BtContext> btctx =
    dynamic_pointer_cast<BtContext>(rg->getDownloadContext());

  if(!btctx.isNull() && rg->downloadFinished()) {
    o << "SEEDING" << "(" << "ratio:";
    if(rg->getCompletedLength() > 0) {
      o << std::fixed << std::setprecision(1)
	<< ((stat.getAllTimeUploadLength()*10)/rg->getCompletedLength())/10.0;
    } else {
      o << "--";
    }
    o << ")";
  } else
#endif // ENABLE_BITTORRENT
    {
      o << "SIZE:"
	<< Util::abbrevSize(rg->getCompletedLength())
	<< "B"
	<< "/"
	<< Util::abbrevSize(rg->getTotalLength())
	<< "B";
      if(rg->getTotalLength() > 0) {
	o << "("
	  << 100*rg->getCompletedLength()/rg->getTotalLength()
	  << "%)";
      }
    }
  o << " "
    << "CN:"
    << rg->getNumConnection();
#ifdef ENABLE_BITTORRENT
  if(!btctx.isNull()) {
    SharedHandle<PeerStorage> ps =
      e->getBtRegistry()->getPeerStorage(btctx->getInfoHashAsString());
    std::deque<SharedHandle<Peer> > peers;
    ps->getActivePeers(peers);
    o << " " << "SEED:"
      << std::count_if(peers.begin(), peers.end(), mem_fun_sh(&Peer::isSeeder));
  }
#endif // ENABLE_BITTORRENT

  if(!rg->downloadFinished()) {
    o << " "
      << "SPD:"
      << Util::abbrevSize(stat.getDownloadSpeed()) << "Bs";
  }
  if(stat.getSessionUploadLength() > 0) {
    o << " "
      << "UP:"
      << Util::abbrevSize(stat.getUploadSpeed()) << "Bs"
      << "(" << Util::abbrevSize(stat.getAllTimeUploadLength()) << "B)";
  }
  if(eta > 0) {
    o << " "
      << "ETA:"
      << Util::secfmt(eta);
  }
  o << "]";
}

class PrintSummary
{
private:
  size_t _cols;
  const DownloadEngine* _e;
public:
  PrintSummary(size_t cols, const DownloadEngine* e):_cols(cols), _e(e) {}

  void operator()(const SharedHandle<RequestGroup>& rg)
  {
    const char SEP_CHAR = '-';
    printProgress(std::cout, rg, _e);
    std::cout << "\n"
	      << "FILE: " << rg->getFilePath() << "\n"
	      << std::setfill(SEP_CHAR) << std::setw(_cols) << SEP_CHAR << "\n";
  }
};

static void printProgressSummary
(const std::deque<SharedHandle<RequestGroup> >& groups, size_t cols,
 const DownloadEngine* e)
{
  const char SEP_CHAR = '=';
  time_t now;
  time(&now);
  std::cout << " *** Download Progress Summary";
  {
    time_t now;
    struct tm* staticNowtmPtr;
    char buf[26];
    if(time(&now) != (time_t)-1 && (staticNowtmPtr = localtime(&now)) != 0 &&
       asctime_r(staticNowtmPtr, buf) != 0) {
      char* lfptr = strchr(buf, '\n');
      if(lfptr) {
	*lfptr = '\0';
      }
      std::cout << " as of " << buf;
    }
  }
  std::cout << " *** " << "\n"
	    << std::setfill(SEP_CHAR) << std::setw(cols) << SEP_CHAR << "\n";
  std::for_each(groups.begin(), groups.end(), PrintSummary(cols, e));
}

ConsoleStatCalc::ConsoleStatCalc(time_t summaryInterval):
  _summaryInterval(summaryInterval)
{}

void
ConsoleStatCalc::calculateStat(const DownloadEngine* e)
{
  if(!_cp.elapsed(1)) {
    return;
  }
  _cp.reset();

  bool isTTY = isatty(STDOUT_FILENO) == 1;
  unsigned short int cols = 80;
  if(isTTY) {
#ifdef HAVE_TERMIOS_H
    struct winsize size;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
      cols = size.ws_col;
    }
#endif // HAVE_TERMIOS_H
    std::cout << '\r' << std::setfill(' ') << std::setw(cols) << ' ' << '\r';
  }
  std::ostringstream o;
  if(e->_requestGroupMan->countRequestGroup() > 0) {
    if((_summaryInterval > 0) &&
       _lastSummaryNotified.elapsed(_summaryInterval)) {
      _lastSummaryNotified.reset();
      printProgressSummary(e->_requestGroupMan->getRequestGroups(), cols, e);
      std::cout << "\n";
    }

    RequestGroupHandle firstRequestGroup = e->_requestGroupMan->getRequestGroup(0);

    printProgress(o, firstRequestGroup, e);

    if(e->_requestGroupMan->countRequestGroup() > 1) {
      o << "("
	<< e->_requestGroupMan->countRequestGroup()-1
	<< "more...)";
    }
  }

  if(e->_requestGroupMan->countRequestGroup() > 1 &&
     !e->_requestGroupMan->downloadFinished()) {
    TransferStat stat = e->_requestGroupMan->calculateStat();
    o << " "
      << "[TOTAL SPD:"
      << Util::abbrevSize(stat.getDownloadSpeed()) << "Bs" << "]";
  }

  {
    SharedHandle<FileAllocationEntry> entry=e->_fileAllocationMan->getPickedEntry();
    if(!entry.isNull()) {
      o << " "
	<< "[FileAlloc:"
	<< "#" << entry->getRequestGroup()->getGID() << " "
	<< Util::abbrevSize(entry->getCurrentLength())
	<< "B"
	<< "/"
	<< Util::abbrevSize(entry->getTotalLength())
	<< "B"
	<< "(";
      if(entry->getTotalLength() > 0) {
	o << 100*entry->getCurrentLength()/entry->getTotalLength();
      } else {
	o << "--";
      }
      o << "%)"
	<< "]";
      if(e->_fileAllocationMan->hasNext()) {
	o << "("
	  << e->_fileAllocationMan->countEntryInQueue()
	  << "waiting...)";
      }
    }
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    CheckIntegrityEntryHandle entry = e->_checkIntegrityMan->getPickedEntry();
    if(!entry.isNull()) {
      o << " "
	<< "[Checksum:"
	<< "#" << entry->getRequestGroup()->getGID() << " "
	<< Util::abbrevSize(entry->getCurrentLength())
	<< "B"
	<< "/"
	<< Util::abbrevSize(entry->getTotalLength())
	<< "B"
	<< "("
	<< 100*entry->getCurrentLength()/entry->getTotalLength()
	<< "%)"
	<< "]";
      if(e->_checkIntegrityMan->hasNext()) {
	o << "("
	  << e->_checkIntegrityMan->countEntryInQueue()
	  << "waiting...)";
      }
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  std::string readout = o.str();
  if(isTTY) {
    std::string::iterator last = readout.begin();
    if(readout.size() > cols) {
      std::advance(last, cols);
    } else {
      last = readout.end();
    }
    std::copy(readout.begin(), last, std::ostream_iterator<char>(std::cout));
    std::cout << std::flush;
  } else {
    std::copy(readout.begin(), readout.end(), std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
  }
}

} // namespace aria2
