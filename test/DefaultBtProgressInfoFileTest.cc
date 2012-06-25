#include "DefaultBtProgressInfoFile.h"

#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "util.h"
#include "Exception.h"
#include "MockPieceStorage.h"
#include "prefs.h"
#include "DownloadContext.h"
#include "Piece.h"
#include "FileEntry.h"
#include "array_fun.h"
#ifdef ENABLE_BITTORRENT
# include "MockPeerStorage.h"
# include "BtRuntime.h"
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

class DefaultBtProgressInfoFileTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtProgressInfoFileTest);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST(testLoad);
#ifndef WORDS_BIGENDIAN
  CPPUNIT_TEST(testLoad_compat);
#endif // !WORDS_BIGENDIAN
#endif // ENABLE_BITTORRENT
  CPPUNIT_TEST(testSave_nonBt);
  CPPUNIT_TEST(testLoad_nonBt);
#ifndef WORDS_BIGENDIAN
  CPPUNIT_TEST(testLoad_nonBt_compat);
#endif // !WORDS_BIGENDIAN
  CPPUNIT_TEST(testLoad_nonBt_pieceLengthShorter);
  CPPUNIT_TEST(testUpdateFilename);
  CPPUNIT_TEST_SUITE_END();
private:

#ifdef ENABLE_BITTORRENT
  SharedHandle<DownloadContext> dctx_;

  SharedHandle<MockPeerStorage> peerStorage_;

  SharedHandle<BtRuntime> btRuntime_;
#endif // ENABLE_BITTORRENT

  SharedHandle<MockPieceStorage> pieceStorage_;
  SharedHandle<Option> option_;
  SharedHandle<BitfieldMan> bitfield_;
public:   
  void initializeMembers(int32_t pieceLength, int64_t totalLength)
  {
    option_.reset(new Option());
    option_->put(PREF_DIR, A2_TEST_OUT_DIR);

    bitfield_.reset(new BitfieldMan(pieceLength, totalLength));

    pieceStorage_.reset(new MockPieceStorage());
    pieceStorage_->setBitfield(bitfield_.get());

#ifdef ENABLE_BITTORRENT
    static unsigned char infoHash[] = {
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
      0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
    };
  
    dctx_.reset(new DownloadContext());
    SharedHandle<TorrentAttribute> torrentAttrs(new TorrentAttribute());
    torrentAttrs->infoHash = std::string(vbegin(infoHash), vend(infoHash));
    dctx_->setAttribute(bittorrent::BITTORRENT, torrentAttrs);
    const SharedHandle<FileEntry> fileEntries[] = {
      SharedHandle<FileEntry>(new FileEntry("/path/to/file",totalLength,0))
    };
    dctx_->setFileEntries(vbegin(fileEntries), vend(fileEntries));
    dctx_->setPieceLength(pieceLength);
    peerStorage_.reset(new MockPeerStorage());
    btRuntime_.reset(new BtRuntime());
#endif // ENABLE_BITTORRENT
  }

#ifdef ENABLE_BITTORRENT
  void testSave();
  void testLoad();
#ifndef WORDS_BIGENDIAN
  void testLoad_compat();
#endif // !WORDS_BIGENDIAN
#endif // ENABLE_BITTORRENT
  void testSave_nonBt();
  void testLoad_nonBt();
#ifndef WORDS_BIGENDIAN
  void testLoad_nonBt_compat();
#endif // !WORDS_BIGENDIAN
  void testLoad_nonBt_pieceLengthShorter();
  void testUpdateFilename();
};

#undef BLOCK_LENGTH
#define BLOCK_LENGTH 256

CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtProgressInfoFileTest);

#ifdef ENABLE_BITTORRENT

// Because load.aria2 is made for little endian systems, exclude
// testLoad_compat() for big endian systems.
#ifndef WORDS_BIGENDIAN
void DefaultBtProgressInfoFileTest::testLoad_compat()
{
  initializeMembers(1024, 81920);
  dctx_->setBasePath(A2_TEST_DIR"/load");

  DefaultBtProgressInfoFile infoFile(dctx_, pieceStorage_, option_.get());
  infoFile.setBtRuntime(btRuntime_);
  infoFile.setPeerStorage(peerStorage_);

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/load.aria2"),
                       infoFile.getFilename());

  infoFile.load();

  // check the contents of objects

  // total length
  CPPUNIT_ASSERT_EQUAL((int64_t)81920, dctx_->getTotalLength());

  // upload length
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, btRuntime_->getUploadLengthAtStartup());

  // bitfield
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfield_->getBitfield(),
                                   bitfield_->getBitfieldLength()));

  // the number of in-flight pieces
  CPPUNIT_ASSERT_EQUAL((size_t)2,
                       pieceStorage_->countInFlightPiece());

  // piece index 1
  std::vector<SharedHandle<Piece> > inFlightPieces;
  pieceStorage_->getInFlightPieces(inFlightPieces);

  SharedHandle<Piece> piece1 = inFlightPieces[0];
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getIndex());
  CPPUNIT_ASSERT_EQUAL(1024, piece1->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getBitfieldLength());
  CPPUNIT_ASSERT_EQUAL(std::string("00"), util::toHex(piece1->getBitfield(),
                                                      piece1->getBitfieldLength()));

  // piece index 2
  SharedHandle<Piece> piece2 = inFlightPieces[1];
  CPPUNIT_ASSERT_EQUAL((size_t)2, piece2->getIndex());
  CPPUNIT_ASSERT_EQUAL(512, piece2->getLength());
}
#endif // !WORDS_BIGENDIAN

void DefaultBtProgressInfoFileTest::testLoad()
{
  initializeMembers(1024, 81920);

  dctx_->setBasePath(A2_TEST_DIR"/load-v0001");

  DefaultBtProgressInfoFile infoFile(dctx_, pieceStorage_, option_.get());
  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/load-v0001.aria2"),
                       infoFile.getFilename());
  infoFile.setBtRuntime(btRuntime_);
  infoFile.setPeerStorage(peerStorage_);

  infoFile.load();

  // check the contents of objects

  // total length
  CPPUNIT_ASSERT_EQUAL((int64_t)81920, dctx_->getTotalLength());

  // upload length
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, btRuntime_->getUploadLengthAtStartup());

  // bitfield
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfield_->getBitfield(),
                                   bitfield_->getBitfieldLength()));

  // the number of in-flight pieces
  CPPUNIT_ASSERT_EQUAL((size_t)2,
                       pieceStorage_->countInFlightPiece());

  // piece index 1
  std::vector<SharedHandle<Piece> > inFlightPieces;
  pieceStorage_->getInFlightPieces(inFlightPieces);

  SharedHandle<Piece> piece1 = inFlightPieces[0];
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getIndex());
  CPPUNIT_ASSERT_EQUAL(1024, piece1->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getBitfieldLength());
  CPPUNIT_ASSERT_EQUAL(std::string("00"), util::toHex(piece1->getBitfield(),
                                                      piece1->getBitfieldLength()));

  // piece index 2
  SharedHandle<Piece> piece2 = inFlightPieces[1];
  CPPUNIT_ASSERT_EQUAL((size_t)2, piece2->getIndex());
  CPPUNIT_ASSERT_EQUAL(512, piece2->getLength());
}

void DefaultBtProgressInfoFileTest::testSave()
{
  initializeMembers(1024, 81920);

  dctx_->setBasePath(A2_TEST_OUT_DIR"/save-temp");
  bitfield_->setAllBit();
  bitfield_->unsetBit(79);
  pieceStorage_->setCompletedLength(80896);
  TransferStat stat;
  stat.setAllTimeUploadLength(1024);
  peerStorage_->setStat(stat);

  SharedHandle<Piece> p1(new Piece(1, 1024));
  SharedHandle<Piece> p2(new Piece(2, 512));
  std::vector<SharedHandle<Piece> > inFlightPieces;
  inFlightPieces.push_back(p1);
  inFlightPieces.push_back(p2);
  pieceStorage_->addInFlightPiece(inFlightPieces);
  
  DefaultBtProgressInfoFile infoFile(dctx_, pieceStorage_, option_.get());
  infoFile.setBtRuntime(btRuntime_);
  infoFile.setPeerStorage(peerStorage_);

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_OUT_DIR"/save-temp.aria2"),
                       infoFile.getFilename());

  infoFile.save();

  // read and validate
  std::ifstream in(infoFile.getFilename().c_str(), std::ios::binary);

  //in.exceptions(ios::failbit);

  unsigned char version[2];
  in.read((char*)version, sizeof(version));
  CPPUNIT_ASSERT_EQUAL(std::string("0001"),
                       util::toHex(version, sizeof(version)));

  unsigned char extension[4];
  in.read((char*)extension, sizeof(extension));
  CPPUNIT_ASSERT_EQUAL(std::string("00000001"),
                       util::toHex(extension, sizeof(extension)));

  uint32_t infoHashLength;
  in.read(reinterpret_cast<char*>(&infoHashLength), sizeof(infoHashLength));
  infoHashLength = ntohl(infoHashLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)20, infoHashLength);

  unsigned char infoHashRead[20];
  in.read((char*)infoHashRead, sizeof(infoHashRead));
  CPPUNIT_ASSERT_EQUAL(std::string("112233445566778899aabbccddeeff00ffffffff"),
                       util::toHex(infoHashRead, sizeof(infoHashRead)));

  uint32_t pieceLength;
  in.read((char*)&pieceLength, sizeof(pieceLength));
  pieceLength = ntohl(pieceLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pieceLength);

  uint64_t totalLength;
  in.read((char*)&totalLength, sizeof(totalLength));
  totalLength = ntoh64(totalLength);
  CPPUNIT_ASSERT_EQUAL((uint64_t)81920/* 80*1024 */, totalLength);

  uint64_t uploadLength;
  in.read((char*)&uploadLength, sizeof(uploadLength));
  uploadLength = ntoh64(uploadLength);
  CPPUNIT_ASSERT_EQUAL((uint64_t)1024, uploadLength);

  uint32_t bitfieldLength;
  in.read((char*)&bitfieldLength, sizeof(bitfieldLength));
  bitfieldLength = ntohl(bitfieldLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)10, bitfieldLength);

  unsigned char bitfieldRead[10];
  in.read((char*)bitfieldRead, sizeof(bitfieldRead));
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfieldRead, sizeof(bitfieldRead)));

  uint32_t numInFlightPiece;
  in.read((char*)&numInFlightPiece, sizeof(numInFlightPiece));
  numInFlightPiece = ntohl(numInFlightPiece);
  CPPUNIT_ASSERT_EQUAL((uint32_t)2, numInFlightPiece);

  // piece index 1
  uint32_t index1;
  in.read((char*)&index1, sizeof(index1));
  index1 = ntohl(index1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1, index1);

  uint32_t pieceLength1;
  in.read((char*)&pieceLength1, sizeof(pieceLength1));
  pieceLength1 = ntohl(pieceLength1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pieceLength1);

  uint32_t pieceBitfieldLength1;
  in.read((char*)&pieceBitfieldLength1, sizeof(pieceBitfieldLength1));
  pieceBitfieldLength1 = ntohl(pieceBitfieldLength1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1, pieceBitfieldLength1);

  unsigned char pieceBitfield1[1];
  in.read((char*)pieceBitfield1, sizeof(pieceBitfield1));
  CPPUNIT_ASSERT_EQUAL(std::string("00"),
                       util::toHex(pieceBitfield1, sizeof(pieceBitfield1)));

  // piece index 2
  uint32_t index2;
  in.read((char*)&index2, sizeof(index2));
  index2 = ntohl(index2);
  CPPUNIT_ASSERT_EQUAL((uint32_t)2, index2);

  uint32_t pieceLength2;
  in.read((char*)&pieceLength2, sizeof(pieceLength2));
  pieceLength2 = ntohl(pieceLength2);
  CPPUNIT_ASSERT_EQUAL((uint32_t)512, pieceLength2);
}

#endif // ENABLE_BITTORRENT

// Because load-nonBt.aria2 is made for little endian systems, exclude
// testLoad_nonBt_compat() for big endian systems.
#ifndef WORDS_BIGENDIAN
void DefaultBtProgressInfoFileTest::testLoad_nonBt_compat()
{
  initializeMembers(1024, 81920);

  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024, 81920, A2_TEST_DIR"/load-nonBt"));
  
  DefaultBtProgressInfoFile infoFile(dctx, pieceStorage_, option_.get());

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/load-nonBt.aria2"),
                       infoFile.getFilename());
  infoFile.load();

  // check the contents of objects

  // total length
  CPPUNIT_ASSERT_EQUAL((int64_t)81920, dctx->getTotalLength());

  // bitfield
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfield_->getBitfield(),
                                   bitfield_->getBitfieldLength()));

  // the number of in-flight pieces
  CPPUNIT_ASSERT_EQUAL((size_t)2,
                       pieceStorage_->countInFlightPiece());

  // piece index 1
  std::vector<SharedHandle<Piece> > inFlightPieces;
  pieceStorage_->getInFlightPieces(inFlightPieces);

  SharedHandle<Piece> piece1 = inFlightPieces[0];
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getIndex());
  CPPUNIT_ASSERT_EQUAL(1024, piece1->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getBitfieldLength());
  CPPUNIT_ASSERT_EQUAL(std::string("00"), util::toHex(piece1->getBitfield(),
                                                      piece1->getBitfieldLength()));

  // piece index 2
  SharedHandle<Piece> piece2 = inFlightPieces[1];
  CPPUNIT_ASSERT_EQUAL((size_t)2, piece2->getIndex());
  CPPUNIT_ASSERT_EQUAL(512, piece2->getLength());
}
#endif // !WORDS_BIGENDIAN

void DefaultBtProgressInfoFileTest::testLoad_nonBt()
{
  initializeMembers(1024, 81920);

  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024, 81920, A2_TEST_DIR"/load-nonBt-v0001"));
  
  DefaultBtProgressInfoFile infoFile(dctx, pieceStorage_, option_.get());

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/load-nonBt-v0001.aria2"),
                       infoFile.getFilename());
  infoFile.load();

  // check the contents of objects

  // total length
  CPPUNIT_ASSERT_EQUAL((int64_t)81920, dctx->getTotalLength());

  // bitfield
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfield_->getBitfield(),
                                   bitfield_->getBitfieldLength()));

  // the number of in-flight pieces
  CPPUNIT_ASSERT_EQUAL((size_t)2,
                       pieceStorage_->countInFlightPiece());

  // piece index 1
  std::vector<SharedHandle<Piece> > inFlightPieces;
  pieceStorage_->getInFlightPieces(inFlightPieces);

  SharedHandle<Piece> piece1 = inFlightPieces[0];
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getIndex());
  CPPUNIT_ASSERT_EQUAL(1024, piece1->getLength());
  CPPUNIT_ASSERT_EQUAL((size_t)1, piece1->getBitfieldLength());
  CPPUNIT_ASSERT_EQUAL(std::string("00"), util::toHex(piece1->getBitfield(),
                                                      piece1->getBitfieldLength()));

  // piece index 2
  SharedHandle<Piece> piece2 = inFlightPieces[1];
  CPPUNIT_ASSERT_EQUAL((size_t)2, piece2->getIndex());
  CPPUNIT_ASSERT_EQUAL(512, piece2->getLength());
}

void DefaultBtProgressInfoFileTest::testLoad_nonBt_pieceLengthShorter()
{
  initializeMembers(512, 81920);
  option_->put(PREF_ALLOW_PIECE_LENGTH_CHANGE, A2_V_TRUE);

  SharedHandle<DownloadContext> dctx
    (new DownloadContext(512, 81920, A2_TEST_DIR"/load-nonBt-v0001"));

  DefaultBtProgressInfoFile infoFile(dctx, pieceStorage_, option_.get());

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/load-nonBt-v0001.aria2"),
                       infoFile.getFilename());
  infoFile.load();

  // check the contents of objects

  // bitfield
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffffffffffffffffffffffc"),
                       util::toHex(bitfield_->getBitfield(),
                                   bitfield_->getBitfieldLength()));

  // the number of in-flight pieces
  CPPUNIT_ASSERT_EQUAL((size_t)0,
                       pieceStorage_->countInFlightPiece());
}

void DefaultBtProgressInfoFileTest::testSave_nonBt()
{
  initializeMembers(1024, 81920);

  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024, 81920, A2_TEST_OUT_DIR"/save-temp"));

  bitfield_->setAllBit();
  bitfield_->unsetBit(79);
  pieceStorage_->setCompletedLength(80896);

  SharedHandle<Piece> p1(new Piece(1, 1024));
  SharedHandle<Piece> p2(new Piece(2, 512));
  std::vector<SharedHandle<Piece> > inFlightPieces;
  inFlightPieces.push_back(p1);
  inFlightPieces.push_back(p2);
  pieceStorage_->addInFlightPiece(inFlightPieces);
  
  DefaultBtProgressInfoFile infoFile(dctx, pieceStorage_, option_.get());

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_OUT_DIR"/save-temp.aria2"),
                       infoFile.getFilename());

  infoFile.save();
  
  // read and validate
  std::ifstream in(infoFile.getFilename().c_str(), std::ios::binary);

  //in.exceptions(ios::failbit);

  unsigned char version[2];
  in.read((char*)version, sizeof(version));
  CPPUNIT_ASSERT_EQUAL(std::string("0001"),
                       util::toHex(version, sizeof(version)));

  unsigned char extension[4];
  in.read((char*)extension, sizeof(extension));
  CPPUNIT_ASSERT_EQUAL(std::string("00000000"),
                       util::toHex(extension, sizeof(extension)));

  uint32_t infoHashLength;
  in.read(reinterpret_cast<char*>(&infoHashLength), sizeof(infoHashLength));
  infoHashLength = ntohl(infoHashLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)0, infoHashLength);

  uint32_t pieceLength;
  in.read((char*)&pieceLength, sizeof(pieceLength));
  pieceLength = ntohl(pieceLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pieceLength);
  
  uint64_t totalLength;
  in.read((char*)&totalLength, sizeof(totalLength));
  totalLength = ntoh64(totalLength);
  CPPUNIT_ASSERT_EQUAL((uint64_t)81920/* 80*1024 */, totalLength);

  uint64_t uploadLength;
  in.read((char*)&uploadLength, sizeof(uploadLength));
  uploadLength = ntoh64(uploadLength);
  CPPUNIT_ASSERT_EQUAL((uint64_t)0, uploadLength);

  uint32_t bitfieldLength;
  in.read((char*)&bitfieldLength, sizeof(bitfieldLength));
  bitfieldLength = ntohl(bitfieldLength);
  CPPUNIT_ASSERT_EQUAL((uint32_t)10, bitfieldLength);

  unsigned char bitfieldRead[10];
  in.read((char*)bitfieldRead, sizeof(bitfieldRead));
  CPPUNIT_ASSERT_EQUAL(std::string("fffffffffffffffffffe"),
                       util::toHex(bitfieldRead, sizeof(bitfieldRead)));

  uint32_t numInFlightPiece;
  in.read((char*)&numInFlightPiece, sizeof(numInFlightPiece));
  numInFlightPiece = ntohl(numInFlightPiece);
  CPPUNIT_ASSERT_EQUAL((uint32_t)2, numInFlightPiece);

  // piece index 1
  uint32_t index1;
  in.read((char*)&index1, sizeof(index1));
  index1 = ntohl(index1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1, index1);

  uint32_t pieceLength1;
  in.read((char*)&pieceLength1, sizeof(pieceLength1));
  pieceLength1 = ntohl(pieceLength1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1024, pieceLength1);

  uint32_t pieceBitfieldLength1;
  in.read((char*)&pieceBitfieldLength1, sizeof(pieceBitfieldLength1));
  pieceBitfieldLength1 = ntohl(pieceBitfieldLength1);
  CPPUNIT_ASSERT_EQUAL((uint32_t)1, pieceBitfieldLength1);

  unsigned char pieceBitfield1[1];
  in.read((char*)pieceBitfield1, sizeof(pieceBitfield1));
  CPPUNIT_ASSERT_EQUAL(std::string("00"),
                       util::toHex(pieceBitfield1, sizeof(pieceBitfield1)));

  // piece index 2
  uint32_t index2;
  in.read((char*)&index2, sizeof(index2));
  index2 = ntohl(index2);
  CPPUNIT_ASSERT_EQUAL((uint32_t)2, index2);

  uint32_t pieceLength2;
  in.read((char*)&pieceLength2, sizeof(pieceLength2));
  pieceLength2 = ntohl(pieceLength2);
  CPPUNIT_ASSERT_EQUAL((uint32_t)512, pieceLength2);

}

void DefaultBtProgressInfoFileTest::testUpdateFilename()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024, 81920, A2_TEST_DIR"/file1"));

  DefaultBtProgressInfoFile infoFile(dctx, SharedHandle<MockPieceStorage>(), 0);
#ifdef ENABLE_BITTORRENT
  infoFile.setBtRuntime(btRuntime_);
  infoFile.setPeerStorage(peerStorage_);
#endif // ENABLE_BITTORRENT

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/file1.aria2"),
                       infoFile.getFilename());

  dctx->getFirstFileEntry()->setPath(A2_TEST_DIR"/file1.1");

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/file1.aria2"),
                       infoFile.getFilename());

  infoFile.updateFilename();

  CPPUNIT_ASSERT_EQUAL(std::string(A2_TEST_DIR"/file1.1.aria2"),
                       infoFile.getFilename());
}

} // namespace aria2
