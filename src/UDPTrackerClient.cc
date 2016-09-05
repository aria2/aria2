/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "UDPTrackerClient.h"

#include "UDPTrackerRequest.h"
#include "bittorrent_helper.h"
#include "util.h"
#include "LogFactory.h"
#include "SimpleRandomizer.h"
#include "fmt.h"

namespace aria2 {

UDPTrackerClient::UDPTrackerClient() : numWatchers_(0) {}

namespace {
template <typename InputIterator>
void failRequest(InputIterator first, InputIterator last, int error)
{
  for (; first != last; ++first) {
    (*first)->state = UDPT_STA_COMPLETE;
    (*first)->error = error;
  }
}
} // namespace

namespace {
uint32_t generateTransactionId()
{
  uint32_t res;
  SimpleRandomizer::getInstance()->getRandomBytes(
      reinterpret_cast<unsigned char*>(&res), sizeof(res));
  return res;
}
} // namespace

namespace {
void logInvalidLength(const std::string& remoteAddr, uint16_t remotePort,
                      int action, unsigned long expected, unsigned long actual)
{
  A2_LOG_INFO(fmt("UDPT received %s reply from %s:%u invalid length "
                  "expected:%lu, actual:%lu",
                  getUDPTrackerActionStr(action), remoteAddr.c_str(),
                  remotePort, expected, actual));
}
} // namespace

namespace {
void logInvalidTransaction(const std::string& remoteAddr, uint16_t remotePort,
                           int action, uint32_t transactionId)
{
  A2_LOG_INFO(
      fmt("UDPT received %s reply from %s:%u invalid transaction_id=%08x",
          getUDPTrackerActionStr(action), remoteAddr.c_str(), remotePort,
          transactionId));
}
} // namespace

namespace {
void logTooShortLength(const std::string& remoteAddr, uint16_t remotePort,
                       int action, unsigned long minLength,
                       unsigned long actual)
{
  A2_LOG_INFO(fmt("UDPT received %s reply from %s:%u length too short "
                  "min:%lu, actual:%lu",
                  getUDPTrackerActionStr(action), remoteAddr.c_str(),
                  remotePort, minLength, actual));
}
} // namespace

UDPTrackerClient::~UDPTrackerClient()
{
  // Make all contained requests fail
  int error = UDPT_ERR_SHUTDOWN;
  failRequest(inflightRequests_.begin(), inflightRequests_.end(), error);
  failRequest(pendingRequests_.begin(), pendingRequests_.end(), error);
  failRequest(connectRequests_.begin(), connectRequests_.end(), error);
}

namespace {
struct CollectAddrPortMatch {
  bool operator()(const std::shared_ptr<UDPTrackerRequest>& req) const
  {
    if (req->remoteAddr == remoteAddr && req->remotePort == remotePort) {
      dest.push_back(req);
      return true;
    }
    else {
      return false;
    }
  }
  std::vector<std::shared_ptr<UDPTrackerRequest>>& dest;
  std::string remoteAddr;
  uint16_t remotePort;
  CollectAddrPortMatch(std::vector<std::shared_ptr<UDPTrackerRequest>>& dest,
                       std::string remoteAddr, uint16_t remotePort)
      : dest(dest), remoteAddr(std::move(remoteAddr)), remotePort(remotePort)
  {
  }
};
} // namespace

int UDPTrackerClient::receiveReply(std::shared_ptr<UDPTrackerRequest>& recvReq,
                                   const unsigned char* data, size_t length,
                                   const std::string& remoteAddr,
                                   uint16_t remotePort, const Timer& now)
{
  int32_t action = bittorrent::getIntParam(data, 0);
  switch (action) {
  case UDPT_ACT_CONNECT: {
    if (length != 16) {
      logInvalidLength(remoteAddr, remotePort, action, 16, length);
      return -1;
    }
    auto transactionId = bittorrent::getIntParam(data, 4);
    std::shared_ptr<UDPTrackerRequest> req =
        findInflightRequest(remoteAddr, remotePort, transactionId, true);
    if (!req) {
      logInvalidTransaction(remoteAddr, remotePort, action, transactionId);
      return -1;
    }

    auto connectionId = bittorrent::getLLIntParam(data, 8);
    A2_LOG_INFO(
        fmt("UDPT received CONNECT reply from %s:%u transaction_id=%08x,"
            "connection_id=%016" PRIx64,
            remoteAddr.c_str(), remotePort, transactionId, connectionId));
    UDPTrackerConnection c(UDPT_CST_CONNECTED, connectionId, now);
    connectionIdCache_[std::make_pair(remoteAddr, remotePort)] = c;
    // Now we have connection ID, push requests which are waiting for
    // it.
    std::vector<std::shared_ptr<UDPTrackerRequest>> reqs;
    connectRequests_.erase(
        std::remove_if(connectRequests_.begin(), connectRequests_.end(),
                       CollectAddrPortMatch(reqs, remoteAddr, remotePort)),
        connectRequests_.end());
    pendingRequests_.insert(pendingRequests_.begin(), reqs.begin(), reqs.end());

    recvReq = std::move(req);

    break;
  }
  case UDPT_ACT_ANNOUNCE: {
    if (length < 20) {
      logTooShortLength(remoteAddr, remotePort, action, 20, length);
      return -1;
    }
    auto transactionId = bittorrent::getIntParam(data, 4);
    std::shared_ptr<UDPTrackerRequest> req =
        findInflightRequest(remoteAddr, remotePort, transactionId, true);
    if (!req) {
      logInvalidTransaction(remoteAddr, remotePort, action, transactionId);
      return -1;
    }
    req->state = UDPT_STA_COMPLETE;

    req->reply = std::make_shared<UDPTrackerReply>();
    req->reply->action = action;
    req->reply->transactionId = transactionId;
    req->reply->interval = bittorrent::getIntParam(data, 8);
    req->reply->leechers = bittorrent::getIntParam(data, 12);
    req->reply->seeders = bittorrent::getIntParam(data, 16);

    int numPeers = 0;
    for (size_t i = 20; i < length; i += 6) {
      std::pair<std::string, uint16_t> hostport =
          bittorrent::unpackcompact(data + i, AF_INET);
      if (!hostport.first.empty()) {
        req->reply->peers.push_back(hostport);
        ++numPeers;
      }
    }

    A2_LOG_INFO(
        fmt("UDPT received ANNOUNCE reply from %s:%u transaction_id=%08x,"
            "connection_id=%016" PRIx64 ", event=%s, infohash=%s, "
            "interval=%d, leechers=%d, "
            "seeders=%d, num_peers=%d",
            remoteAddr.c_str(), remotePort, transactionId, req->connectionId,
            getUDPTrackerEventStr(req->event),
            util::toHex(req->infohash).c_str(), req->reply->interval,
            req->reply->leechers, req->reply->seeders, numPeers));

    recvReq = std::move(req);

    break;
  }
  case UDPT_ACT_ERROR: {
    if (length < 8) {
      logTooShortLength(remoteAddr, remotePort, action, 8, length);
      return -1;
    }
    auto transactionId = bittorrent::getIntParam(data, 4);
    std::shared_ptr<UDPTrackerRequest> req =
        findInflightRequest(remoteAddr, remotePort, transactionId, true);
    if (!req) {
      logInvalidTransaction(remoteAddr, remotePort, action, transactionId);
      return -1;
    }
    std::string errorString(data + 8, data + length);
    errorString = util::encodeNonUtf8(errorString);

    req->state = UDPT_STA_COMPLETE;
    req->error = UDPT_ERR_TRACKER;

    A2_LOG_INFO(fmt("UDPT received ERROR reply from %s:%u transaction_id=%08x,"
                    "connection_id=%016" PRIx64 ", action=%d, error_string=%s",
                    remoteAddr.c_str(), remotePort, transactionId,
                    req->connectionId, action, errorString.c_str()));
    if (req->action == UDPT_ACT_CONNECT) {
      failConnect(req->remoteAddr, req->remotePort, UDPT_ERR_TRACKER);
    }

    recvReq = std::move(req);

    break;
  }
  case UDPT_ACT_SCRAPE:
    A2_LOG_INFO(fmt("unexpected scrape action reply from %s:%u",
                    remoteAddr.c_str(), remotePort));
    return -1;
  default:
    A2_LOG_INFO(
        fmt("unknown action reply from %s:%u", remoteAddr.c_str(), remotePort));
    return -1;
  }
  return 0;
}

ssize_t UDPTrackerClient::createRequest(unsigned char* data, size_t length,
                                        std::string& remoteAddr,
                                        uint16_t& remotePort, const Timer& now)
{
  if (pendingRequests_.empty()) {
    return -1;
  }
  while (!pendingRequests_.empty()) {
    const std::shared_ptr<UDPTrackerRequest>& req = pendingRequests_.front();
    if (req->action == UDPT_ACT_CONNECT) {
      ssize_t rv;
      rv = createUDPTrackerConnect(data, length, remoteAddr, remotePort, req);
      return rv;
    }
    UDPTrackerConnection* c =
        getConnectionId(req->remoteAddr, req->remotePort, now);
    if (!c) {
      auto creq = std::make_shared<UDPTrackerRequest>();
      creq->action = UDPT_ACT_CONNECT;
      creq->remoteAddr = req->remoteAddr;
      creq->remotePort = req->remotePort;
      creq->transactionId = generateTransactionId();
      pendingRequests_.push_front(creq);
      ssize_t rv;
      rv = createUDPTrackerConnect(data, length, remoteAddr, remotePort, creq);
      return rv;
    }
    if (c->state == UDPT_CST_CONNECTING) {
      connectRequests_.push_back(req);
      pendingRequests_.pop_front();
      continue;
    }
    req->connectionId = c->connectionId;
    req->transactionId = generateTransactionId();
    ssize_t rv;
    rv = createUDPTrackerAnnounce(data, length, remoteAddr, remotePort, req);
    return rv;
  }
  return -1;
}

void UDPTrackerClient::requestSent(const Timer& now)
{
  if (pendingRequests_.empty()) {
    A2_LOG_WARN("pendingRequests_ is empty");
    return;
  }
  const std::shared_ptr<UDPTrackerRequest>& req = pendingRequests_.front();
  switch (req->action) {
  case UDPT_ACT_CONNECT:
    A2_LOG_INFO(fmt("UDPT sent CONNECT to %s:%u transaction_id=%08x",
                    req->remoteAddr.c_str(), req->remotePort,
                    req->transactionId));
    break;
  case UDPT_ACT_ANNOUNCE:
    A2_LOG_INFO(fmt("UDPT sent ANNOUNCE to %s:%u transaction_id=%08x, "
                    "connection_id=%016" PRIx64 ", event=%s, infohash=%s",
                    req->remoteAddr.c_str(), req->remotePort,
                    req->transactionId, req->connectionId,
                    getUDPTrackerEventStr(req->event),
                    util::toHex(req->infohash).c_str()));
    break;
  default:
    // unreachable
    assert(0);
  }
  req->dispatched = now;
  switch (req->action) {
  case UDPT_ACT_CONNECT: {
    connectionIdCache_[std::make_pair(req->remoteAddr, req->remotePort)] =
        UDPTrackerConnection();
    break;
  }
  }
  inflightRequests_.push_back(req);
  pendingRequests_.pop_front();
}

void UDPTrackerClient::requestFail(int error)
{
  if (pendingRequests_.empty()) {
    A2_LOG_WARN("pendingRequests_ is empty");
    return;
  }
  const std::shared_ptr<UDPTrackerRequest>& req = pendingRequests_.front();
  switch (req->action) {
  case UDPT_ACT_CONNECT:
    A2_LOG_INFO(fmt("UDPT fail CONNECT to %s:%u transaction_id=%08x",
                    req->remoteAddr.c_str(), req->remotePort,
                    req->transactionId));
    failConnect(req->remoteAddr, req->remotePort, error);
    break;
  case UDPT_ACT_ANNOUNCE:
    A2_LOG_INFO(fmt("UDPT fail ANNOUNCE to %s:%u transaction_id=%08x, "
                    "connection_id=%016" PRIx64 ", event=%s, infohash=%s",
                    req->remoteAddr.c_str(), req->remotePort,
                    req->transactionId, req->connectionId,
                    getUDPTrackerEventStr(req->event),
                    util::toHex(req->infohash).c_str()));
    break;
  default:
    // unreachable
    assert(0);
  }
  req->state = UDPT_STA_COMPLETE;
  req->error = error;
  pendingRequests_.pop_front();
}

void UDPTrackerClient::addRequest(const std::shared_ptr<UDPTrackerRequest>& req)
{
  req->state = UDPT_STA_PENDING;
  req->error = UDPT_ERR_SUCCESS;
  pendingRequests_.push_back(req);
}

namespace {
struct TimeoutCheck {
  bool operator()(const std::shared_ptr<UDPTrackerRequest>& req) const
  {
    auto t = req->dispatched.difference(now);
    if (req->failCount == 0) {
      if (t >= 5_s) {
        switch (req->action) {
        case UDPT_ACT_CONNECT:
          A2_LOG_INFO(fmt("UDPT resend CONNECT to %s:%u transaction_id=%08x",
                          req->remoteAddr.c_str(), req->remotePort,
                          req->transactionId));
          break;
        case UDPT_ACT_ANNOUNCE:
          A2_LOG_INFO(fmt("UDPT resend ANNOUNCE to %s:%u transaction_id=%08x, "
                          "connection_id=%016" PRIx64 ", event=%s, infohash=%s",
                          req->remoteAddr.c_str(), req->remotePort,
                          req->transactionId, req->connectionId,
                          getUDPTrackerEventStr(req->event),
                          util::toHex(req->infohash).c_str()));
          break;
        default:
          // unreachable
          assert(0);
        }
        ++req->failCount;
        dest.push_back(req);
        return true;
      }
      else {
        return false;
      }
    }
    else {
      if (t >= 10_s) {
        switch (req->action) {
        case UDPT_ACT_CONNECT:
          A2_LOG_INFO(fmt("UDPT timeout CONNECT to %s:%u transaction_id=%08x",
                          req->remoteAddr.c_str(), req->remotePort,
                          req->transactionId));
          client->failConnect(req->remoteAddr, req->remotePort,
                              UDPT_ERR_TIMEOUT);
          break;
        case UDPT_ACT_ANNOUNCE:
          A2_LOG_INFO(fmt("UDPT timeout ANNOUNCE to %s:%u transaction_id=%08x, "
                          "connection_id=%016" PRIx64 ", event=%s, infohash=%s",
                          req->remoteAddr.c_str(), req->remotePort,
                          req->transactionId, req->connectionId,
                          getUDPTrackerEventStr(req->event),
                          util::toHex(req->infohash).c_str()));
          break;
        default:
          // unreachable
          assert(0);
        }
        ++req->failCount;
        req->state = UDPT_STA_COMPLETE;
        req->error = UDPT_ERR_TIMEOUT;
        return true;
      }
      else {
        return false;
      }
    }
  }
  std::vector<std::shared_ptr<UDPTrackerRequest>>& dest;
  UDPTrackerClient* client;
  const Timer& now;
  TimeoutCheck(std::vector<std::shared_ptr<UDPTrackerRequest>>& dest,
               UDPTrackerClient* client, const Timer& now)
      : dest(dest), client(client), now(now)
  {
  }
};
} // namespace

void UDPTrackerClient::handleTimeout(const Timer& now)
{
  std::vector<std::shared_ptr<UDPTrackerRequest>> dest;
  inflightRequests_.erase(std::remove_if(inflightRequests_.begin(),
                                         inflightRequests_.end(),
                                         TimeoutCheck(dest, this, now)),
                          inflightRequests_.end());
  pendingRequests_.insert(pendingRequests_.begin(), dest.begin(), dest.end());
}

std::shared_ptr<UDPTrackerRequest>
UDPTrackerClient::findInflightRequest(const std::string& remoteAddr,
                                      uint16_t remotePort,
                                      uint32_t transactionId, bool remove)
{
  std::shared_ptr<UDPTrackerRequest> res;
  for (auto i = inflightRequests_.begin(), eoi = inflightRequests_.end();
       i != eoi; ++i) {
    if ((*i)->remoteAddr == remoteAddr && (*i)->remotePort == remotePort &&
        (*i)->transactionId == transactionId) {
      res = *i;
      if (remove) {
        inflightRequests_.erase(i);
      }
      break;
    }
  }
  return res;
}

UDPTrackerConnection*
UDPTrackerClient::getConnectionId(const std::string& remoteAddr,
                                  uint16_t remotePort, const Timer& now)
{
  auto i = connectionIdCache_.find(std::make_pair(remoteAddr, remotePort));
  if (i == connectionIdCache_.end()) {
    return nullptr;
  }
  if ((*i).second.state == UDPT_CST_CONNECTED &&
      (*i).second.lastUpdated.difference(now) > 1_min) {
    connectionIdCache_.erase(i);
    return nullptr;
  }
  else {
    return &(*i).second;
  }
}

namespace {
struct FailConnectDelete {
  bool operator()(const std::shared_ptr<UDPTrackerRequest>& req) const
  {
    if (req->action == UDPT_ACT_ANNOUNCE && req->remoteAddr == remoteAddr &&
        req->remotePort == remotePort) {
      A2_LOG_INFO(
          fmt("Force fail infohash=%s", util::toHex(req->infohash).c_str()));
      req->state = UDPT_STA_COMPLETE;
      req->error = error;
      return true;
    }
    else {
      return false;
    }
  }
  std::string remoteAddr;
  uint16_t remotePort;
  int error;
  FailConnectDelete(std::string remoteAddr, uint16_t remotePort, int error)
      : remoteAddr(std::move(remoteAddr)), remotePort(remotePort), error(error)
  {
  }
};
} // namespace

void UDPTrackerClient::failConnect(const std::string& remoteAddr,
                                   uint16_t remotePort, int error)
{
  connectionIdCache_.erase(std::make_pair(remoteAddr, remotePort));
  // Fail all requests which are waiting for connection ID of the host.
  connectRequests_.erase(
      std::remove_if(connectRequests_.begin(), connectRequests_.end(),
                     FailConnectDelete(remoteAddr, remotePort, error)),
      connectRequests_.end());
  pendingRequests_.erase(
      std::remove_if(pendingRequests_.begin(), pendingRequests_.end(),
                     FailConnectDelete(remoteAddr, remotePort, error)),
      pendingRequests_.end());
}

void UDPTrackerClient::failAll()
{
  int error = UDPT_ERR_SHUTDOWN;
  failRequest(inflightRequests_.begin(), inflightRequests_.end(), error);
  failRequest(pendingRequests_.begin(), pendingRequests_.end(), error);
  failRequest(connectRequests_.begin(), connectRequests_.end(), error);
}

void UDPTrackerClient::increaseWatchers() { ++numWatchers_; }

void UDPTrackerClient::decreaseWatchers() { --numWatchers_; }

ssize_t createUDPTrackerConnect(unsigned char* data, size_t length,
                                std::string& remoteAddr, uint16_t& remotePort,
                                const std::shared_ptr<UDPTrackerRequest>& req)
{
  assert(length >= 16);
  remoteAddr = req->remoteAddr;
  remotePort = req->remotePort;
  bittorrent::setLLIntParam(data, UDPT_INITIAL_CONNECTION_ID);
  bittorrent::setIntParam(data + 8, req->action);
  bittorrent::setIntParam(data + 12, req->transactionId);
  return 16;
}

ssize_t createUDPTrackerAnnounce(unsigned char* data, size_t length,
                                 std::string& remoteAddr, uint16_t& remotePort,
                                 const std::shared_ptr<UDPTrackerRequest>& req)
{
  assert(length >= 100);
  remoteAddr = req->remoteAddr;
  remotePort = req->remotePort;
  bittorrent::setLLIntParam(data, req->connectionId);
  bittorrent::setIntParam(data + 8, req->action);
  bittorrent::setIntParam(data + 12, req->transactionId);
  memcpy(data + 16, req->infohash.c_str(), req->infohash.size());
  memcpy(data + 36, req->peerId.c_str(), req->peerId.size());
  bittorrent::setLLIntParam(data + 56, req->downloaded);
  bittorrent::setLLIntParam(data + 64, req->left);
  bittorrent::setLLIntParam(data + 72, req->uploaded);
  bittorrent::setIntParam(data + 80, req->event);
  // ip is already network-byte order
  memcpy(data + 84, &req->ip, sizeof(req->ip));
  bittorrent::setIntParam(data + 88, req->key);
  bittorrent::setIntParam(data + 92, req->numWant);
  bittorrent::setShortIntParam(data + 96, req->port);
  // extensions is always 0
  bittorrent::setShortIntParam(data + 98, 0);
  return 100;
}

const char* getUDPTrackerActionStr(int action)
{
  switch (action) {
  case UDPT_ACT_CONNECT:
    return "CONNECT";
  case UDPT_ACT_ANNOUNCE:
    return "ANNOUNCE";
  case UDPT_ACT_ERROR:
    return "ERROR";
  default:
    return "(unknown)";
  }
}

const char* getUDPTrackerEventStr(int event)
{
  switch (event) {
  case UDPT_EVT_NONE:
    return "NONE";
  case UDPT_EVT_COMPLETED:
    return "COMPLETED";
  case UDPT_EVT_STARTED:
    return "STARTED";
  case UDPT_EVT_STOPPED:
    return "STOPPED";
  default:
    return "(unknown)";
  }
}

} // namespace aria2
