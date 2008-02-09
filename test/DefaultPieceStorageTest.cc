#include "DefaultPieceStorage.h"
#include "DefaultBtContext.h"
#include "Util.h"
#include "Exception.h"
#include "FixedNumberRandomizer.h"
#include "BitfieldManFactory.h"
#include "Piece.h"
#include "Peer.h"
#include "Option.h"
#include "FileEntry.h"
#include "MockBtContext.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  CPPUNIT_TEST(testMarkPiecesDone);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<BtContext> btContext;
  SharedHandle<Peer> peer;
  Option* option;
public:
  DefaultPieceStorageTest():btContext(0), peer(0) {
    FixedNumberRandomizer* randomizer = new FixedNumberRandomizer();
    randomizer->setFixedNumber(0);
    BitfieldManFactory::setDefaultRandomizer(randomizer);
  }

  void setUp() {
    btContext = new DefaultBtContext();
    btContext->load("test.torrent");
    peer = new Peer("192.168.0.1", 6889);
    peer->allocateSessionResource(btContext->getPieceLength(),
				  btContext->getTotalLength());
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
  void testMarkPiecesDone();
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
  SharedHandle<Piece> piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
		       piece->toString());
  piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT(piece.isNull());
}

void DefaultPieceStorageTest::testGetMissingFastPiece() {
  DefaultPieceStorage pss(btContext, option);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(2);

  SharedHandle<Piece> piece = pss.getMissingFastPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
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

  SharedHandle<Piece> piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
		       piece->toString());

  CPPUNIT_ASSERT_EQUAL((int64_t)0,
		       pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)128,
		       pss.getCompletedLength());

  SharedHandle<Piece> incompletePiece = pss.getMissingPiece(peer);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)256,
		       pss.getCompletedLength());
}

void DefaultPieceStorageTest::testGetPiece() {
  DefaultPieceStorage pss(btContext, option);
  
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces() {
  DefaultPieceStorage pss(btContext, option);
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((int32_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int32_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece() {
  DefaultPieceStorage pss(btContext, option);
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  pss.completePiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
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
  std::deque<std::string> uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  std::deque<std::string> uris2;
  uris2.push_back("http://localhost/src/file2.txt");
  SharedHandle<FileEntry> file1 = new FileEntry("src/file1.txt", 150*1024, 0/*, uris1*/);
  SharedHandle<FileEntry> file2 = new FileEntry("src/file2.txt", 106*1024, file1->getLength() /*, uris2*/);

  SharedHandle<MockBtContext> dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->addFileEntry(file1);
  dctx->addFileEntry(file2);

  SharedHandle<DefaultPieceStorage> ps = new DefaultPieceStorage(dctx, option);

  SharedHandle<Piece> p = ps->getMissingPiece(file1);
  CPPUNIT_ASSERT(!p.isNull());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, p->getIndex());

  for(int32_t i = 0; i < 9; ++i) {
    p->completeBlock(i);
  }
  SharedHandle<Piece> subPiece = new Piece(9, blockLength, 1);
  p->addSubPiece(subPiece);

  ps->cancelPiece(p);

  // Piece index = 0 should be retrieved again because the part of file1 is
  // not complete
  SharedHandle<Piece> p2 = ps->getMissingPiece(file1);
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
  SharedHandle<Piece> p3 = ps->getMissingPiece(file2);
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
  std::deque<std::string> uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  SharedHandle<FileEntry> file1 = new FileEntry("src/file1.txt", totalLength, 0 /*, uris1*/);

  SharedHandle<MockBtContext> dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->addFileEntry(file1);

  SharedHandle<DefaultPieceStorage> ps = new DefaultPieceStorage(dctx, option);

  SharedHandle<Piece> p = ps->getMissingPiece(file1);
  
  SharedHandle<Piece> subPiece = new Piece(0, blockLength, 1);
  subPiece->completeBlock(0);
  p->addSubPiece(subPiece);

  ps->cancelPiece(p);

  // See the sub piece is also hibernated...
  SharedHandle<Piece> p2 = ps->getMissingPiece(file1);

  CPPUNIT_ASSERT(!p2->getSubPiece(0).isNull());  
}

void DefaultPieceStorageTest::testMarkPiecesDone()
{
  int32_t pieceLength = 256*1024;
  int64_t totalLength = 4*1024*1024;
  SharedHandle<MockBtContext> dctx = new MockBtContext();
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);

  DefaultPieceStorage ps(dctx, option);

  ps.markPiecesDone(pieceLength*10+16*1024*2+1);

  for(int32_t i = 0; i < 10; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
  for(int32_t i = 10; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(!ps.hasPiece(i));
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)pieceLength*10+16*1024*2, ps.getCompletedLength());

  ps.markPiecesDone(totalLength);

  for(int32_t i = 0; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
}

} // namespace aria2
