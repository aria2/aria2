#include "DefaultPeerStorage.h"

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "Exception.h"
#include "Peer.h"
#include "Option.h"
#include "BtRuntime.h"
#include "array_fun.h"

namespace aria2 {

class DefaultPeerStorageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPeerStorageTest);
  CPPUNIT_TEST(testCountAllPeer);
  CPPUNIT_TEST(testDeleteUnusedPeer);
  CPPUNIT_TEST(testAddPeer);
  CPPUNIT_TEST(testAddAndCheckoutPeer);
  CPPUNIT_TEST(testIsPeerAvailable);
  CPPUNIT_TEST(testCheckoutPeer);
  CPPUNIT_TEST(testReturnPeer);
  CPPUNIT_TEST(testOnErasingPeer);
  CPPUNIT_TEST(testAddBadPeer);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<BtRuntime> btRuntime;
  Option* option;

public:
  void setUp()
  {
    option = new Option();
    btRuntime.reset(new BtRuntime());
  }

  void tearDown() { delete option; }

  void testCountAllPeer();
  void testDeleteUnusedPeer();
  void testAddPeer();
  void testAddAndCheckoutPeer();
  void testIsPeerAvailable();
  void testCheckoutPeer();
  void testReturnPeer();
  void testOnErasingPeer();
  void testAddBadPeer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPeerStorageTest);

void DefaultPeerStorageTest::testCountAllPeer()
{
  DefaultPeerStorage ps;

  CPPUNIT_ASSERT_EQUAL((size_t)0, ps.countAllPeer());
  for (int i = 0; i < 2; ++i) {
    std::shared_ptr<Peer> peer(new Peer("192.168.0.1", 6889 + i));
    ps.addPeer(peer);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.countAllPeer());
  std::shared_ptr<Peer> peer = ps.checkoutPeer(1);
  CPPUNIT_ASSERT(peer);
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.countAllPeer());
  ps.returnPeer(peer);
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.countAllPeer());
}

void DefaultPeerStorageTest::testDeleteUnusedPeer()
{
  DefaultPeerStorage ps;

  auto peer1 = std::make_shared<Peer>("192.168.0.1", 6889);
  auto peer2 = std::make_shared<Peer>("192.168.0.2", 6889);
  auto peer3 = std::make_shared<Peer>("192.168.0.3", 6889);

  CPPUNIT_ASSERT(ps.addPeer(peer1));
  CPPUNIT_ASSERT(ps.addPeer(peer2));
  CPPUNIT_ASSERT(ps.addPeer(peer3));

  ps.deleteUnusedPeer(2);

  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  ps.deleteUnusedPeer(100);
  CPPUNIT_ASSERT(ps.getUnusedPeers().empty());
}

void DefaultPeerStorageTest::testAddPeer()
{
  DefaultPeerStorage ps;
  std::shared_ptr<BtRuntime> btRuntime(new BtRuntime());
  ps.setMaxPeerListSize(2);
  ps.setBtRuntime(btRuntime);

  auto peer1 = std::make_shared<Peer>("192.168.0.1", 6889);
  auto peer2 = std::make_shared<Peer>("192.168.0.2", 6889);
  auto peer3 = std::make_shared<Peer>("192.168.0.3", 6889);

  CPPUNIT_ASSERT(ps.addPeer(peer1));
  CPPUNIT_ASSERT(ps.addPeer(peer2));
  CPPUNIT_ASSERT(!ps.addPeer(peer3));

  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  CPPUNIT_ASSERT(!ps.addPeer(peer2));
  CPPUNIT_ASSERT(!ps.addPeer(peer3));

  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  CPPUNIT_ASSERT_EQUAL(peer1->getIPAddress(),
                       ps.checkoutPeer(1)->getIPAddress());
  CPPUNIT_ASSERT(!ps.addPeer(peer1));
}

void DefaultPeerStorageTest::testAddAndCheckoutPeer()
{
  DefaultPeerStorage ps;
  auto btRuntime = std::make_shared<BtRuntime>();

  ps.setBtRuntime(btRuntime);

  auto peer1 = std::make_shared<Peer>("192.168.0.1", 6889);
  // peer2 and peer3 have the same address and port deliberately.
  auto peer2 = std::make_shared<Peer>("192.168.0.2", 6889);
  auto peer3 = std::make_shared<Peer>("192.168.0.2", 6889);

  CPPUNIT_ASSERT(ps.addPeer(peer1));
  CPPUNIT_ASSERT(ps.addPeer(peer2));
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUnusedPeers().size());

  auto usedPeer = ps.checkoutPeer(1);

  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(peer1->getIPAddress(), usedPeer->getIPAddress());
  // Since peer1 is already used, this fails.
  CPPUNIT_ASSERT(!ps.addAndCheckoutPeer(peer1, 2));
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getUnusedPeers().size());

  // Since peer2 is not used yet, this succeeds.
  usedPeer = ps.addAndCheckoutPeer(peer3, 2);

  CPPUNIT_ASSERT_EQUAL((size_t)0, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(peer2->getIPAddress(), usedPeer->getIPAddress());
  CPPUNIT_ASSERT_EQUAL(peer3.get(), usedPeer.get());
}

void DefaultPeerStorageTest::testIsPeerAvailable()
{
  DefaultPeerStorage ps;
  ps.setBtRuntime(btRuntime);
  std::shared_ptr<Peer> peer1(new Peer("192.168.0.1", 6889));

  CPPUNIT_ASSERT(!ps.isPeerAvailable());
  ps.addPeer(peer1);
  CPPUNIT_ASSERT(ps.isPeerAvailable());
  CPPUNIT_ASSERT(ps.checkoutPeer(1));
  CPPUNIT_ASSERT(!ps.isPeerAvailable());
}

void DefaultPeerStorageTest::testCheckoutPeer()
{
  DefaultPeerStorage ps;

  auto peers = {
      std::make_shared<Peer>("192.168.0.1", 1000),
      std::make_shared<Peer>("192.168.0.2", 1000),
      std::make_shared<Peer>("192.168.0.3", 1000),
  };

  for (auto& peer : peers) {
    ps.addPeer(peer);
  }

  int i = 0;
  for (auto& peer : peers) {
    auto p = ps.checkoutPeer(i + 1);
    ++i;
    CPPUNIT_ASSERT_EQUAL(peer->getIPAddress(), p->getIPAddress());
  }
  CPPUNIT_ASSERT(!ps.checkoutPeer(peers.size() + 1));
}

void DefaultPeerStorageTest::testReturnPeer()
{
  DefaultPeerStorage ps;

  std::shared_ptr<Peer> peer1(new Peer("192.168.0.1", 0));
  peer1->allocateSessionResource(1_m, 10_m);
  std::shared_ptr<Peer> peer2(new Peer("192.168.0.2", 6889));
  peer2->allocateSessionResource(1_m, 10_m);
  std::shared_ptr<Peer> peer3(new Peer("192.168.0.1", 6889));
  peer2->setDisconnectedGracefully(true);
  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);
  for (int i = 1; i <= 3; ++i) {
    CPPUNIT_ASSERT(ps.checkoutPeer(i));
  }
  CPPUNIT_ASSERT_EQUAL((size_t)3, ps.getUsedPeers().size());

  ps.returnPeer(peer2); // peer2 removed from the container
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUsedPeers().size());
  CPPUNIT_ASSERT(ps.getUsedPeers().count(peer2) == 0);
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getDroppedPeers().size());

  ps.returnPeer(peer1); // peer1 is removed from the container
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getUsedPeers().size());
  CPPUNIT_ASSERT(ps.getUsedPeers().count(peer1) == 0);
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getDroppedPeers().size());
}

void DefaultPeerStorageTest::testOnErasingPeer()
{
  // test this
}

void DefaultPeerStorageTest::testAddBadPeer()
{
  DefaultPeerStorage ps;
  ps.addBadPeer("192.168.0.1");
  CPPUNIT_ASSERT(ps.isBadPeer("192.168.0.1"));
  CPPUNIT_ASSERT(!ps.isBadPeer("192.168.0.2"));
}

} // namespace aria2
