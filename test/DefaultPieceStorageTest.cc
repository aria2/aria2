#include "DefaultPieceStorage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "Exception.h"
#include "Piece.h"
#include "Peer.h"
#include "Option.h"
#include "FileEntry.h"
#include "RarestPieceSelector.h"
#include "InorderPieceSelector.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "DiskAdaptor.h"
#include "DiskWriterFactory.h"
#include "PieceStatMan.h"
#include "prefs.h"

namespace aria2 {

class DefaultPieceStorageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultPieceStorageTest);
  CPPUNIT_TEST(testGetTotalLength);
  CPPUNIT_TEST(testGetMissingPiece);
  CPPUNIT_TEST(testGetMissingPiece_many);
  CPPUNIT_TEST(testGetMissingPiece_excludedIndexes);
  CPPUNIT_TEST(testGetMissingPiece_manyWithExcludedIndexes);
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
  CPPUNIT_TEST(testGetFilteredCompletedLength);
  CPPUNIT_TEST(testGetNextUsedIndex);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<Peer> peer;
  SharedHandle<Option> option_;
  SharedHandle<PieceSelector> pieceSelector_;
public:
  void setUp() {
    option_.reset(new Option());
    option_->put(PREF_DIR, ".");
    dctx_.reset(new DownloadContext());
    bittorrent::load(A2_TEST_DIR"/test.torrent", dctx_, option_);
    peer.reset(new Peer("192.168.0.1", 6889));
    peer->allocateSessionResource(dctx_->getPieceLength(),
                                  dctx_->getTotalLength());
    pieceSelector_.reset(new InorderPieceSelector());
  }

  void testGetTotalLength();
  void testGetMissingPiece();
  void testGetMissingPiece_many();
  void testGetMissingPiece_excludedIndexes();
  void testGetMissingPiece_manyWithExcludedIndexes();
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
  void testGetFilteredCompletedLength();
  void testGetNextUsedIndex();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPieceStorageTest);

void DefaultPieceStorageTest::testGetTotalLength() {
  DefaultPieceStorage pss(dctx_, option_.get());

  CPPUNIT_ASSERT_EQUAL((int64_t)384LL, pss.getTotalLength());
}

void DefaultPieceStorageTest::testGetMissingPiece() {
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  peer->setAllBitfield();

  SharedHandle<Piece> piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       piece->toString());
  CPPUNIT_ASSERT(piece->usedBy(1));
  piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
                       piece->toString());
  piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       piece->toString());
  piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT(!piece);
}

void DefaultPieceStorageTest::testGetMissingPiece_many() {
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  peer->setAllBitfield();
  std::vector<SharedHandle<Piece> > pieces;
  pss.getMissingPiece(pieces, 2, peer, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pieces.size());
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       pieces[0]->toString());
  CPPUNIT_ASSERT(pieces[0]->usedBy(1));
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
                       pieces[1]->toString());
  pieces.clear();
  pss.getMissingPiece(pieces, 2, peer, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieces.size());
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       pieces[0]->toString());
}

void DefaultPieceStorageTest::testGetMissingPiece_excludedIndexes()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(1);

  SharedHandle<Piece> piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT(!piece);
}

void DefaultPieceStorageTest::testGetMissingPiece_manyWithExcludedIndexes() {
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  peer->setAllBitfield();
  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(1);
  std::vector<SharedHandle<Piece> > pieces;
  pss.getMissingPiece(pieces, 2, peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pieces.size());
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       pieces[0]->toString());
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       pieces[1]->toString());
  pieces.clear();
  pss.getMissingPiece(pieces, 2, peer, excludedIndexes, 1);
  CPPUNIT_ASSERT(pieces.empty());
}

void DefaultPieceStorageTest::testGetMissingFastPiece() {
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(2);

  SharedHandle<Piece> piece = pss.getMissingFastPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       piece->toString());

  CPPUNIT_ASSERT(!pss.getMissingFastPiece(peer, 1));
}

void DefaultPieceStorageTest::testGetMissingFastPiece_excludedIndexes()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(1);
  peer->addPeerAllowedIndex(2);

  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(2);

  SharedHandle<Piece> piece = pss.getMissingFastPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
                       piece->toString());
  
  CPPUNIT_ASSERT(!pss.getMissingFastPiece(peer, excludedIndexes, 1));
}

void DefaultPieceStorageTest::testHasMissingPiece() {
  DefaultPieceStorage pss(dctx_, option_.get());

  CPPUNIT_ASSERT(!pss.hasMissingPiece(peer));
  
  peer->setAllBitfield();

  CPPUNIT_ASSERT(pss.hasMissingPiece(peer));
}

void DefaultPieceStorageTest::testCompletePiece() {
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(pieceSelector_);
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  SharedHandle<Piece> piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       piece->toString());

  CPPUNIT_ASSERT_EQUAL((int64_t)0LL, pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)128LL, pss.getCompletedLength());

  SharedHandle<Piece> incompletePiece = pss.getMissingPiece(peer, 1);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)256LL, pss.getCompletedLength());
}

void DefaultPieceStorageTest::testGetPiece() {
  DefaultPieceStorage pss(dctx_, option_.get());
  
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces() {
  DefaultPieceStorage pss(dctx_, option_.get());
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece() {
  DefaultPieceStorage pss(dctx_, option_.get());
  SharedHandle<Piece> piece = SharedHandle<Piece>(new Piece(0, 128));
  pss.completePiece(piece);
  SharedHandle<Piece> pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL(128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(true, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testCancelPiece()
{
  size_t pieceLength = 256*1024;
  int64_t totalLength = 32*pieceLength; // <-- make the number of piece greater than END_GAME_PIECE_NUM
  std::deque<std::string> uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  SharedHandle<FileEntry> file1(new FileEntry("src/file1.txt", totalLength, 0 /*, uris1*/));

  SharedHandle<DownloadContext> dctx
    (new DownloadContext(pieceLength, totalLength, "src/file1.txt"));

  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, option_.get()));

  SharedHandle<Piece> p = ps->getMissingPiece(0, 1);
  p->completeBlock(0);
  
  ps->cancelPiece(p, 1);

  SharedHandle<Piece> p2 = ps->getMissingPiece(0, 2);

  CPPUNIT_ASSERT(p2->hasBlock(0));
  CPPUNIT_ASSERT(p2->usedBy(2));
  CPPUNIT_ASSERT(!p2->usedBy(1));
}

void DefaultPieceStorageTest::testMarkPiecesDone()
{
  size_t pieceLength = 256*1024;
  int64_t totalLength = 4*1024*1024;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(pieceLength, totalLength));

  DefaultPieceStorage ps(dctx, option_.get());

  ps.markPiecesDone(pieceLength*10+16*1024*2+1);

  for(size_t i = 0; i < 10; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
  for(size_t i = 10; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(!ps.hasPiece(i));
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)pieceLength*10+16*1024*2, ps.getCompletedLength());

  ps.markPiecesDone(totalLength);

  for(size_t i = 0; i < (totalLength+pieceLength-1)/pieceLength; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }

  ps.markPiecesDone(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, ps.getCompletedLength());
}

void DefaultPieceStorageTest::testGetCompletedLength()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024*1024, 256*1024*1024));
  
  DefaultPieceStorage ps(dctx, option_.get());
  
  CPPUNIT_ASSERT_EQUAL((int64_t)0, ps.getCompletedLength());

  ps.markPiecesDone(250*1024*1024);
  CPPUNIT_ASSERT_EQUAL((int64_t)250*1024*1024, ps.getCompletedLength());

  std::vector<SharedHandle<Piece> > inFlightPieces;
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Piece> p(new Piece(250+i, 1024*1024));
    for(int j = 0; j < 32; ++j) {
      p->completeBlock(j);
    }
    inFlightPieces.push_back(p);
    CPPUNIT_ASSERT_EQUAL(512*1024, p->getCompletedLength());
  }
  ps.addInFlightPiece(inFlightPieces);
  
  CPPUNIT_ASSERT_EQUAL((int64_t)251*1024*1024, ps.getCompletedLength());

  ps.markPiecesDone(256*1024*1024);

  CPPUNIT_ASSERT_EQUAL((int64_t)256*1024*1024, ps.getCompletedLength());
}

void DefaultPieceStorageTest::testGetFilteredCompletedLength()
{
  const size_t pieceLength = 1024*1024;
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setPieceLength(pieceLength);
  SharedHandle<FileEntry> files[] = {
    SharedHandle<FileEntry>(new FileEntry("foo", 2*pieceLength, 0)),
    SharedHandle<FileEntry>(new FileEntry("bar", 4*pieceLength, 2*pieceLength))
  };
  files[1]->setRequested(false);
  dctx->setFileEntries(&files[0], &files[2]);

  DefaultPieceStorage ps(dctx, option_.get());
  std::vector<SharedHandle<Piece> > inflightPieces(2);
  inflightPieces[0] = SharedHandle<Piece>(new Piece(1, pieceLength));
  inflightPieces[0]->completeBlock(0);
  inflightPieces[1] = SharedHandle<Piece>(new Piece(2, pieceLength));
  inflightPieces[1]->completeBlock(1);
  inflightPieces[1]->completeBlock(2);

  ps.addInFlightPiece(inflightPieces);
  ps.setupFileFilter();

  SharedHandle<Piece> piece = ps.getMissingPiece(0, 1);
  ps.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)pieceLength+16*1024,
                       ps.getFilteredCompletedLength());
}

void DefaultPieceStorageTest::testGetNextUsedIndex()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  CPPUNIT_ASSERT_EQUAL((size_t)3, pss.getNextUsedIndex(0));
  SharedHandle<Piece> piece = pss.getMissingPiece(2, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
  pss.completePiece(piece);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
  piece = pss.getMissingPiece(0, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
}

} // namespace aria2
