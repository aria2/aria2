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
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "FileAllocationMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityMan.h"
#include "CheckIntegrityEntry.h"
#include "Util.h"
#ifdef ENABLE_BITTORRENT
# include "BtContext.h"
#endif // ENABLE_BITTORRENT
#include <iomanip>

void
ConsoleStatCalc::calculateStat(const RequestGroupManHandle& requestGroupMan,
			       const FileAllocationManHandle& fileAllocationMan,
			       const CheckIntegrityManHandle& checkIntegrityMan)
{
  if(!_cp.elapsed(1)) {
    return;
  }
  _cp.reset();

  cout << "\r                                                                             ";
  cout << "\r";
  if(requestGroupMan->countRequestGroup() > 0) {
    RequestGroupHandle firstRequestGroup = requestGroupMan->getRequestGroup(0);
    TransferStat stat = firstRequestGroup->calculateStat();
    int32_t eta = 0;
    if(firstRequestGroup->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
      eta = (firstRequestGroup->getTotalLength()-firstRequestGroup->getCompletedLength())/stat.getDownloadSpeed();
    }

    cout << "["
	 << "#" << firstRequestGroup->getGID() << " ";
#ifdef ENABLE_BITTORRENT
    if(firstRequestGroup->downloadFinished() &&
       !BtContextHandle(firstRequestGroup->getDownloadContext()).isNull()) {
      cout << "SEEDING" << "(" << "ratio:"
	   << fixed << setprecision(2) << stat.getAllTimeUploadLength()*1.0/firstRequestGroup->getCompletedLength()
	   << ")";
    } else
#endif // ENABLE_BITTORRENT
      {
	cout << "SIZE:"
	     << Util::abbrevSize(firstRequestGroup->getCompletedLength())
	     << "B"
	     << "/"
	     << Util::abbrevSize(firstRequestGroup->getTotalLength())
	     << "B";
	if(firstRequestGroup->getTotalLength() > 0) {
	  cout << "("
	       << 100*firstRequestGroup->getCompletedLength()/firstRequestGroup->getTotalLength()
	       << "%)";
	}
      }
    cout << " "
	 << "CN:"
	 << firstRequestGroup->getNumConnection();
    if(!firstRequestGroup->downloadFinished()) {
      cout << " "
	   << "SPD:"
	   << fixed << setprecision(2) << stat.getDownloadSpeed()/1024.0 << "KiB/s";
    }
    if(stat.getSessionUploadLength() > 0) {
      cout << " "
	   << "UP:"
	   << fixed << setprecision(2) << stat.getUploadSpeed()/1024.0 << "KiB/s"
	   << "(" << Util::abbrevSize(stat.getAllTimeUploadLength()) << "B)";
    }
    if(eta > 0) {
      cout << " "
	   << "ETA:"
	   << Util::secfmt(eta);
    }
    cout << "]";
    if(requestGroupMan->countRequestGroup() > 1) {
      cout << "("
	   << requestGroupMan->countRequestGroup()-1
	   << "more...)";
    }
  }

  if(requestGroupMan->countRequestGroup() > 1 &&
     !requestGroupMan->downloadFinished()) {
    TransferStat stat = requestGroupMan->calculateStat();
    cout << " "
	 << "[TOTAL SPD:"
	 << fixed << setprecision(2) << stat.getDownloadSpeed()/1024.0 << "KiB/s" << "]";
  }

  {
    FileAllocationEntryHandle entry = fileAllocationMan->getCurrentFileAllocationEntry();
    if(!entry.isNull()) {
      cout << " "
	   << "[FileAlloc:"
	   << "#" << entry->getRequestGroup()->getGID() << " "
	   << Util::abbrevSize(entry->getCurrentLength())
	   << "B"
	   << "/"
	   << Util::abbrevSize(entry->getTotalLength())
	   << "B"
	   << "(";
      if(entry->getTotalLength() > 0) {
	cout << 100*entry->getCurrentLength()/entry->getTotalLength();
      } else {
	cout << "--";
      }
      cout << "%)"
	   << "]";
      if(fileAllocationMan->countFileAllocationEntryInQueue() > 0) {
	cout << "("
	     << fileAllocationMan->countFileAllocationEntryInQueue()
	     << "waiting...)";
      }
    }
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    CheckIntegrityEntryHandle entry = checkIntegrityMan->getFirstCheckIntegrityEntry();
    if(!entry.isNull()) {
      cout << " "
	   << "[Checksum:"
	   << "#" << entry->getRequestGroup()->getGID() << " "
	   << Util::abbrevSize(entry->getCurrentLength())
	   << "B"
	   << "/"
	   << Util::abbrevSize(entry->getTotalLength())
	   << "B"
	   << "("
	   << 100*entry->getCurrentLength()/entry->getTotalLength()
	   << "%)";
      cout << "]";
      if(checkIntegrityMan->countCheckIntegrityEntry() > 1) {
	cout << "("
	     << checkIntegrityMan->countCheckIntegrityEntry()-1
	     << "more...)";
      }
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  cout << flush;
}
