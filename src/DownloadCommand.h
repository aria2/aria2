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
#ifndef D_DOWNLOAD_COMMAND_H
#define D_DOWNLOAD_COMMAND_H

#include "AbstractCommand.h"

#include <unistd.h>

namespace aria2 {

class PeerStat;
class StreamFilter;
#ifdef ENABLE_MESSAGE_DIGEST
class MessageDigest;
#endif // ENABLE_MESSAGE_DIGEST

class DownloadCommand : public AbstractCommand {
private:
  time_t startupIdleTime_;
  int lowestDownloadSpeedLimit_;
  SharedHandle<PeerStat> peerStat_;

  bool pieceHashValidationEnabled_;

#ifdef ENABLE_MESSAGE_DIGEST

  SharedHandle<MessageDigest> messageDigest_;

#endif // ENABLE_MESSAGE_DIGEST

  void validatePieceHash(const SharedHandle<Segment>& segment,
                         const std::string& expectedPieceHash,
                         const std::string& actualPieceHash);

  void checkLowestDownloadSpeed() const;

  SharedHandle<StreamFilter> streamFilter_;

  bool sinkFilterOnly_;
protected:
  virtual bool executeInternal();

  virtual bool prepareForNextSegment();

  // This is file local offset
  virtual int64_t getRequestEndOffset() const = 0;
public:
  DownloadCommand(cuid_t cuid,
                  const SharedHandle<Request>& req,
                  const SharedHandle<FileEntry>& fileEntry,
                  RequestGroup* requestGroup,
                  DownloadEngine* e,
                  const SharedHandle<SocketCore>& s,
                  const SharedHandle<SocketRecvBuffer>& socketRecvBuffer);
  virtual ~DownloadCommand();

  const SharedHandle<StreamFilter>& getStreamFilter() const
  {
    return streamFilter_;
  }

  void installStreamFilter(const SharedHandle<StreamFilter>& streamFilter);

  void setStartupIdleTime(time_t startupIdleTime)
  {
    startupIdleTime_ = startupIdleTime;
  }

  void setLowestDownloadSpeedLimit(int lowestDownloadSpeedLimit)
  {
    lowestDownloadSpeedLimit_ = lowestDownloadSpeedLimit;
  }
};

} // namespace aria2

#endif // D_DOWNLOAD_COMMAND_H
