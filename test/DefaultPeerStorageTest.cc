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

class DefaultPeerStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPeerStorageTest);
  CPPUNIT_TEST(testCountAllPeer);
  CPPUNIT_TEST(testDeleteUnusedPeer);
  CPPUNIT_TEST(testAddPeer);
  CPPUNIT_TEST(testIsPeerAvailable);
  CPPUNIT_TEST(testCheckoutPeer);
  CPPUNIT_TEST(testReturnPeer);
  CPPUNIT_TEST(testOnErasingPeer);
  CPPUNIT_TEST(testAddBadPeer);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<BtRuntime> btRuntime;
  Option* option;
public:
  void setUp() {
    option = new Option();
    btRuntime.reset(new BtRuntime());
  }

  void tearDown() {
    delete option;
  }

  void testCountAllPeer();
  void testDeleteUnusedPeer();
  void testAddPeer();
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
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Peer> peer(new Peer("192.168.0.1", 6889+i));
    ps.addPeer(peer);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.countAllPeer());
  SharedHandle<Peer> peer = ps.checkoutPeer(1);
  CPPUNIT_ASSERT(peer);
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.countAllPeer());
  ps.returnPeer(peer);
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.countAllPeer());
}

void DefaultPeerStorageTest::testDeleteUnusedPeer()
{
  DefaultPeerStorage ps;

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));
  SharedHandle<Peer> peer2(new Peer("192.168.0.2", 6889));
  SharedHandle<Peer> peer3(new Peer("192.168.0.3", 6889));

  CPPUNIT_ASSERT(ps.addPeer(peer1));
  CPPUNIT_ASSERT(ps.addPeer(peer2));
  CPPUNIT_ASSERT(ps.addPeer(peer3));

  ps.deleteUnusedPeer(2);

  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  ps.deleteUnusedPeer(100);
  CPPUNIT_ASSERT(ps.getUnusedPeers().empty());
}

void DefaultPeerStorageTest::testAddPeer()
{
  DefaultPeerStorage ps;
  SharedHandle<BtRuntime> btRuntime(new BtRuntime());
  ps.setMaxPeerListSize(2);
  ps.setBtRuntime(btRuntime);

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));
  SharedHandle<Peer> peer2(new Peer("192.168.0.2", 6889));
  SharedHandle<Peer> peer3(new Peer("192.168.0.3", 6889));

  CPPUNIT_ASSERT(ps.addPeer(peer1));
  CPPUNIT_ASSERT(ps.addPeer(peer2));
  CPPUNIT_ASSERT(ps.addPeer(peer3));

  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  CPPUNIT_ASSERT(!ps.addPeer(peer2));
  CPPUNIT_ASSERT(ps.addPeer(peer1));

  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getUnusedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       ps.getUnusedPeers()[0]->getIPAddress());

  CPPUNIT_ASSERT_EQUAL(peer1->getIPAddress(),
                       ps.checkoutPeer(1)->getIPAddress());
  CPPUNIT_ASSERT(!ps.addPeer(peer1));
}

void DefaultPeerStorageTest::testIsPeerAvailable() {
  DefaultPeerStorage ps;
  ps.setBtRuntime(btRuntime);
  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));

  CPPUNIT_ASSERT(!ps.isPeerAvailable());
  ps.addPeer(peer1);
  CPPUNIT_ASSERT(ps.isPeerAvailable());
  CPPUNIT_ASSERT(ps.checkoutPeer(1));
  CPPUNIT_ASSERT(!ps.isPeerAvailable());
}

void DefaultPeerStorageTest::testCheckoutPeer()
{
  DefaultPeerStorage ps;
  SharedHandle<Peer> peers[] = {
    SharedHandle<Peer>(new Peer("192.168.0.1", 1000)),
    SharedHandle<Peer>(new Peer("192.168.0.2", 1000)),
    SharedHandle<Peer>(new Peer("192.168.0.3", 1000))
  };
  int len = A2_ARRAY_LEN(peers);
  for(int i = 0; i < len; ++i) {
    ps.addPeer(peers[i]);
  }
  for(int i = 0; i < len; ++i) {
    SharedHandle<Peer> peer = ps.checkoutPeer(i+1);
    CPPUNIT_ASSERT_EQUAL(peers[len-i-1]->getIPAddress(), peer->getIPAddress());
  }
  CPPUNIT_ASSERT(!ps.checkoutPeer(len+1));
}

void DefaultPeerStorageTest::testReturnPeer()
{
  DefaultPeerStorage ps;

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 0));
  peer1->allocateSessionResource(1024*1024, 1024*1024*10);
  SharedHandle<Peer> peer2(new Peer("192.168.0.2", 6889));
  peer2->allocateSessionResource(1024*1024, 1024*1024*10);
  SharedHandle<Peer> peer3(new Peer("192.168.0.1", 6889));
  peer2->setDisconnectedGracefully(true);
  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);
  for(int i = 1; i <= 3; ++i) {
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
