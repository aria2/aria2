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
#ifndef D_DEFAULT_BT_PROGRESS_INFO_FILE_H
#define D_DEFAULT_BT_PROGRESS_INFO_FILE_H

#include "BtProgressInfoFile.h"

#include <memory>

namespace aria2 {

class DownloadContext;
class PieceStorage;
class PeerStorage;
class BtRuntime;
class Option;
class IOFile;

class DefaultBtProgressInfoFile : public BtProgressInfoFile {
private:
  std::shared_ptr<DownloadContext> dctx_;
  std::shared_ptr<PieceStorage> pieceStorage_;
#ifdef ENABLE_BITTORRENT
  std::shared_ptr<PeerStorage> peerStorage_;
  std::shared_ptr<BtRuntime> btRuntime_;
#endif // ENABLE_BITTORRENT
  const Option* option_;
  std::string filename_;
  // Last SHA1 digest value of the content written.  Initially, this
  // is empty string.  This is used to avoid to write same content
  // repeatedly, which could wake up disk that may be sleeping.
  std::string lastDigest_;

  bool isTorrentDownload();
  void save(IOFile& fp);

public:
  DefaultBtProgressInfoFile(const std::shared_ptr<DownloadContext>& btContext,
                            const std::shared_ptr<PieceStorage>& pieceStorage,
                            const Option* option);

  virtual ~DefaultBtProgressInfoFile();

  virtual std::string getFilename() CXX11_OVERRIDE { return filename_; }

  virtual bool exists() CXX11_OVERRIDE;

  virtual void save() CXX11_OVERRIDE;

  virtual void load() CXX11_OVERRIDE;

  virtual void removeFile() CXX11_OVERRIDE;

  // re-set filename using current dctx_.
  virtual void updateFilename() CXX11_OVERRIDE;

#ifdef ENABLE_BITTORRENT
  // for torrents
  void setPeerStorage(const std::shared_ptr<PeerStorage>& peerStorage);

  void setBtRuntime(const std::shared_ptr<BtRuntime>& btRuntime);
#endif // ENABLE_BITTORRENT

  static const std::string& getSuffix()
  {
    static std::string suffix = ".aria2";
    return suffix;
  }
};

} // namespace aria2

#endif // D_DEFAULT_BT_PROGRESS_INFO_FILE_H
