#include "DefaultPieceStorage.h"

#include <cppunit/extensions/HelperMacros.h>

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
#include "RarestPieceSelector.h"

namespace aria2 {

class DefaultPieceStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPieceStorageTest);
  CPPUNIT_TEST(testGetTotalLength);
  CPPUNIT_TEST(testGetMissingPiece);
  CPPUNIT_TEST(testGetMissingPiece_excludedIndexes);
  CPPUNIT_TEST(testGetMissingFastPiece);
  CPPUNIT_TEST(testGetMissingFastPiece_excludedIndexes);
  CPPUNIT_TEST(testHasMissingPiece);
  CPPUNIT_TEST(testCompletePiece);
  CPPUNIT_TEST(testGetPiece);
  CPPUNIT_TEST(testGetPieceInUsedPieces);
  CPPUNIT_TEST(testGetPieceCompletedPiece);
  CPPUNIT_TEST(testCancelPiece);
  CPPUNIT_TEST(testMarkPiecesDone);
  CPPUNIT_TEST(testGetCompletedLength);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<BtContext> btContext;
  SharedHandle<Peer> peer;
  Option* option;
  SharedHandle<RarestPieceSelector> _selector;
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
    _selector.reset(new RarestPieceSelector(btContext->getNumPieces(), false));
  }

  void tearDown()
  {
    delete option;
    option = 0;
  }

  void testGetTotalLength();
  void testGetMissingPiece();
  void testGetMissingPiece_excludedIndexes();
  void testGetMissingFastPiece();
  void testGetMissingFastPiece_excludedIndexes();
  void testHasMissingPiece();
  void testCompletePiece();
  void testGetPiece();
  void testGetPieceInUsedPieces();
  void testGetPieceCompletedPiece();
  void testCancelPiece();
  void testMarkPiecesDone();
  void testGetCompletedLength();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPieceStorageTest);

void DefaultPieceStorageTest::testGetTotalLength() {
  DefaultPieceStorage pss(btContext, option);

  CPPUNIT_ASSERT_EQUAL((uint64_t)384ULL, pss.getTotalLength());
}

void DefaultPieceStorageTest::testGetMissingPiece() {
  DefaultPieceStorage pss(btContext, option, _selector);
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

void DefaultPieceStorageTest::testGetMissingPiece_excludedIndexes()
{
  DefaultPieceStorage pss(btContext, option, _selector);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  std::deque<size_t> excludedIndexes;
  excludedIndexes.push_back(1);

  SharedHandle<Piece> piece = pss.getMissingPiece(peer, excludedIndexes);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
		       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
		       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes);
  CPPUNIT_ASSERT(piece.isNull());
}

void DefaultPieceStorageTest::testGetMissingFastPiece() {
  DefaultPieceStorage pss(btContext, option, _selector);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(2);

  SharedHandle<Piece> piece = pss.getMissingFastPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
		       piece->toString());

  CPPUNIT_ASSERT(pss.getMissingFastPiece(peer).isNull());
}

void DefaultPieceStorageTest::testGetMissingFastPiece_excludedIndexes()
{
  DefaultPieceStorage pss(btContext, option, _selector);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(1);
  peer->addPeerAllowedIndex(2);

  std::deque<size_t> excludedIndexes;
  excludedIndexes.push_back(2);

  SharedHandle<Piece> piece = pss.getMissingFastPiece(peer, excludedIndexes);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
		       piece->toString());
  
  CPPUNIT_ASSERT(pss.getMissingFastPiece(peer, excludedIndexes).isNull());
}

void DefaultPieceStorageTest::testHasMissingPiece() {
  DefaultPieceStorage pss(btContext, option);

  CPPUNIT_ASSERT(!pss.hasMissingPiece(peer));
  
  peer->setAllBitfield();

  CPPUNIT_ASSERT(pss.hasMissingPiece(peer));
}

void DefaultPieceStorageTest::testCompletePiece() {
  DefaultPieceStorage pss(btContext, option, _selector);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  SharedHandle<Piece> piece = pss.getMissingPiece(peer);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
		       piece->toString());

  CPPUNIT_ASSERT_EQUAL((uint64_t)0ULL, pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((uint64_t)128ULL, pss.getCompletedLength());

  SharedHandle<Piece> incompletePiece = pss.getMissingPiece(peer);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL((uint64_t)256ULL, pss.getCompletedLength());
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

void DefaultPieceStorageTest::testGetCompletedLength()
{
  SharedHandle<MockBtContext> dctx(new MockBtContext());
  dctx->setPieceLength(1024*1024);
  dctx->setTotalLength(256*1024*1024);
  
  DefaultPieceStorage ps(dctx, option);
  
  CPPUNIT_ASSERT_EQUAL((uint64_t)0, ps.getCompletedLength());

  ps.markPiecesDone(250*1024*1024);
  CPPUNIT_ASSERT_EQUAL((uint64_t)250*1024*1024, ps.getCompletedLength());

  std::deque<SharedHandle<Piece> > inFlightPieces;
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Piece> p(new Piece(250+i, 1024*1024));
    for(int j = 0; j < 32; ++j) {
      p->completeBlock(j);
    }
    inFlightPieces.push_back(p);
    CPPUNIT_ASSERT_EQUAL((size_t)512*1024, p->getCompletedLength());
  }
  ps.addInFlightPiece(inFlightPieces);
  
  CPPUNIT_ASSERT_EQUAL((uint64_t)251*1024*1024, ps.getCompletedLength());

  ps.markPiecesDone(256*1024*1024);

  CPPUNIT_ASSERT_EQUAL((uint64_t)256*1024*1024, ps.getCompletedLength());
}

} // namespace aria2
