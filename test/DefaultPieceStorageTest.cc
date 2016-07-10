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

class DefaultPieceStorageTest : public CppUnit::TestFixture {

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
  CPPUNIT_TEST(testAdvertisePiece);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<DownloadContext> dctx_;
  std::shared_ptr<Peer> peer;
  std::shared_ptr<Option> option_;
  std::unique_ptr<PieceSelector> pieceSelector_;

public:
  void setUp()
  {
    option_ = std::make_shared<Option>();
    option_->put(PREF_DIR, ".");
    dctx_ = std::make_shared<DownloadContext>();
    bittorrent::load(A2_TEST_DIR "/test.torrent", dctx_, option_);
    peer = std::make_shared<Peer>("192.168.0.1", 6889);
    peer->allocateSessionResource(dctx_->getPieceLength(),
                                  dctx_->getTotalLength());
    pieceSelector_ = make_unique<InorderPieceSelector>();
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
  void testAdvertisePiece();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPieceStorageTest);

void DefaultPieceStorageTest::testGetTotalLength()
{
  DefaultPieceStorage pss(dctx_, option_.get());

  CPPUNIT_ASSERT_EQUAL((int64_t)384LL, pss.getTotalLength());
}

void DefaultPieceStorageTest::testGetMissingPiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  peer->setAllBitfield();

  auto piece = pss.getMissingPiece(peer, 1);
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

void DefaultPieceStorageTest::testGetMissingPiece_many()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  peer->setAllBitfield();
  std::vector<std::shared_ptr<Piece>> pieces;
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
  pss.setPieceSelector(std::move(pieceSelector_));
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(1);

  auto piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       piece->toString());

  piece = pss.getMissingPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT(!piece);
}

void DefaultPieceStorageTest::testGetMissingPiece_manyWithExcludedIndexes()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  peer->setAllBitfield();
  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(1);
  std::vector<std::shared_ptr<Piece>> pieces;
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

void DefaultPieceStorageTest::testGetMissingFastPiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(2);

  auto piece = pss.getMissingFastPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=2, length=128"),
                       piece->toString());

  CPPUNIT_ASSERT(!pss.getMissingFastPiece(peer, 1));
}

void DefaultPieceStorageTest::testGetMissingFastPiece_excludedIndexes()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();
  peer->setFastExtensionEnabled(true);
  peer->addPeerAllowedIndex(1);
  peer->addPeerAllowedIndex(2);

  std::vector<size_t> excludedIndexes;
  excludedIndexes.push_back(2);

  auto piece = pss.getMissingFastPiece(peer, excludedIndexes, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=1, length=128"),
                       piece->toString());

  CPPUNIT_ASSERT(!pss.getMissingFastPiece(peer, excludedIndexes, 1));
}

void DefaultPieceStorageTest::testHasMissingPiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());

  CPPUNIT_ASSERT(!pss.hasMissingPiece(peer));

  peer->setAllBitfield();

  CPPUNIT_ASSERT(pss.hasMissingPiece(peer));
}

void DefaultPieceStorageTest::testCompletePiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  pss.setPieceSelector(std::move(pieceSelector_));
  pss.setEndGamePieceNum(0);

  peer->setAllBitfield();

  auto piece = pss.getMissingPiece(peer, 1);
  CPPUNIT_ASSERT_EQUAL(std::string("piece: index=0, length=128"),
                       piece->toString());

  CPPUNIT_ASSERT_EQUAL((int64_t)0LL, pss.getCompletedLength());

  pss.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)128LL, pss.getCompletedLength());

  auto incompletePiece = pss.getMissingPiece(peer, 1);
  incompletePiece->completeBlock(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)256LL, pss.getCompletedLength());
}

void DefaultPieceStorageTest::testGetPiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());

  auto pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int64_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(false, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testGetPieceInUsedPieces()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  auto piece = std::make_shared<Piece>(0, 128);
  piece->completeBlock(0);
  pss.addUsedPiece(piece);
  auto pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int64_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, pieceGot->countCompleteBlock());
}

void DefaultPieceStorageTest::testGetPieceCompletedPiece()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  auto piece = std::make_shared<Piece>(0, 128);
  pss.completePiece(piece);
  auto pieceGot = pss.getPiece(0);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pieceGot->getIndex());
  CPPUNIT_ASSERT_EQUAL((int64_t)128, pieceGot->getLength());
  CPPUNIT_ASSERT_EQUAL(true, pieceGot->pieceComplete());
}

void DefaultPieceStorageTest::testCancelPiece()
{
  size_t pieceLength = 256_k;
  int64_t totalLength = 32 * pieceLength; // <-- make the number of piece
                                          // greater than END_GAME_PIECE_NUM
  std::deque<std::string> uris1;
  uris1.push_back("http://localhost/src/file1.txt");
  auto file1 =
      std::make_shared<FileEntry>("src/file1.txt", totalLength, 0 /*, uris1*/);

  auto dctx = std::make_shared<DownloadContext>(pieceLength, totalLength,
                                                "src/file1.txt");

  DefaultPieceStorage ps{dctx, option_.get()};

  auto p = ps.getMissingPiece(0, 1);
  p->completeBlock(0);

  ps.cancelPiece(p, 1);

  auto p2 = ps.getMissingPiece(0, 2);

  CPPUNIT_ASSERT(p2->hasBlock(0));
  CPPUNIT_ASSERT(p2->usedBy(2));
  CPPUNIT_ASSERT(!p2->usedBy(1));
}

void DefaultPieceStorageTest::testMarkPiecesDone()
{
  size_t pieceLength = 256_k;
  int64_t totalLength = 4_m;
  auto dctx = std::make_shared<DownloadContext>(pieceLength, totalLength);

  DefaultPieceStorage ps(dctx, option_.get());

  ps.markPiecesDone(pieceLength * 10 + 32_k + 1);

  for (size_t i = 0; i < 10; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }
  for (size_t i = 10; i < (totalLength + pieceLength - 1) / pieceLength; ++i) {
    CPPUNIT_ASSERT(!ps.hasPiece(i));
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)pieceLength * 10 + (int64_t)32_k,
                       ps.getCompletedLength());

  ps.markPiecesDone(totalLength);

  for (size_t i = 0; i < (totalLength + pieceLength - 1) / pieceLength; ++i) {
    CPPUNIT_ASSERT(ps.hasPiece(i));
  }

  ps.markPiecesDone(0);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, ps.getCompletedLength());
}

void DefaultPieceStorageTest::testGetCompletedLength()
{
  auto dctx = std::make_shared<DownloadContext>(1_m, 256_m);

  DefaultPieceStorage ps(dctx, option_.get());

  CPPUNIT_ASSERT_EQUAL((int64_t)0, ps.getCompletedLength());

  ps.markPiecesDone(250_m);
  CPPUNIT_ASSERT_EQUAL((int64_t)250_m, ps.getCompletedLength());

  std::vector<std::shared_ptr<Piece>> inFlightPieces;
  for (int i = 0; i < 2; ++i) {
    auto p = std::make_shared<Piece>(250 + i, 1_m);
    for (int j = 0; j < 32; ++j) {
      p->completeBlock(j);
    }
    inFlightPieces.push_back(p);
    CPPUNIT_ASSERT_EQUAL((int64_t)512_k, p->getCompletedLength());
  }
  ps.addInFlightPiece(inFlightPieces);

  CPPUNIT_ASSERT_EQUAL((int64_t)251_m, ps.getCompletedLength());

  ps.markPiecesDone(256_m);

  CPPUNIT_ASSERT_EQUAL((int64_t)256_m, ps.getCompletedLength());
}

void DefaultPieceStorageTest::testGetFilteredCompletedLength()
{
  const size_t pieceLength = 1_m;
  auto dctx = std::make_shared<DownloadContext>();
  dctx->setPieceLength(pieceLength);
  auto files = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>("foo", 2 * pieceLength, 0),
      std::make_shared<FileEntry>("bar", 4 * pieceLength, 2 * pieceLength)};
  files[1]->setRequested(false);
  dctx->setFileEntries(std::begin(files), std::end(files));

  DefaultPieceStorage ps(dctx, option_.get());
  std::vector<std::shared_ptr<Piece>> inflightPieces(2);
  inflightPieces[0] = std::make_shared<Piece>(1, pieceLength);
  inflightPieces[0]->completeBlock(0);
  inflightPieces[1] = std::make_shared<Piece>(2, pieceLength);
  inflightPieces[1]->completeBlock(1);
  inflightPieces[1]->completeBlock(2);

  ps.addInFlightPiece(inflightPieces);
  ps.setupFileFilter();

  auto piece = ps.getMissingPiece(0, 1);
  ps.completePiece(piece);

  CPPUNIT_ASSERT_EQUAL((int64_t)pieceLength + (int64_t)16_k,
                       ps.getFilteredCompletedLength());
}

void DefaultPieceStorageTest::testGetNextUsedIndex()
{
  DefaultPieceStorage pss(dctx_, option_.get());
  CPPUNIT_ASSERT_EQUAL((size_t)3, pss.getNextUsedIndex(0));
  auto piece = pss.getMissingPiece(2, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
  pss.completePiece(piece);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
  piece = pss.getMissingPiece(0, 1);
  CPPUNIT_ASSERT_EQUAL((size_t)2, pss.getNextUsedIndex(0));
}

void DefaultPieceStorageTest::testAdvertisePiece()
{
  DefaultPieceStorage ps(dctx_, option_.get());

  ps.advertisePiece(1, 100, Timer(10_s));
  ps.advertisePiece(2, 101, Timer(11_s));
  ps.advertisePiece(3, 102, Timer(11_s));
  ps.advertisePiece(1, 103, Timer(12_s));
  ps.advertisePiece(2, 104, Timer(100_s));

  std::vector<size_t> res, ans;
  uint64_t lastHaveIndex;

  lastHaveIndex = ps.getAdvertisedPieceIndexes(res, 1, 0);
  ans = std::vector<size_t>{100, 101, 102, 103, 104};

  CPPUNIT_ASSERT_EQUAL((uint64_t)5, lastHaveIndex);
  CPPUNIT_ASSERT(ans == res);

  res.clear();
  lastHaveIndex = ps.getAdvertisedPieceIndexes(res, 1, 3);
  ans = std::vector<size_t>{103, 104};

  CPPUNIT_ASSERT_EQUAL((uint64_t)5, lastHaveIndex);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT(ans == res);

  res.clear();
  lastHaveIndex = ps.getAdvertisedPieceIndexes(res, 1, 5);

  CPPUNIT_ASSERT_EQUAL((uint64_t)5, lastHaveIndex);
  CPPUNIT_ASSERT_EQUAL((size_t)0, res.size());

  // remove haves

  ps.removeAdvertisedPiece(Timer(11_s));

  res.clear();
  lastHaveIndex = ps.getAdvertisedPieceIndexes(res, 1, 0);
  ans = std::vector<size_t>{103, 104};

  CPPUNIT_ASSERT_EQUAL((uint64_t)5, lastHaveIndex);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT(ans == res);

  ps.removeAdvertisedPiece(Timer(300_s));

  res.clear();
  lastHaveIndex = ps.getAdvertisedPieceIndexes(res, 1, 0);

  CPPUNIT_ASSERT_EQUAL((uint64_t)0, lastHaveIndex);
  CPPUNIT_ASSERT_EQUAL((size_t)0, res.size());
}

} // namespace aria2
