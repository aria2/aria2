#include "DefaultPeerStorage.h"
#include "DefaultBtContext.h"
#include "Util.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultPeerStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPeerStorageTest);
  CPPUNIT_TEST(testCountPeer);
  CPPUNIT_TEST(testDeleteUnusedPeer);
  CPPUNIT_TEST(testAddPeer);
  CPPUNIT_TEST(testGetUnusedPeer);
  CPPUNIT_TEST(testIsPeerAvailable);
  CPPUNIT_TEST(testActivatePeer);
  CPPUNIT_TEST(testCalculateStat);
  CPPUNIT_TEST(testReturnPeer);
  CPPUNIT_TEST(testOnErasingPeer);
  CPPUNIT_TEST_SUITE_END();
private:
  BtContextHandle btContext;
  BtRuntimeHandle btRuntime;
  Option* option;
public:
  DefaultPeerStorageTest():btContext(0) {}

  void setUp() {
    btContext = BtContextHandle(new DefaultBtContext());
    btContext->load("test.torrent");
    option = new Option();
    btRuntime = BtRuntimeHandle(new BtRuntime());
  }

  void testCountPeer();
  void testDeleteUnusedPeer();
  void testAddPeer();
  void testGetUnusedPeer();
  void testIsPeerAvailable();
  void testActivatePeer();
  void testCalculateStat();
  void testReturnPeer();
  void testOnErasingPeer();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPeerStorageTest);

void DefaultPeerStorageTest::testCountPeer() {
  DefaultPeerStorage ps(btContext, option);

  CPPUNIT_ASSERT_EQUAL((int32_t)0,
		       ps.countPeer());

  PeerHandle peer(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer);
  CPPUNIT_ASSERT_EQUAL((int32_t)1,
		       ps.countPeer());
}

void DefaultPeerStorageTest::testDeleteUnusedPeer() {
  DefaultPeerStorage ps(btContext, option);

  PeerHandle peer1(new Peer("192.168.0.1", 6889));
  PeerHandle peer2(new Peer("192.168.0.2", 6889));
  PeerHandle peer3(new Peer("192.168.0.3", 6889));

  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);

  ps.deleteUnusedPeer(2);

  CPPUNIT_ASSERT_EQUAL((int32_t)1, ps.countPeer());
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.3"),
		       ps.getPeer("192.168.0.3", 6889)->ipaddr);

  ps.addPeer(peer1);
  ps.addPeer(peer2);

  peer2->cuid = 1;

  ps.deleteUnusedPeer(3);

  // peer2 has been in use, so it did't deleted.
  CPPUNIT_ASSERT_EQUAL((int32_t)1, ps.countPeer());
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.2"),
		       ps.getPeer("192.168.0.2", 6889)->ipaddr);
  
}

void DefaultPeerStorageTest::testAddPeer() {
  DefaultPeerStorage ps(btContext, option);

  PeerHandle peer1(new Peer("192.168.0.1", 6889));
  PeerHandle peer2(new Peer("192.168.0.2", 6889));
  PeerHandle peer3(new Peer("192.168.0.3", 6889));

  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);

  CPPUNIT_ASSERT_EQUAL((int32_t)3, ps.countPeer());

  // this returns false, because peer1 is already in the container
  CPPUNIT_ASSERT_EQUAL(false, ps.addPeer(peer1));
  // the number of peers doesn't change.
  CPPUNIT_ASSERT_EQUAL((int32_t)3, ps.countPeer());

  ps.setMaxPeerListSize(3);

  PeerHandle peer4(new Peer("192.168.0.4", 6889));

  peer1->cuid = 1;
  CPPUNIT_ASSERT(ps.addPeer(peer4));
  // peer2 was deleted. While peer1 is oldest, its cuid is not 0.
  CPPUNIT_ASSERT_EQUAL((int32_t)3, ps.countPeer());
  CPPUNIT_ASSERT(find(ps.getPeers().begin(), ps.getPeers().end(), peer2) == ps.getPeers().end());

  PeerHandle peer5(new Peer("192.168.0.4", 0));

  peer5->port = 6889;

  // this returns false because the peer which has same ip and port has already added
  CPPUNIT_ASSERT_EQUAL(false, ps.addPeer(peer5));
}

void DefaultPeerStorageTest::testGetUnusedPeer() {
  DefaultPeerStorage ps(btContext, option);
  ps.setBtRuntime(btRuntime);

  PeerHandle peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  CPPUNIT_ASSERT_EQUAL(string("192.168.0.1"),
		       ps.getUnusedPeer()->ipaddr);

  peer1->cuid = 1;

  CPPUNIT_ASSERT(ps.getUnusedPeer().isNull());

  peer1->resetStatus();
  peer1->startBadCondition();

  CPPUNIT_ASSERT(ps.getUnusedPeer().isNull());
}

void DefaultPeerStorageTest::testIsPeerAvailable() {
  DefaultPeerStorage ps(btContext, option);
  ps.setBtRuntime(btRuntime);

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());

  PeerHandle peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  CPPUNIT_ASSERT_EQUAL(true, ps.isPeerAvailable());

  peer1->cuid = 1;

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());

  peer1->resetStatus();

  peer1->startBadCondition();

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());
}

void DefaultPeerStorageTest::testActivatePeer() {
  DefaultPeerStorage ps(btContext, option);

  CPPUNIT_ASSERT_EQUAL((size_t)0, ps.getActivePeers().size());

  PeerHandle peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  Peers activePeer = ps.getActivePeers();

  CPPUNIT_ASSERT_EQUAL((size_t)0, ps.getActivePeers().size());

  peer1->activate();

  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getActivePeers().size());
}

void DefaultPeerStorageTest::testCalculateStat() {
}

void DefaultPeerStorageTest::testReturnPeer()
{
  DefaultPeerStorage ps(btContext, option);

  PeerHandle peer1(new Peer("192.168.0.1", 0));
  PeerHandle peer2(new Peer("192.168.0.2", 6889));
  PeerHandle peer3(new Peer("192.168.0.1", 6889));
  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);

  ps.returnPeer(peer2);
  // peer2 is moved to the end of container
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.2"),
		       ps.getPeers().back()->ipaddr);

  ps.returnPeer(peer1); // peer1 is removed from the container
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getPeers().size());
  CPPUNIT_ASSERT(find(ps.getPeers().begin(), ps.getPeers().end(), peer1) == ps.getPeers().end());
}

void DefaultPeerStorageTest::testOnErasingPeer()
{
  // test this
}
