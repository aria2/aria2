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
  CPPUNIT_TEST(testCountPeer);
  CPPUNIT_TEST(testDeleteUnusedPeer);
  CPPUNIT_TEST(testAddPeer);
  CPPUNIT_TEST(testGetUnusedPeer);
  CPPUNIT_TEST(testIsPeerAvailable);
  CPPUNIT_TEST(testActivatePeer);
  CPPUNIT_TEST(testCalculateStat);
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

  void testCountPeer();
  void testDeleteUnusedPeer();
  void testAddPeer();
  void testGetUnusedPeer();
  void testIsPeerAvailable();
  void testActivatePeer();
  void testCalculateStat();
  void testReturnPeer();
  void testOnErasingPeer();
  void testAddBadPeer();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPeerStorageTest);

void DefaultPeerStorageTest::testCountPeer() {
  DefaultPeerStorage ps;

  CPPUNIT_ASSERT_EQUAL((size_t)0, ps.countPeer());

  SharedHandle<Peer> peer(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer);
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.countPeer());
}

void DefaultPeerStorageTest::testDeleteUnusedPeer() {
  DefaultPeerStorage ps;

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));
  SharedHandle<Peer> peer2(new Peer("192.168.0.2", 6889));
  SharedHandle<Peer> peer3(new Peer("192.168.0.3", 6889));

  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);

  ps.deleteUnusedPeer(2);

  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.countPeer());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.3"),
                       ps.getPeer("192.168.0.3", 6889)->getIPAddress());

  ps.addPeer(peer1);
  ps.addPeer(peer2);

  peer2->usedBy(1);

  ps.deleteUnusedPeer(3);

  // peer2 has been in use, so it did't deleted.
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.countPeer());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"),
                       ps.getPeer("192.168.0.2", 6889)->getIPAddress());
  
}

void DefaultPeerStorageTest::testAddPeer() {
  DefaultPeerStorage ps;
  SharedHandle<BtRuntime> btRuntime(new BtRuntime());
  ps.setMaxPeerListSize(3);
  ps.setBtRuntime(btRuntime);

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));
  SharedHandle<Peer> peer2(new Peer("192.168.0.2", 6889));
  SharedHandle<Peer> peer3(new Peer("192.168.0.3", 6889));

  ps.addPeer(peer1);
  ps.addPeer(peer2);
  ps.addPeer(peer3);

  CPPUNIT_ASSERT_EQUAL((size_t)3, ps.countPeer());

  // this returns false, because peer1 is already in the container
  CPPUNIT_ASSERT_EQUAL(false, ps.addPeer(peer1));
  // the number of peers doesn't change.
  CPPUNIT_ASSERT_EQUAL((size_t)3, ps.countPeer());

  SharedHandle<Peer> peer4(new Peer("192.168.0.4", 6889));

  peer1->usedBy(1);
  CPPUNIT_ASSERT(ps.addPeer(peer4));
  // peer2 was deleted. While peer1 is oldest, its cuid is not 0.
  CPPUNIT_ASSERT_EQUAL((size_t)3, ps.countPeer());
  CPPUNIT_ASSERT(std::find_if(ps.getPeers().begin(), ps.getPeers().end(),
                              derefEqual(peer2)) == ps.getPeers().end());

  SharedHandle<Peer> peer5(new Peer("192.168.0.4", 0));

  peer5->setPort(6889);

  // this returns false because the peer which has same ip and port
  // has already added
  CPPUNIT_ASSERT_EQUAL(false, ps.addPeer(peer5));

  SharedHandle<Peer> pa[] = {
    SharedHandle<Peer>(new Peer("192.168.0.4", 6889)),
    SharedHandle<Peer>(new Peer("192.168.0.5", 6889)),
    SharedHandle<Peer>(new Peer("192.168.0.6", 6889)),
    SharedHandle<Peer>(new Peer("192.168.0.7", 6889)),
    SharedHandle<Peer>(new Peer("192.168.0.8", 6889))
  };
  std::vector<SharedHandle<Peer> > peers(vbegin(pa), vend(pa));
  ps.addPeer(peers);
  // peers[0] is not added because it has been already added.
  // peers[1], peers[2] and peers[3] are going to be added.  peers[4]
  // is not added because DefaultPeerStorage::addPeer() limits the
  // number of peers to add.  Finally, unused peers are removed from
  // back and size 3 vector is made.
  CPPUNIT_ASSERT_EQUAL((size_t)3, ps.countPeer());
  CPPUNIT_ASSERT(std::find_if(ps.getPeers().begin(), ps.getPeers().end(),
                              derefEqual(peers[2])) != ps.getPeers().end());
  CPPUNIT_ASSERT(std::find_if(ps.getPeers().begin(), ps.getPeers().end(),
                              derefEqual(peers[3])) != ps.getPeers().end());
}

void DefaultPeerStorageTest::testGetUnusedPeer() {
  DefaultPeerStorage ps;
  ps.setBtRuntime(btRuntime);

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       ps.getUnusedPeer()->getIPAddress());

  peer1->usedBy(1);

  CPPUNIT_ASSERT(!ps.getUnusedPeer());

  peer1->resetStatus();
  peer1->startBadCondition();

  CPPUNIT_ASSERT(!ps.getUnusedPeer());
}

void DefaultPeerStorageTest::testIsPeerAvailable() {
  DefaultPeerStorage ps;
  ps.setBtRuntime(btRuntime);

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  CPPUNIT_ASSERT_EQUAL(true, ps.isPeerAvailable());

  peer1->usedBy(1);

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());

  peer1->resetStatus();

  peer1->startBadCondition();

  CPPUNIT_ASSERT_EQUAL(false, ps.isPeerAvailable());
}

void DefaultPeerStorageTest::testActivatePeer() {
  DefaultPeerStorage ps;

  {
    std::vector<SharedHandle<Peer> > peers;
    ps.getActivePeers(peers);
    CPPUNIT_ASSERT_EQUAL((size_t)0, peers.size());
  }

  SharedHandle<Peer> peer1(new Peer("192.168.0.1", 6889));

  ps.addPeer(peer1);

  {
    std::vector<SharedHandle<Peer> > activePeers;
    ps.getActivePeers(activePeers);

    CPPUNIT_ASSERT_EQUAL((size_t)0, activePeers.size());
  }
  {
    peer1->allocateSessionResource(1024*1024, 1024*1024*10);

    std::vector<SharedHandle<Peer> > activePeers;
    ps.getActivePeers(activePeers);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, activePeers.size());
  }
}

void DefaultPeerStorageTest::testCalculateStat() {
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

  ps.returnPeer(peer2); // peer2 removed from the container
  CPPUNIT_ASSERT_EQUAL((size_t)2, ps.getPeers().size());
  CPPUNIT_ASSERT(std::find_if(ps.getPeers().begin(), ps.getPeers().end(),
                              derefEqual(peer2)) == ps.getPeers().end());
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getDroppedPeers().size());

  ps.returnPeer(peer1); // peer1 is removed from the container
  CPPUNIT_ASSERT_EQUAL((size_t)1, ps.getPeers().size());
  CPPUNIT_ASSERT(std::find_if(ps.getPeers().begin(), ps.getPeers().end(),
                              derefEqual(peer1)) == ps.getPeers().end());
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
