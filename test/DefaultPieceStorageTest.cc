#include "DefaultPieceStorage.h"
#include "DefaultBtContext.h"
#include "Util.h"
#include "Exception.h"
#include "FixedNumberRandomizer.h"
#include "BitfieldManFactory.h"
#include "Piece.h"
#include "Peer.h"
#include "Option.h"
#include "MockBtContext.h"
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
  CPPUNIT_TEST(testGetMissingPiece_fileEntry);
  CPPUNIT_TEST(testCancelPiece);
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

  void tearDown()
  {
    delete option;
    option = 0;
  }

  void testGetTotalLength();
  void testGetMissingPiece();
  void testGetMissingFastPiece();
  void testHasMissingPiece();
  void testCompletePiece();
  void testGetPiece();
  void testGetPieceInUsedPieces();
  void testGetPieceCompletedPiece();
  void testGetMissingPiece_fileEntry();
  void testCancelPiece();
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

  CPPUNIT_ASSERT_EQUAL((int64_t)0,
		       pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)128,
		       pss.getCompletedLength());

  PieceHandle incompletePiece = pss.getMissingPiece(peer);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)256,
		       pss.getCompletedLength());
}

void DefaultPieceStorageTest::testGetPiece() {
  DefaultPieceStorage pss(btContext, option);
  
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces() {
  DefaultPieceStorage pss(btContext, option);
  PieceHandle piece = PieceHandle(new Piece(0, 128));
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece() {
  DefaultPieceStorage pss(btContext, option);
  PieceHandle piece = PieceHandle(new Piece(0, 128));
  pss.completePiece(piece);
  PieceHandle pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(true, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetMissingPiece_fileEntry()
{
  // - 32KB
  // +--------+
  // |11111222|
  int32_t pieceLength = 256*1024;
  int64_t totalLength = 1*pieceLength;
  int32_t blockLength = 16*1024;
  Strings uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  Strings uris2;
  uris2.push_back("http://localhost/src/file2.txt");
  FileEntryHandle file1 = new FileEntry("src/file1.txt", 150*1024, 0/*, uris1*/);
  FileEntryHandle file2 = new FileEntry("src/file2.txt", 106*1024, file1->getLength() /*, uris2*/);

  MockBtContextHandle dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->addFileEntry(file1);
  dctx->addFileEntry(file2);

  DefaultPieceStorageHandle ps = new DefaultPieceStorage(dctx, option);

  PieceHandle p = ps->getMissingPiece(file1);
  CPPUNIT_ASSERT(!p.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, p->getIndex());

  for(int32_t i = 0; i < 9; ++i) {
    p->completeBlock(i);
  }
  PieceHandle subPiece = new Piece(9, blockLength, 1);
  p->addSubPiece(subPiece);

  ps->cancelPiece(p);

  // Piece index = 0 should be retrieved again because the part of file1 is
  // not complete
  PieceHandle p2 = ps->getMissingPiece(file1);
  CPPUNIT_ASSERT(!p2.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, p2->getIndex());
 
  // Make the part of file1 complete
  for(int32_t i = 0; i < 6*1024; ++i) {
    p2->getSubPiece(9)->completeBlock(i);
  }
  ps->cancelPiece(p2);

  // Null Piece should be retrieved
  CPPUNIT_ASSERT(ps->getMissingPiece(file1).isNull());

  // Next, I retrive the piece giving file2
  PieceHandle p3 = ps->getMissingPiece(file2);
  CPPUNIT_ASSERT(!p3.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, p3->getIndex());

  // Make the part of file2 complete
  for(int32_t i = 6*1024; i < 16*1024; ++i) {
    p3->getSubPiece(9)->completeBlock(i);
  }
  for(int32_t i = 10; i < 16; ++i) {
    p3->completeBlock(i);
  }
  ps->cancelPiece(p3);
  
  // Null Piece should be retrieved
  CPPUNIT_ASSERT(ps->getMissingPiece(file2).isNull());
}

void DefaultPieceStorageTest::testCancelPiece()
{
  int32_t pieceLength = 256*1024;
  int64_t totalLength = 32*pieceLength; // <-- make the number of piece greater than END_GAME_PIECE_NUM
  int32_t blockLength = 16*1024;
  Strings uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  FileEntryHandle file1 = new FileEntry("src/file1.txt", totalLength, 0 /*, uris1*/);

  MockBtContextHandle dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->addFileEntry(file1);

  DefaultPieceStorageHandle ps = new DefaultPieceStorage(dctx, option);

  PieceHandle p = ps->getMissingPiece(file1);
  
  PieceHandle subPiece = new Piece(0, blockLength, 1);
  subPiece->completeBlock(0);
  p->addSubPiece(subPiece);

  ps->cancelPiece(p);

  // See the sub piece is also hibernated...
  PieceHandle p2 = ps->getMissingPiece(file1);

  CPPUNIT_ASSERT(!p2->getSubPiece(0).isNull());
  
}
