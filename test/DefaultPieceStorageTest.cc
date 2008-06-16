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
  CPPUNIT_TEST(testCancelPiece);
  CPPUNIT_TEST(testMarkPiecesDone);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<BtContext> btContext;
  SharedHandle<Peer> peer;
  Option* option;
public:
  DefaultPieceStorageTest() {
    SharedHandle<FixedNumberRandomizer> randomizer
      (new FixedNumberRandomizer());
    randomizer->setFixedNumber(0);
    BitfieldManFactory::setDefaultRandomizer(randomizer);
  }

  void setUp() {
    btContext.reset(new DefaultBtContext());
    btContext->load("test.torrent");
    peer.reset(new Peer("192.168.0.1", 6889));
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
  void testCancelPiece();
  void testMarkPiecesDone();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPieceStorageTest);

void DefaultPieceStorageTest::testGetTotalLength() {
  DefaultPieceStorage pss(btContext, option);

  CPPUNIT_ASSERT_EQUAL(384ULL, pss.getTotalLength());
}

void DefaultPieceStorageTest::testGetMissingPiece() {
  DefaultPieceStorage pss(btContext, option, false);
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
  DefaultPieceStorage pss(btContext, option, true);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  SharedHandle<Piece> piece = pss.getMissingPiece(peer);
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
		       piece->toString());
#else // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
		       piece->toString());
#endif // !__MINGW32__

  CPPUNIT_ASSERT_EQUAL(0ULL, pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL(128ULL, pss.getCompletedLength());

  SharedHandle<Piece> incompletePiece = pss.getMissingPiece(peer);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL(256ULL, pss.getCompletedLength());
}

void DefaultPieceStorageTest::testGetPiece() {
  DefaultPieceStorage pss(btContext, option);
  
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces() {
  DefaultPieceStorage pss(btContext, option);
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece() {
  DefaultPieceStorage pss(btContext, option);
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  pss.completePiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(true, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testCancelPiece()
{
  size_t pieceLength = 256*1024;
  uint64_t totalLength = 32*pieceLength; // <-- make the number of piece greater than END_GAME_PIECE_NUM
  std::deque<std::string> uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  SharedHandle<FileEntry> file1(new FileEntry("src/file1.txt", totalLength, 0 /*, uris1*/));

  SharedHandle<MockBtContext> dctx(new MockBtContext());
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);
  dctx->addFileEntry(file1);

  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, option));

  SharedHandle<Piece> p = ps->getMissingPiece();
  p->completeBlock(0);
  
  ps->cancelPiece(p);

  SharedHandle<Piece> p2 = ps->getMissingPiece();

  CPPUNIT_ASSERT(p2->hasBlock(0));
}

void DefaultPieceStorageTest::testMarkPiecesDone()
{
  size_t pieceLength = 256*1024;
  uint64_t totalLength = 4*1024*1024;
  SharedHandle<MockBtContext> dctx(new MockBtContext());
  dctx->setPieceLength(pieceLength);
  dctx->setTotalLength(totalLength);

  DefaultPieceStorage ps(dctx, option);

  ps.markPiecesDone(pieceLength*10+16*1024*2+1);

  for(size_t i = 0; i < 10; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
  for(size_t i = 10; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(!ps.hasPiece(i));
  }
  CPPUNIT_ASSERT_EQUAL((uint64_t)pieceLength*10+16*1024*2, ps.getCompletedLength());

  ps.markPiecesDone(totalLength);

  for(size_t i = 0; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
}

} // namespace aria2
