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
#ifndef D_TRACKER_WATCHER_COMMAND_H
#define D_TRACKER_WATCHER_COMMAND_H

#include "Command.h"

#include <string>

#include "SharedHandle.h"

namespace aria2 {

class DownloadEngine;
class RequestGroup;
class PeerStorage;
class PieceStorage;
class BtRuntime;
class BtAnnounce;
class Option;
struct UDPTrackerRequest;
class UDPTrackerClient;

class AnnRequest {
public:
  virtual ~AnnRequest() {}
  // Returns true if tracker request is finished, regardless of the
  // outcome.
  virtual bool stopped() const = 0;
  // Returns true if tracker request is successful.
  virtual bool success() const = 0;
  // Returns true if issuing request is successful.
  virtual bool issue(DownloadEngine* e) = 0;
  // Stop this request.
  virtual void stop(DownloadEngine* e) = 0;
  // Returns true if processing tracker response is successful.
  virtual bool processResponse(const SharedHandle<BtAnnounce>& btAnnounce) = 0;
};

class HTTPAnnRequest:public AnnRequest {
public:
  HTTPAnnRequest(const SharedHandle<RequestGroup>& rg);
  virtual ~HTTPAnnRequest();
  virtual bool stopped() const;
  virtual bool success() const;
  virtual bool issue(DownloadEngine* e);
  virtual void stop(DownloadEngine* e);
  virtual bool processResponse(const SharedHandle<BtAnnounce>& btAnnounce);
private:
  SharedHandle<RequestGroup> rg_;
};

class UDPAnnRequest:public AnnRequest {
public:
  UDPAnnRequest(const SharedHandle<UDPTrackerRequest>& req);
  virtual ~UDPAnnRequest();
  virtual bool stopped() const;
  virtual bool success() const;
  virtual bool issue(DownloadEngine* e);
  virtual void stop(DownloadEngine* e);
  virtual bool processResponse(const SharedHandle<BtAnnounce>& btAnnounce);
private:
  SharedHandle<UDPTrackerRequest> req_;
};

class TrackerWatcherCommand : public Command
{
private:
  RequestGroup* requestGroup_;

  DownloadEngine* e_;

  SharedHandle<UDPTrackerClient> udpTrackerClient_;

  SharedHandle<PeerStorage> peerStorage_;

  SharedHandle<PieceStorage> pieceStorage_;

  SharedHandle<BtRuntime> btRuntime_;

  SharedHandle<BtAnnounce> btAnnounce_;

  SharedHandle<AnnRequest> trackerRequest_;

  /**
   * Returns a command for announce request. Returns 0 if no announce request
   * is needed.
   */
  SharedHandle<AnnRequest>
  createHTTPAnnRequest(const std::string& uri);

  SharedHandle<AnnRequest>
  createUDPAnnRequest(const std::string& host, uint16_t port,
                      uint16_t localPort);

  void addConnection();

  const SharedHandle<Option>& getOption() const;
public:
  TrackerWatcherCommand(cuid_t cuid,
                        RequestGroup* requestGroup,
                        DownloadEngine* e);

  virtual ~TrackerWatcherCommand();

  SharedHandle<AnnRequest> createAnnounce(DownloadEngine* e);

  virtual bool execute();

  void setPeerStorage(const SharedHandle<PeerStorage>& peerStorage);

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setBtRuntime(const SharedHandle<BtRuntime>& btRuntime);

  void setBtAnnounce(const SharedHandle<BtAnnounce>& btAnnounce);
};

} // namespace aria2

#endif // D_TRACKER_WATCHER_COMMAND_H
