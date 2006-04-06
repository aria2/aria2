#include "TorrentMan.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class TorrentManTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TorrentManTest);
  /*
  CPPUNIT_TEST(testUpdatePeers);
  //CPPUNIT_TEST(testUpdatePeer);
  CPPUNIT_TEST(testGetPeer);
  CPPUNIT_TEST(testGetMissingPiece);
  CPPUNIT_TEST(testCancelPiece);
  CPPUNIT_TEST(testAddPeer);
  */
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testUpdatePeers();
  //void testUpdatePeer();
  void testGetPeer();
  void testGetMissingPiece();
  void testCancelPiece();
  void testAddPeer();
};


CPPUNIT_TEST_SUITE_REGISTRATION( TorrentManTest );

Peers createPeers() {
  Peers peers;
  Peer* peer1 = new Peer("192.168.0.1", 6881, 512*1024, 5242870);
  peer1->entryId = 1;
  Peer* peer2 = new Peer("192.168.0.2", 6881, 512*1024, 5242870);
  peer2->entryId = 2;
  Peer* peer3 = new Peer("192.168.0.3", 6881, 512*1024, 5242870);
  peer3->entryId = 3;
  peers.push_back(peer1);
  peers.push_back(peer2);
  peers.push_back(peer3);
  return peers;
}
/*
void TorrentManTest::testUpdatePeers() {
  TorrentMan tm;
  Peers peers = createPeers();
  tm.updatePeers(peers);
  const Peers& peersGot = tm.getPeers();
  Peers::const_iterator itr = peersGot.begin();
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.1"), (*itr)->ipaddr);
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.2"), (*itr)->ipaddr);
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.3"), (*itr)->ipaddr);
  itr++;
}
*/
/*
void TorrentManTest::testUpdatePeer() {
  TorrentMan tm;
  Peers peers = createPeers();
  tm.updatePeers(peers);
  Peer* peer = tm.getPeer(1);
  peer->amChocking = true;
  peer->amInterested = true;
  tm.updatePeer(peer);
  
  Peers::const_iterator itr = tm.getPeers().begin();
  CPPUNIT_ASSERT_EQUAL(3, (int)tm.getPeers().size());
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.1"), itr->ipaddr);
  CPPUNIT_ASSERT_EQUAL(true, itr->amChocking);
  CPPUNIT_ASSERT_EQUAL(true, itr->amInterested);
  CPPUNIT_ASSERT_EQUAL(1, itr->cuid);
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.2"), itr->ipaddr);
  CPPUNIT_ASSERT_EQUAL(false, itr->amChocking);
  CPPUNIT_ASSERT_EQUAL(false, itr->amInterested);
  CPPUNIT_ASSERT_EQUAL(0, itr->cuid);
  itr++;
  CPPUNIT_ASSERT_EQUAL(string("192.168.0.3"), itr->ipaddr);
  CPPUNIT_ASSERT_EQUAL(false, itr->amChocking);
  CPPUNIT_ASSERT_EQUAL(false, itr->amInterested);
  CPPUNIT_ASSERT_EQUAL(0, itr->cuid);
  itr++;
}
*/
/*
void TorrentManTest::testGetPeer() {
  TorrentMan tm;
  Peers peers = createPeers();
  tm.updatePeers(peers);
  CPPUNIT_ASSERT(tm.getPeer() != Peer::nullPeer);
  CPPUNIT_ASSERT(tm.getPeer() != Peer::nullPeer);
  CPPUNIT_ASSERT(tm.getPeer() != Peer::nullPeer);
  CPPUNIT_ASSERT(tm.getPeer() == Peer::nullPeer);
}

void TorrentManTest::testGetMissingPiece() {
  TorrentMan tm;
  tm.pieceLength = 512*1024;
  tm.pieces = 10;
  tm.totalSize = 5242870;
  tm.initBitfield();

  unsigned char peerBitfield[2] = { 0xff, 0xff };
  Piece piece1 = tm.getMissingPiece(peerBitfield, 2);
  CPPUNIT_ASSERT_EQUAL(0, piece1.getIndex());
  CPPUNIT_ASSERT_EQUAL(512*1024, piece1.getLength());

  Piece piece2 = tm.getMissingPiece(peerBitfield, 2);
  CPPUNIT_ASSERT_EQUAL(1, piece2.getIndex());
  CPPUNIT_ASSERT_EQUAL(512*1024, piece2.getLength());
 
  tm.completePiece(piece1);

  int len = tm.getBitfieldLength();
  const unsigned char* bitfield = tm.getBitfield();
  CPPUNIT_ASSERT_EQUAL(2, len);
  CPPUNIT_ASSERT(bitfield[0]&(1 << 7));
  for(int i = 0; i < 7; i++) {
    CPPUNIT_ASSERT(!(bitfield[0]&(1 << i)));
  }

  tm.completePiece(piece2);
  bitfield = tm.getBitfield();
  CPPUNIT_ASSERT_EQUAL(2, len);
  CPPUNIT_ASSERT(bitfield[0]&(1 << 7));
  CPPUNIT_ASSERT(bitfield[0]&(1 << 6));
  for(int i = 0; i < 6; i++) {
    CPPUNIT_ASSERT(!(bitfield[0]&(1 << i)));
  }

  for(int i = 0; i < 8; i++) {
    CPPUNIT_ASSERT(!IS_NULL_PIECE(tm.getMissingPiece(peerBitfield, 2)));
  }
  CPPUNIT_ASSERT(IS_NULL_PIECE(tm.getMissingPiece(peerBitfield, 2)));
}

void TorrentManTest::testCancelPiece() {
  TorrentMan tm;
  tm.pieceLength = 512*1024;
  tm.pieces = 10;
  tm.totalSize = 5242870;
  tm.initBitfield();

  unsigned char peerBitfield[2] = { 0xff, 0xff };
  Piece piece = tm.getMissingPiece(peerBitfield, 2);
  CPPUNIT_ASSERT_EQUAL(0, piece.getIndex());
  CPPUNIT_ASSERT_EQUAL(512*1024, piece.getLength());

  tm.cancelPiece(piece);
  int len = tm.getBitfieldLength();
  const unsigned char* bitfield = tm.getBitfield();
  for(int i = 0; i < 8; i++) {
    CPPUNIT_ASSERT(!(bitfield[0]&(1 << i)));
  }  
}

void TorrentManTest::testAddPeer() {
  TorrentMan tm;
  Peers peers = createPeers();
  tm.updatePeers(peers);

  // try to add already added peer
  Peer* dupPeer = new Peer("192.168.0.2", 6881);
  CPPUNIT_ASSERT(!tm.addPeer(dupPeer));
  CPPUNIT_ASSERT_EQUAL(3, (int)tm.getPeers().size());

  // duplicate flag on
  CPPUNIT_ASSERT(tm.addPeer(dupPeer, true));
  CPPUNIT_ASSERT_EQUAL(4, (int)tm.getPeers().size());

  // cannot add error peer even though duplicte flag turns on
  dupPeer->error = 1;
  Peer* dupPeer2 = new Peer("192.168.0.2", 6881);
  CPPUNIT_ASSERT(!tm.addPeer(dupPeer2, true));
  CPPUNIT_ASSERT_EQUAL(4, (int)tm.getPeers().size());

  // try to add new peer
  Peer* newPeer = new Peer("10.1.0.1", 6881);
  CPPUNIT_ASSERT(tm.addPeer(newPeer));
  CPPUNIT_ASSERT_EQUAL(5, (int)tm.getPeers().size());

}
*/
