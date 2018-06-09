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
#  include <termios.h>
#endif // HAVE_TERMIOS_H
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
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
#include "ColorizedStream.h"
#include "Option.h"

#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#  include "PeerStorage.h"
#  include "BtRegistry.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

std::string SizeFormatter::operator()(int64_t size) const
{
  return format(size);
}

namespace {
class AbbrevSizeFormatter : public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const CXX11_OVERRIDE
  {
    return util::abbrevSize(size);
  }
};
} // namespace

namespace {
class PlainSizeFormatter : public SizeFormatter {
protected:
  virtual std::string format(int64_t size) const CXX11_OVERRIDE
  {
    return util::itos(size);
  }
};
} // namespace

namespace {
void printSizeProgress(ColorizedStream& o,
                       const std::shared_ptr<RequestGroup>& rg,
                       const TransferStat& stat,
                       const SizeFormatter& sizeFormatter)
{
#ifdef ENABLE_BITTORRENT
  if (rg->isSeeder()) {
    o << "SEED(";
    if (rg->getCompletedLength() > 0) {
      std::streamsize oldprec = o.precision();
      o << std::fixed << std::setprecision(1)
        << ((stat.allTimeUploadLength * 10) / rg->getCompletedLength()) / 10.0
        << std::setprecision(oldprec) << std::resetiosflags(std::ios::fixed);
    }
    else {
      o << "--";
    }
    o << ")";
  }
  else
#endif // ENABLE_BITTORRENT
  {
    o << sizeFormatter(rg->getCompletedLength()) << "B/"
      << sizeFormatter(rg->getTotalLength()) << "B";
    if (rg->getTotalLength() > 0) {
      o << colors::cyan << "("
        << 100 * rg->getCompletedLength() / rg->getTotalLength() << "%)";
      o << colors::clear;
    }
  }
}
} // namespace

namespace {
void printProgressCompact(ColorizedStream& o, const DownloadEngine* e,
                          const SizeFormatter& sizeFormatter)
{
  if (!e->getRequestGroupMan()->downloadFinished()) {
    NetStat& netstat = e->getRequestGroupMan()->getNetStat();
    int dl = netstat.calculateDownloadSpeed();
    int ul = netstat.calculateUploadSpeed();
    o << colors::magenta << "[" << colors::clear << "DL:" << colors::green
      << sizeFormatter(dl) << "B" << colors::clear;
    if (ul) {
      o << " UL:" << colors::cyan << sizeFormatter(ul) << "B" << colors::clear;
    }
    o << colors::magenta << "]" << colors::clear;
  }

  const RequestGroupList& groups = e->getRequestGroupMan()->getRequestGroups();
  size_t cnt = 0;
  const size_t MAX_ITEM = 5;
  for (auto i = groups.begin(), eoi = groups.end(); i != eoi && cnt < MAX_ITEM;
       ++i, ++cnt) {
    const std::shared_ptr<RequestGroup>& rg = *i;
    TransferStat stat = rg->calculateStat();
    o << colors::magenta << "[" << colors::clear << "#"
      << GroupId::toAbbrevHex(rg->getGID()) << " ";
    printSizeProgress(o, rg, stat, sizeFormatter);
    o << colors::magenta << "]" << colors::clear;
  }
  if (cnt < groups.size()) {
    o << "(+" << groups.size() - cnt << ")";
  }
}
} // namespace

namespace {
void printProgress(ColorizedStream& o, const std::shared_ptr<RequestGroup>& rg,
                   const DownloadEngine* e, const SizeFormatter& sizeFormatter)
{
  TransferStat stat = rg->calculateStat();
  int eta = 0;
  if (rg->getTotalLength() > 0 && stat.downloadSpeed > 0) {
    eta =
        (rg->getTotalLength() - rg->getCompletedLength()) / stat.downloadSpeed;
  }
  o << colors::magenta << "[" << colors::clear << "#"
    << GroupId::toAbbrevHex(rg->getGID()) << " ";
  printSizeProgress(o, rg, stat, sizeFormatter);
  o << " CN:" << rg->getNumConnection();
#ifdef ENABLE_BITTORRENT
  auto btObj = e->getBtRegistry()->get(rg->getGID());
  if (btObj) {
    const PeerSet& peers = btObj->peerStorage->getUsedPeers();
    o << " SD:" << countSeeder(peers.begin(), peers.end());
  }
#endif // ENABLE_BITTORRENT

  if (!rg->downloadFinished()) {
    o << " DL:" << colors::green << sizeFormatter(stat.downloadSpeed) << "B"
      << colors::clear;
  }
  if (stat.sessionUploadLength > 0) {
    o << " UL:" << colors::cyan << sizeFormatter(stat.uploadSpeed) << "B"
      << colors::clear;
    o << "(" << sizeFormatter(stat.allTimeUploadLength) << "B)";
  }
  if (eta > 0) {
    o << " ETA:" << colors::yellow << util::secfmt(eta) << colors::clear;
  }
  o << colors::magenta << "]" << colors::clear;
}
} // namespace

namespace {
class PrintSummary {
private:
  size_t cols_;
  const DownloadEngine* e_;
  const SizeFormatter& sizeFormatter_;

public:
  PrintSummary(size_t cols, const DownloadEngine* e,
               const SizeFormatter& sizeFormatter)
      : cols_(cols), e_(e), sizeFormatter_(sizeFormatter)
  {
  }

  void operator()(const RequestGroupList::value_type& rg)
  {
    const char SEP_CHAR = '-';
    ColorizedStream o;
    printProgress(o, rg, e_, sizeFormatter_);
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
        rg->getDownloadContext()->getFileEntries();
    o << "\nFILE: ";
    writeFilePath(fileEntries.begin(), fileEntries.end(), o,
                  rg->inMemoryDownload());
    o << "\n" << std::setfill(SEP_CHAR) << std::setw(cols_) << SEP_CHAR << "\n";
    auto str = o.str(false);
    global::cout()->write(str.c_str());
  }
};
} // namespace

namespace {
void printProgressSummary(const RequestGroupList& groups, size_t cols,
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
    if (time(&now) != (time_t)-1 &&
        (staticNowtmPtr = localtime(&now)) != nullptr &&
        asctime_r(staticNowtmPtr, buf) != nullptr) {
      char* lfptr = strchr(buf, '\n');
      if (lfptr) {
        *lfptr = '\0';
      }
      o << " as of " << buf;
    }
  }
  o << " *** \n"
    << std::setfill(SEP_CHAR) << std::setw(cols) << SEP_CHAR << "\n";
  global::cout()->write(o.str().c_str());
  std::for_each(groups.begin(), groups.end(),
                PrintSummary(cols, e, sizeFormatter));
}
} // namespace

ConsoleStatCalc::ConsoleStatCalc(std::chrono::seconds summaryInterval,
                                 bool colorOutput, bool humanReadable)
    : summaryInterval_(std::move(summaryInterval)),
      readoutVisibility_(true),
      truncate_(true),
#ifdef __MINGW32__
      isTTY_(true),
#else  // !__MINGW32__
      isTTY_(isatty(STDOUT_FILENO) == 1),
#endif // !__MINGW32__
      colorOutput_(colorOutput)
{
  if (humanReadable) {
    sizeFormatter_ = make_unique<AbbrevSizeFormatter>();
  }
  else {
    sizeFormatter_ = make_unique<PlainSizeFormatter>();
  }
}

void ConsoleStatCalc::calculateStat(const DownloadEngine* e)
{
  if (cp_.difference(global::wallclock()) + A2_DELTA_MILLIS <
      std::chrono::milliseconds(1000)) {
    return;
  }
  cp_ = global::wallclock();
  const SizeFormatter& sizeFormatter = *sizeFormatter_.get();

  // Some terminals (e.g., Windows terminal) prints next line when the
  // character reached at the last column.
  unsigned short int cols = 79;

  if (isTTY_) {
#ifndef __MINGW32__
#  ifdef HAVE_TERMIOS_H
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) {
      cols = std::max(0, (int)size.ws_col - 1);
    }
#  endif // HAVE_TERMIOS_H
#else    // __MINGW32__
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE),
                                     &info)) {
      cols = std::max(0, info.dwSize.X - 2);
    }
#endif   // !__MINGW32__
    std::string line(cols, ' ');
    global::cout()->printf("\r%s\r", line.c_str());
  }
  ColorizedStream o;
  if (e->getRequestGroupMan()->countRequestGroup() > 0) {
    if ((summaryInterval_ > 0_s) &&
        lastSummaryNotified_.difference(global::wallclock()) +
                A2_DELTA_MILLIS >=
            summaryInterval_) {
      lastSummaryNotified_ = global::wallclock();
      printProgressSummary(e->getRequestGroupMan()->getRequestGroups(), cols, e,
                           sizeFormatter);
      global::cout()->write("\n");
      global::cout()->flush();
    }
  }
  if (!readoutVisibility_) {
    return;
  }
  size_t numGroup = e->getRequestGroupMan()->countRequestGroup();
  const bool color = global::cout()->supportsColor() && isTTY_ && colorOutput_;
  if (numGroup == 1) {
    const std::shared_ptr<RequestGroup>& rg =
        *e->getRequestGroupMan()->getRequestGroups().begin();
    printProgress(o, rg, e, sizeFormatter);
  }
  else if (numGroup > 1) {
    // For more than 2 RequestGroups, use compact readout form
    printProgressCompact(o, e, sizeFormatter);
  }

  {
    auto& entry = e->getFileAllocationMan()->getPickedEntry();
    if (entry) {
      o << " [FileAlloc:#"
        << GroupId::toAbbrevHex(entry->getRequestGroup()->getGID()) << " "
        << sizeFormatter(entry->getCurrentLength()) << "B/"
        << sizeFormatter(entry->getTotalLength()) << "B(";
      if (entry->getTotalLength() > 0) {
        o << 100LL * entry->getCurrentLength() / entry->getTotalLength();
      }
      else {
        o << "--";
      }
      o << "%)]";
      if (e->getFileAllocationMan()->hasNext()) {
        o << "(+" << e->getFileAllocationMan()->countEntryInQueue() << ")";
      }
    }
  }
  {
    auto& entry = e->getCheckIntegrityMan()->getPickedEntry();
    if (entry) {
      o << " [Checksum:#"
        << GroupId::toAbbrevHex(entry->getRequestGroup()->getGID()) << " "
        << sizeFormatter(entry->getCurrentLength()) << "B/"
        << sizeFormatter(entry->getTotalLength()) << "B(";
      if (entry->getTotalLength() > 0) {
        o << 100LL * entry->getCurrentLength() / entry->getTotalLength();
      }
      else {
        o << "--";
      }
      o << "%)]";
      if (e->getCheckIntegrityMan()->hasNext()) {
        o << "(+" << e->getCheckIntegrityMan()->countEntryInQueue() << ")";
      }
    }
  }
  if (isTTY_) {
    if (truncate_) {
      auto str = o.str(color, cols);
      global::cout()->write(str.c_str());
    }
    else {
      auto str = o.str(color);
      global::cout()->write(str.c_str());
    }
    global::cout()->flush();
  }
  else {
    auto str = o.str(false);
    global::cout()->write(str.c_str());
    global::cout()->write("\n");
  }
}

} // namespace aria2
