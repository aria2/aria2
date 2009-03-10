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
#ifndef _D_DOWNLOAD_CONTEXT_H_
#define _D_DOWNLOAD_CONTEXT_H_

#include "common.h"

#include <string>
#include <deque>

#include "SharedHandle.h"
#include "Signature.h"
#include "TimeA2.h"

namespace aria2 {

class FileEntry;

class DownloadContext
{
protected:
  std::string _dir;

private:
  Time _downloadStartTime;

  Time _downloadStopTime;

  SharedHandle<Signature> _signature;
public:
  DownloadContext();

  virtual ~DownloadContext();

  enum FILE_MODE {
    SINGLE,
    MULTI
  };

  virtual const std::string& getPieceHash(size_t index) const = 0;
  
  virtual const std::deque<std::string>& getPieceHashes() const = 0;

  virtual uint64_t getTotalLength() const = 0;

  virtual bool knowsTotalLength() const = 0;

  virtual FILE_MODE getFileMode() const = 0;

  virtual std::deque<SharedHandle<FileEntry> > getFileEntries() const = 0;

  virtual size_t getPieceLength() const = 0;

  virtual size_t getNumPieces() const = 0;

  virtual const std::string& getPieceHashAlgo() const = 0;

  /**
   * Returns an actual file path.
   * If this contains a single file entry, then returns its file path,
   * for example, "/tmp/downloads/aria2.txt"
   * If this contains multiple file entries(i,e /tmp/downloads/aria2.txt,
   * /tmp/downloads/aria2.bin), then returns its base dir path,
   * for example, "/tmp/downloads"
   */
  virtual std::string getActualBasePath() const = 0;

  const std::string& getDir() const;

  void setDir(const std::string& dir);

  SharedHandle<Signature> getSignature() const;

  void setSignature(const SharedHandle<Signature>& signature);

  void resetDownloadStartTime();

  void resetDownloadStopTime();

  int64_t calculateSessionTime() const;
};

typedef SharedHandle<DownloadContext> DownloadContextHandle;

} // namespace aria2

#endif // _D_DOWNLOAD_CONTEXT_H_
