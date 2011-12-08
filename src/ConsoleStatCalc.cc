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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <cstdio>
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
#include "util.h"
#include "DownloadContext.h"
#include "wallclock.h"
#include "FileEntry.h"
#include "console.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "PeerStorage.h"
# include "BtRegistry.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

std::string SizeFormatter::operator()(int64_t size) const
{
  return format(size);
}

namespace {
class AbbrevSizeFormatter:public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const
  {
    return util::abbrevSize(size);
  }
};
} // namespace

namespace {
class PlainSizeFormatter:public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const
  {
    return util::itos(size);
  }
};
} // namespace

namespace {
void printProgress
(std::ostream& o, const SharedHandle<RequestGroup>& rg, const DownloadEngine* e,
 const SizeFormatter& sizeFormatter)
{
  TransferStat stat = rg->calculateStat();
  int eta = 0;
  if(rg->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
    eta = (rg->getTotalLength()-rg->getCompletedLength())/stat.getDownloadSpeed();
  }

  o << "["
    << "#" << rg->getGID() << " ";

#ifdef ENABLE_BITTORRENT
  if(rg->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT) &&
     !bittorrent::getTorrentAttrs(rg->getDownloadContext())->metadata.empty() &&
     rg->downloadFinished()) {
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
        << sizeFormatter(rg->getCompletedLength())
        << "B"
        << "/"
        << sizeFormatter(rg->getTotalLength())
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
  const SharedHandle<BtObject>& btObj = e->getBtRegistry()->get(rg->getGID());
  if(btObj) {
    std::vector<SharedHandle<Peer> > peers;
    btObj->peerStorage->getActivePeers(peers);
    o << " " << "SEED:"
      << countSeeder(peers.begin(), peers.end());
  }
#endif // ENABLE_BITTORRENT

  if(!rg->downloadFinished()) {
    o << " "
      << "SPD:"
      << sizeFormatter(stat.getDownloadSpeed()) << "Bs";
  }
  if(stat.getSessionUploadLength() > 0) {
    o << " "
      << "UP:"
      << sizeFormatter(stat.getUploadSpeed()) << "Bs"
      << "(" << sizeFormatter(stat.getAllTimeUploadLength()) << "B)";
  }
  if(eta > 0) {
    o << " "
      << "ETA:"
      << util::secfmt(eta);
  }
  o << "]";
}
} // namespace

namespace {
class PrintSummary
{
private:
  size_t cols_;
  const DownloadEngine* e_;
  const SizeFormatter& sizeFormatter_;
public:
  PrintSummary
  (size_t cols, const DownloadEngine* e,
   const SizeFormatter& sizeFormatter):
    cols_(cols), e_(e), sizeFormatter_(sizeFormatter) {}

  void operator()(const SharedHandle<RequestGroup>& rg)
  {
    const char SEP_CHAR = '-';
    std::stringstream o;
    printProgress(o, rg, e_, sizeFormatter_);
    const std::vector<SharedHandle<FileEntry> >& fileEntries =
      rg->getDownloadContext()->getFileEntries();
    o << "\n"
      << "FILE: ";
    writeFilePath(fileEntries.begin(), fileEntries.end(),
                  o, rg->inMemoryDownload());
    o << "\n"
      << std::setfill(SEP_CHAR) << std::setw(cols_) << SEP_CHAR << "\n";
    global::cout()->write(o.str().c_str());
  }
};
} // namespace

namespace {
void printProgressSummary
(const std::deque<SharedHandle<RequestGroup> >& groups, size_t cols,
 const DownloadEngine* e,
 const SizeFormatter& sizeFormatter)
{
  const char SEP_CHAR = '=';
  time_t now;
  time(&now);
  std::stringstream o;
  o << " *** Download Progress Summary";
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
      o << " as of " << buf;
    }
  }
  o << " *** " << "\n"
    << std::setfill(SEP_CHAR) << std::setw(cols) << SEP_CHAR << "\n";
  global::cout()->write(o.str().c_str());
  std::for_each(groups.begin(), groups.end(),
                PrintSummary(cols, e, sizeFormatter));
}
} // namespace

ConsoleStatCalc::ConsoleStatCalc(time_t summaryInterval, bool humanReadable):
  summaryInterval_(summaryInterval),
  readoutVisibility_(true),
  truncate_(true)
{
  if(humanReadable) {
    sizeFormatter_.reset(new AbbrevSizeFormatter());
  } else {
    sizeFormatter_.reset(new PlainSizeFormatter());
  }
}

void
ConsoleStatCalc::calculateStat(const DownloadEngine* e)
{
  if(cp_.differenceInMillis(global::wallclock())+A2_DELTA_MILLIS < 1000) {
    return;
  }
  cp_ = global::wallclock();
  const SizeFormatter& sizeFormatter = *sizeFormatter_.get();

#ifdef __MINGW32__
  bool isTTY = true;
  // Windows terminal cannot handle at the end of line properly.
  unsigned short int cols = 79;
#else // !__MINGW32__
  bool isTTY = isatty(STDOUT_FILENO) == 1;
  unsigned short int cols = 80;
#endif // !__MINGW32__

  if(isTTY) {
#ifndef __MINGW32__
#ifdef HAVE_TERMIOS_H
    struct winsize size;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
      cols = size.ws_col;
    }
#endif // HAVE_TERMIOS_H
#endif // !__MINGW32__
    std::string line(cols, ' ');
    global::cout()->printf("\r%s\r", line.c_str());
  }
  std::ostringstream o;
  if(e->getRequestGroupMan()->countRequestGroup() > 0) {
    if((summaryInterval_ > 0) &&
       lastSummaryNotified_.differenceInMillis(global::wallclock())+
       A2_DELTA_MILLIS >= summaryInterval_*1000) {
      lastSummaryNotified_ = global::wallclock();
      printProgressSummary(e->getRequestGroupMan()->getRequestGroups(), cols, e,
                           sizeFormatter);
      global::cout()->write("\n");
      global::cout()->flush();
    }
  }
  if(!readoutVisibility_) {
    return;
  }
  if(e->getRequestGroupMan()->countRequestGroup() > 0) {
    SharedHandle<RequestGroup> firstRequestGroup =
      e->getRequestGroupMan()->getRequestGroup(0);

    printProgress(o, firstRequestGroup, e, sizeFormatter);

    if(e->getRequestGroupMan()->countRequestGroup() > 1) {
      o << "("
        << e->getRequestGroupMan()->countRequestGroup()-1
        << "more...)";
    }
  }

  if(e->getRequestGroupMan()->countRequestGroup() > 1 &&
     !e->getRequestGroupMan()->downloadFinished()) {
    TransferStat stat = e->getRequestGroupMan()->calculateStat();
    o << " "
      << "[TOTAL SPD:"
      << sizeFormatter(stat.getDownloadSpeed()) << "Bs" << "]";
  }

  {
    SharedHandle<FileAllocationEntry> entry =
      e->getFileAllocationMan()->getPickedEntry();
    if(entry) {
      o << " "
        << "[FileAlloc:"
        << "#" << entry->getRequestGroup()->getGID() << " "
        << sizeFormatter(entry->getCurrentLength())
        << "B"
        << "/"
        << sizeFormatter(entry->getTotalLength())
        << "B"
        << "(";
      if(entry->getTotalLength() > 0) {
        o << 100LL*entry->getCurrentLength()/entry->getTotalLength();
      } else {
        o << "--";
      }
      o << "%)"
        << "]";
      if(e->getFileAllocationMan()->hasNext()) {
        o << "("
          << e->getFileAllocationMan()->countEntryInQueue()
          << "waiting...)";
      }
    }
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<CheckIntegrityEntry> entry =
      e->getCheckIntegrityMan()->getPickedEntry();
    if(entry) {
      o << " "
        << "[Checksum:"
        << "#" << entry->getRequestGroup()->getGID() << " "
        << sizeFormatter(entry->getCurrentLength())
        << "B"
        << "/"
        << sizeFormatter(entry->getTotalLength())
        << "B"
        << "(";
      if(entry->getTotalLength() > 0) {
        o << 100LL*entry->getCurrentLength()/entry->getTotalLength();
      } else {
        o << "--";
      }
      o << "%)"
        << "]";
      if(e->getCheckIntegrityMan()->hasNext()) {
        o << "("
          << e->getCheckIntegrityMan()->countEntryInQueue()
          << "waiting...)";
      }
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  std::string readout = o.str();
  if(isTTY) {
    if(truncate_ && readout.size() > cols) {
      readout[cols] = '\0';
    }
    global::cout()->write(readout.c_str());
    global::cout()->flush();
  } else {
    global::cout()->write(readout.c_str());
    global::cout()->write("\n");
  }
}

} // namespace aria2
