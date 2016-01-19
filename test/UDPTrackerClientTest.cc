#include "UDPTrackerClient.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "UDPTrackerRequest.h"
#include "bittorrent_helper.h"
#include "wallclock.h"

namespace aria2 {

class UDPTrackerClientTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UDPTrackerClientTest);
  CPPUNIT_TEST(testCreateUDPTrackerConnect);
  CPPUNIT_TEST(testCreateUDPTrackerAnnounce);
  CPPUNIT_TEST(testConnectFollowedByAnnounce);
  CPPUNIT_TEST(testRequestFailure);
  CPPUNIT_TEST(testTimeout);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void testCreateUDPTrackerConnect();
  void testCreateUDPTrackerAnnounce();
  void testConnectFollowedByAnnounce();
  void testRequestFailure();
  void testTimeout();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UDPTrackerClientTest);

namespace {
std::shared_ptr<UDPTrackerRequest> createAnnounce(const std::string& remoteAddr,
                                                  uint16_t remotePort,
                                                  uint32_t transactionId)
{
  std::shared_ptr<UDPTrackerRequest> req(new UDPTrackerRequest());
  req->connectionId = std::numeric_limits<uint64_t>::max();
  req->action = UDPT_ACT_ANNOUNCE;
  req->remoteAddr = remoteAddr;
  req->remotePort = remotePort;
  req->transactionId = transactionId;
  req->infohash = "bittorrent-infohash-";
  req->peerId = "bittorrent-peer-id--";
  req->downloaded = INT64_MAX - 1;
  req->left = INT64_MAX - 2;
  req->uploaded = INT64_MAX - 3;
  req->event = UDPT_EVT_STARTED;
  req->ip = 0;
  req->key = 1000000007;
  req->numWant = 50;
  req->port = 6889;
  req->extensions = 0;
  return req;
}
} // namespace

namespace {
ssize_t createErrorReply(unsigned char* data, size_t len,
                         uint32_t transactionId, const std::string& errorString)
{
  bittorrent::setIntParam(data, UDPT_ACT_ERROR);
  bittorrent::setIntParam(data + 4, transactionId);
  memcpy(data + 8, errorString.c_str(), errorString.size());
  return 8 + errorString.size();
}
} // namespace

namespace {
ssize_t createConnectReply(unsigned char* data, size_t len,
                           uint64_t connectionId, uint32_t transactionId)
{
  bittorrent::setIntParam(data, UDPT_ACT_CONNECT);
  bittorrent::setIntParam(data + 4, transactionId);
  bittorrent::setLLIntParam(data + 8, connectionId);
  return 16;
}
} // namespace

namespace {
ssize_t createAnnounceReply(unsigned char* data, size_t len,
                            uint32_t transactionId, int numPeers = 0)
{
  bittorrent::setIntParam(data, UDPT_ACT_ANNOUNCE);
  bittorrent::setIntParam(data + 4, transactionId);
  bittorrent::setIntParam(data + 8, 1800);
  bittorrent::setIntParam(data + 12, 100);
  bittorrent::setIntParam(data + 16, 256);
  for (int i = 0; i < numPeers; ++i) {
    bittorrent::packcompact(data + 20 + 6 * i,
                            "192.168.0." + util::uitos(i + 1), 6990 + i);
  }
  return 20 + 6 * numPeers;
}
} // namespace

void UDPTrackerClientTest::testCreateUDPTrackerConnect()
{
  unsigned char data[16];
  std::string remoteAddr;
  uint16_t remotePort = 0;
  std::shared_ptr<UDPTrackerRequest> req(new UDPTrackerRequest());
  req->action = UDPT_ACT_CONNECT;
  req->remoteAddr = "192.168.0.1";
  req->remotePort = 6991;
  req->transactionId = 1000000009;
  ssize_t rv =
      createUDPTrackerConnect(data, sizeof(data), remoteAddr, remotePort, req);
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);
  CPPUNIT_ASSERT_EQUAL(req->remoteAddr, remoteAddr);
  CPPUNIT_ASSERT_EQUAL(req->remotePort, remotePort);
  CPPUNIT_ASSERT_EQUAL((int64_t)UDPT_INITIAL_CONNECTION_ID,
                       (int64_t)bittorrent::getLLIntParam(data, 0));
  CPPUNIT_ASSERT_EQUAL((int)req->action, (int)bittorrent::getIntParam(data, 8));
  CPPUNIT_ASSERT_EQUAL(req->transactionId, bittorrent::getIntParam(data, 12));
}

void UDPTrackerClientTest::testCreateUDPTrackerAnnounce()
{
  unsigned char data[100];
  std::string remoteAddr;
  uint16_t remotePort = 0;
  std::shared_ptr<UDPTrackerRequest> req(
      createAnnounce("192.168.0.1", 6991, 1000000009));
  ssize_t rv =
      createUDPTrackerAnnounce(data, sizeof(data), remoteAddr, remotePort, req);
  CPPUNIT_ASSERT_EQUAL((ssize_t)100, rv);
  CPPUNIT_ASSERT_EQUAL(req->connectionId, bittorrent::getLLIntParam(data, 0));
  CPPUNIT_ASSERT_EQUAL((int)req->action, (int)bittorrent::getIntParam(data, 8));
  CPPUNIT_ASSERT_EQUAL(req->transactionId, bittorrent::getIntParam(data, 12));
  CPPUNIT_ASSERT_EQUAL(req->infohash, std::string(&data[16], &data[36]));
  CPPUNIT_ASSERT_EQUAL(req->peerId, std::string(&data[36], &data[56]));
  CPPUNIT_ASSERT_EQUAL(req->downloaded,
                       (int64_t)bittorrent::getLLIntParam(data, 56));
  CPPUNIT_ASSERT_EQUAL(req->left, (int64_t)bittorrent::getLLIntParam(data, 64));
  CPPUNIT_ASSERT_EQUAL(req->uploaded,
                       (int64_t)bittorrent::getLLIntParam(data, 72));
  CPPUNIT_ASSERT_EQUAL(req->event, (int32_t)bittorrent::getIntParam(data, 80));
  CPPUNIT_ASSERT_EQUAL(req->ip, bittorrent::getIntParam(data, 84));
  CPPUNIT_ASSERT_EQUAL(req->key, bittorrent::getIntParam(data, 88));
  CPPUNIT_ASSERT_EQUAL(req->numWant,
                       (int32_t)bittorrent::getIntParam(data, 92));
  CPPUNIT_ASSERT_EQUAL(req->port, bittorrent::getShortIntParam(data, 96));
  CPPUNIT_ASSERT_EQUAL(req->extensions, bittorrent::getShortIntParam(data, 98));
}

void UDPTrackerClientTest::testConnectFollowedByAnnounce()
{
  ssize_t rv;
  UDPTrackerClient tr;
  unsigned char data[100];
  std::string remoteAddr;
  uint16_t remotePort;
  Timer now;
  std::shared_ptr<UDPTrackerRequest> recvReq;

  std::shared_ptr<UDPTrackerRequest> req1(
      createAnnounce("192.168.0.1", 6991, 0));
  std::shared_ptr<UDPTrackerRequest> req2(
      createAnnounce("192.168.0.1", 6991, 0));
  req2->infohash = "bittorrent-infohash2";

  tr.addRequest(req1);
  tr.addRequest(req2);
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.getPendingRequests().size());
  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  // CONNECT request was inserted
  CPPUNIT_ASSERT_EQUAL((size_t)3, tr.getPendingRequests().size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);
  CPPUNIT_ASSERT_EQUAL(req1->remoteAddr, remoteAddr);
  CPPUNIT_ASSERT_EQUAL(req1->remotePort, remotePort);
  CPPUNIT_ASSERT_EQUAL((int64_t)UDPT_INITIAL_CONNECTION_ID,
                       (int64_t)bittorrent::getLLIntParam(data, 0));
  uint32_t transactionId = bittorrent::getIntParam(data, 12);
  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  // Duplicate CONNECT request was not inserted
  CPPUNIT_ASSERT_EQUAL((size_t)3, tr.getPendingRequests().size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);

  tr.requestSent(now);
  // CONNECT request was moved to inflight
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.getPendingRequests().size());
  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  // Now all pending requests were moved to connect
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, rv);
  CPPUNIT_ASSERT(tr.getPendingRequests().empty());

  uint64_t connectionId = 12345;
  rv = createConnectReply(data, sizeof(data), connectionId, transactionId);
  rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                       now);
  CPPUNIT_ASSERT_EQUAL(0, (int)rv);
  if (rv == 0) {
    CPPUNIT_ASSERT_EQUAL((int32_t)UDPT_ACT_CONNECT, recvReq->action);
  }
  // Now 2 requests get back to pending
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.getPendingRequests().size());

  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  // Creates announce for req1
  CPPUNIT_ASSERT_EQUAL((ssize_t)100, rv);
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.getPendingRequests().size());
  CPPUNIT_ASSERT_EQUAL(connectionId, bittorrent::getLLIntParam(data, 0));
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE,
                       (int)bittorrent::getIntParam(data, 8));
  CPPUNIT_ASSERT_EQUAL(req1->infohash, std::string(&data[16], &data[36]));

  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  // Don't duplicate same request data
  CPPUNIT_ASSERT_EQUAL((ssize_t)100, rv);
  CPPUNIT_ASSERT_EQUAL((size_t)2, tr.getPendingRequests().size());
  uint32_t transactionId1 = bittorrent::getIntParam(data, 12);

  tr.requestSent(now);
  CPPUNIT_ASSERT_EQUAL((size_t)1, tr.getPendingRequests().size());

  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  uint32_t transactionId2 = bittorrent::getIntParam(data, 12);
  // Creates announce for req2
  CPPUNIT_ASSERT_EQUAL((ssize_t)100, rv);
  CPPUNIT_ASSERT_EQUAL((size_t)1, tr.getPendingRequests().size());
  CPPUNIT_ASSERT_EQUAL(connectionId, bittorrent::getLLIntParam(data, 0));
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE,
                       (int)bittorrent::getIntParam(data, 8));
  CPPUNIT_ASSERT_EQUAL(req2->infohash, std::string(&data[16], &data[36]));

  tr.requestSent(now);
  // Now all requests are inflight
  CPPUNIT_ASSERT_EQUAL((size_t)0, tr.getPendingRequests().size());

  // Reply for req2
  rv = createAnnounceReply(data, sizeof(data), transactionId2);
  rv = tr.receiveReply(recvReq, data, rv, req2->remoteAddr, req2->remotePort,
                       now);
  CPPUNIT_ASSERT_EQUAL(0, (int)rv);
  if (rv == 0) {
    CPPUNIT_ASSERT_EQUAL((int32_t)UDPT_ACT_ANNOUNCE, recvReq->action);
  }
  CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req2->state);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_SUCCESS, req2->error);

  // Reply for req1
  rv = createAnnounceReply(data, sizeof(data), transactionId1, 2);
  rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                       now);
  CPPUNIT_ASSERT_EQUAL(0, (int)rv);
  if (rv == 0) {
    CPPUNIT_ASSERT_EQUAL((int32_t)UDPT_ACT_ANNOUNCE, recvReq->action);
  }
  CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_SUCCESS, req1->error);
  CPPUNIT_ASSERT_EQUAL((size_t)2, req1->reply->peers.size());
  for (int i = 0; i < 2; ++i) {
    CPPUNIT_ASSERT_EQUAL("192.168.0." + util::uitos(i + 1),
                         req1->reply->peers[i].first);
    CPPUNIT_ASSERT_EQUAL((uint16_t)(6990 + i), req1->reply->peers[i].second);
  }

  // Since we have connection ID, next announce request can be sent
  // immediately
  std::shared_ptr<UDPTrackerRequest> req3(
      createAnnounce("192.168.0.1", 6991, 0));
  req3->infohash = "bittorrent-infohash3";
  tr.addRequest(req3);
  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
  CPPUNIT_ASSERT_EQUAL((ssize_t)100, rv);
  CPPUNIT_ASSERT_EQUAL(req3->infohash, std::string(&data[16], &data[36]));

  tr.requestSent(now);

  std::shared_ptr<UDPTrackerRequest> req4(
      createAnnounce("192.168.0.1", 6991, 0));
  req4->infohash = "bittorrent-infohash4";
  tr.addRequest(req4);
  Timer future = now;
  future.advance(1_h);
  rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, future);
  // connection ID is stale because of the timeout
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);
  CPPUNIT_ASSERT_EQUAL((int64_t)UDPT_INITIAL_CONNECTION_ID,
                       (int64_t)bittorrent::getLLIntParam(data, 0));
}

void UDPTrackerClientTest::testRequestFailure()
{
  ssize_t rv;
  UDPTrackerClient tr;
  unsigned char data[100];
  std::string remoteAddr;
  uint16_t remotePort;
  Timer now;
  std::shared_ptr<UDPTrackerRequest> recvReq;

  {
    std::shared_ptr<UDPTrackerRequest> req1(
        createAnnounce("192.168.0.1", 6991, 0));
    std::shared_ptr<UDPTrackerRequest> req2(
        createAnnounce("192.168.0.1", 6991, 0));

    tr.addRequest(req1);
    tr.addRequest(req2);
    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    tr.requestFail(UDPT_ERR_NETWORK);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_NETWORK, req1->error);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req2->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_NETWORK, req2->error);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT(tr.getPendingRequests().empty());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());
  }
  {
    std::shared_ptr<UDPTrackerRequest> req1(
        createAnnounce("192.168.0.1", 6991, 0));
    std::shared_ptr<UDPTrackerRequest> req2(
        createAnnounce("192.168.0.1", 6991, 0));

    tr.addRequest(req1);
    tr.addRequest(req2);
    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    uint32_t transactionId = bittorrent::getIntParam(data, 12);
    tr.requestSent(now);

    rv = createErrorReply(data, sizeof(data), transactionId, "error");
    rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                         now);
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, rv);
    if (rv == 0) {
      CPPUNIT_ASSERT_EQUAL((int32_t)UDPT_ACT_CONNECT, recvReq->action);
    }
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TRACKER, req1->error);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req2->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TRACKER, req2->error);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT(tr.getPendingRequests().empty());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());
  }
  {
    std::shared_ptr<UDPTrackerRequest> req1(
        createAnnounce("192.168.0.1", 6991, 0));

    tr.addRequest(req1);
    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    uint32_t transactionId = bittorrent::getIntParam(data, 12);
    tr.requestSent(now);

    uint64_t connectionId = 12345;
    rv = createConnectReply(data, sizeof(data), connectionId, transactionId);
    rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                         now);
    CPPUNIT_ASSERT_EQUAL(0, (int)rv);

    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE,
                         (int)bittorrent::getIntParam(data, 8));
    transactionId = bittorrent::getIntParam(data, 12);
    tr.requestSent(now);

    rv = createErrorReply(data, sizeof(data), transactionId, "announce error");
    rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                         now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TRACKER, req1->error);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT(tr.getPendingRequests().empty());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());
  }
}

void UDPTrackerClientTest::testTimeout()
{
  ssize_t rv;
  unsigned char data[100];
  std::string remoteAddr;
  uint16_t remotePort;
  Timer now;
  UDPTrackerClient tr;
  std::shared_ptr<UDPTrackerRequest> recvReq;

  {
    std::shared_ptr<UDPTrackerRequest> req1(
        createAnnounce("192.168.0.1", 6991, 0));
    std::shared_ptr<UDPTrackerRequest> req2(
        createAnnounce("192.168.0.1", 6991, 0));

    tr.addRequest(req1);
    tr.addRequest(req2);
    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    tr.requestSent(now);
    now.advance(20_s);
    // 15 seconds 1st stage timeout passed
    tr.handleTimeout(now);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT_EQUAL((size_t)3, tr.getPendingRequests().size());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());

    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    // CONNECT request was inserted
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    tr.requestSent(now);
    now.advance(65_s);
    // 60 seconds 2nd stage timeout passed
    tr.handleTimeout(now);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT(tr.getPendingRequests().empty());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TIMEOUT, req1->error);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req2->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TIMEOUT, req2->error);
  }
  {
    std::shared_ptr<UDPTrackerRequest> req1(
        createAnnounce("192.168.0.1", 6991, 0));

    tr.addRequest(req1);
    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((ssize_t)16, rv);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_CONNECT,
                         (int)bittorrent::getIntParam(data, 8));
    uint32_t transactionId = bittorrent::getIntParam(data, 12);
    tr.requestSent(now);

    uint64_t connectionId = 12345;
    rv = createConnectReply(data, sizeof(data), connectionId, transactionId);
    rv = tr.receiveReply(recvReq, data, rv, req1->remoteAddr, req1->remotePort,
                         now);
    CPPUNIT_ASSERT_EQUAL(0, (int)rv);

    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE,
                         (int)bittorrent::getIntParam(data, 8));
    tr.requestSent(now);
    now.advance(20_s);
    // 15 seconds 1st stage timeout passed
    tr.handleTimeout(now);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT_EQUAL((size_t)1, tr.getPendingRequests().size());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());

    rv = tr.createRequest(data, sizeof(data), remoteAddr, remotePort, now);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE,
                         (int)bittorrent::getIntParam(data, 8));
    tr.requestSent(now);
    now.advance(65_s);
    // 60 seconds 2nd stage timeout passed
    tr.handleTimeout(now);
    CPPUNIT_ASSERT(tr.getConnectRequests().empty());
    CPPUNIT_ASSERT(tr.getPendingRequests().empty());
    CPPUNIT_ASSERT(tr.getInflightRequests().empty());
    CPPUNIT_ASSERT_EQUAL((int)UDPT_STA_COMPLETE, req1->state);
    CPPUNIT_ASSERT_EQUAL((int)UDPT_ERR_TIMEOUT, req1->error);
  }
}

} // namespace aria2
