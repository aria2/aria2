#include "DefaultPieceStorage.h"
#include "DefaultBtContext.h"
#include "Util.h"
#include "Exception.h"
#include "FixedNumberRandomizer.h"
#include "BitfieldManFactory.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultPieceStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPieceStorageTest);
  CPPUNIT_TEST(testGetTotalLength);
  CPPUNIT_TEST(testGetMissingPiece);
  CPPUNIT_TEST(testGetMissingFastPiece);
  CPPUNIT_TEST(testHasMissingPiece);
  CPPUNIT_TEST(testCompletePiece);
  CPPUNIT_TEST(testGetPiece);
  CPPUNIT_TEST(testGetPieceInUsedPieces);
  CPPUNIT_TEST(testGetPieceCompletedPiece);
  CPPUNIT_TEST_SUITE_END();
private:
  BtContextHandle btContext;
  PeerHandle peer;
  Option* option;
public:
  DefaultPieceStorageTest():btContext(0), peer(0) {
    FixedNumberRandomizer* randomizer = new FixedNumberRandomizer();
    randomizer->setFixedNumber(0);
    BitfieldManFactory::setDefaultRandomizer(randomizer);
  }

  void setUp() {
    btContext = BtContextHandle(new DefaultBtContext());
    btContext->load("test.torrent");
    peer = PeerHandle(new Peer("192.168.0.1", 6889,
			       btContext->getPieceLength(),
			       btContext->getTotalLength()));
    option = new Option();
  }

  void testGetTotalLength();
  void testGetMissingPiece();
  void testGetMissingFastPiece();
  void testHasMissingPiece();
  void testCompletePiece();
  void testGetPiece();
  void testGetPieceInUsedPieces();
  void testGetPieceCompletedPiece();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPieceStorageTest);

void DefaultPieceStorageTest::testGetTotalLength() {
  DefaultPieceStorage pss(btContext, option);

  CPPUNIT_ASSERT_EQUAL((long long int)384,
		       pss.getTotalLength());
}

void DefaultPieceStorageTest::testGetMissingPiece() {
  DefaultPieceStorage pss(btContext, option);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  PieceHandle piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(string("piece: index=0, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(string("piece: index=1, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(string("piece: index=2, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT(piece.isNull());
}

void DefaultPieceStorageTest::testGetMissingFastPiece() {
  DefaultPieceStorage pss(btContext, option);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addFastSetIndex(2);

  PieceHandle piece = pss.getMissingFastPiece(peer);
  CPPUNIT_ASSERT_EQUAL(string("piece: index=2, length=128"),
		       piece->toString());
}

void DefaultPieceStorageTest::testHasMissingPiece() {
  DefaultPieceStorage pss(btContext, option);

  CPPUNIT_ASSERT(!pss.hasMissingPiece(peer));
  
  peer->setAllBitfield();

  CPPUNIT_ASSERT(pss.hasMissingPiece(peer));
}

void DefaultPieceStorageTest::testCompletePiece() {
  DefaultPieceStorage pss(btContext, option);
  pss.setEndGamePieceNum(0);

    peer->setAllBitfield();

  PieceHandle piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(string("piece: index=0, length=128"),
		       piece->toString());

  CPPUNIT_ASSERT_EQUAL((long long int)0,
		       pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((long long int)128,
		       pss.getCompletedLength());

}

void DefaultPieceStorageTest::testGetPiece() {
  DefaultPieceStorage pss(btContext, option);
  
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL(0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces() {
  DefaultPieceStorage pss(btContext, option);
  PieceHandle piece = PieceHandle(new Piece(0, 128));
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL(0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece() {
  DefaultPieceStorage pss(btContext, option);
  PieceHandle piece = PieceHandle(new Piece(0, 128));
  pss.completePiece(piece);
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL(0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(true, pieceGot->pieceComplete());
}
