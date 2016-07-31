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
#include <memory>

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
  virtual ~AnnRequest() = default;
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
  virtual bool
  processResponse(const std::shared_ptr<BtAnnounce>& btAnnounce) = 0;
};

class HTTPAnnRequest : public AnnRequest {
public:
  HTTPAnnRequest(std::unique_ptr<RequestGroup> rg);
  virtual ~HTTPAnnRequest();
  virtual bool stopped() const CXX11_OVERRIDE;
  virtual bool success() const CXX11_OVERRIDE;
  virtual bool issue(DownloadEngine* e) CXX11_OVERRIDE;
  virtual void stop(DownloadEngine* e) CXX11_OVERRIDE;
  virtual bool
  processResponse(const std::shared_ptr<BtAnnounce>& btAnnounce) CXX11_OVERRIDE;

private:
  std::unique_ptr<RequestGroup> rg_;
};

class UDPAnnRequest : public AnnRequest {
public:
  UDPAnnRequest(const std::shared_ptr<UDPTrackerRequest>& req);
  virtual ~UDPAnnRequest();
  virtual bool stopped() const CXX11_OVERRIDE;
  virtual bool success() const CXX11_OVERRIDE;
  virtual bool issue(DownloadEngine* e) CXX11_OVERRIDE;
  virtual void stop(DownloadEngine* e) CXX11_OVERRIDE;
  virtual bool
  processResponse(const std::shared_ptr<BtAnnounce>& btAnnounce) CXX11_OVERRIDE;

private:
  std::shared_ptr<UDPTrackerRequest> req_;
};

class TrackerWatcherCommand : public Command {
private:
  RequestGroup* requestGroup_;

  DownloadEngine* e_;

  std::shared_ptr<UDPTrackerClient> udpTrackerClient_;

  std::shared_ptr<PeerStorage> peerStorage_;

  std::shared_ptr<PieceStorage> pieceStorage_;

  std::shared_ptr<BtRuntime> btRuntime_;

  std::shared_ptr<BtAnnounce> btAnnounce_;

  std::unique_ptr<AnnRequest> trackerRequest_;

  /**
   * Returns a command for announce request. Returns 0 if no announce request
   * is needed.
   */
  std::unique_ptr<AnnRequest> createHTTPAnnRequest(const std::string& uri);

  std::unique_ptr<AnnRequest> createUDPAnnRequest(const std::string& host,
                                                  uint16_t port,
                                                  uint16_t localPort);

  void addConnection();

  const std::shared_ptr<Option>& getOption() const;

public:
  TrackerWatcherCommand(cuid_t cuid, RequestGroup* requestGroup,
                        DownloadEngine* e);

  virtual ~TrackerWatcherCommand();

  std::unique_ptr<AnnRequest> createAnnounce(DownloadEngine* e);

  virtual bool execute() CXX11_OVERRIDE;

  void setPeerStorage(const std::shared_ptr<PeerStorage>& peerStorage);

  void setPieceStorage(const std::shared_ptr<PieceStorage>& pieceStorage);

  void setBtRuntime(const std::shared_ptr<BtRuntime>& btRuntime);

  void setBtAnnounce(const std::shared_ptr<BtAnnounce>& btAnnounce);
};

} // namespace aria2

#endif // D_TRACKER_WATCHER_COMMAND_H
