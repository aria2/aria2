#include "DefaultBtProgressInfoFile.h"
#include "DefaultBtContext.h"
#include "Option.h"
#include "Util.h"
#include "Exception.h"
#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "prefs.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultBtProgressInfoFileTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtProgressInfoFileTest);
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST_SUITE_END();
private:
  BtContextHandle btContext;
  Option* option;
public:
  DefaultBtProgressInfoFileTest():btContext(0) {}

  void setUp() {
    btContext = BtContextHandle(new DefaultBtContext());
    btContext->load("test.torrent");
    option = new Option();
  }

  void testSave();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtProgressInfoFileTest);

void DefaultBtProgressInfoFileTest::testSave() {
  unsigned char infoHash[] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
    0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
  };

  Option option;
  option.put(PREF_DIR, ".");

  MockBtContextHandle btContext = new MockBtContext();
  btContext->setInfoHash(infoHash);
  btContext->setName("save-temp");

  BitfieldMan bitfield(1024, 80*1024);
  bitfield.setAllBit();
  bitfield.unsetBit(79);
  MockPieceStorageHandle pieceStorage = new MockPieceStorage();
  pieceStorage->setBitfield(&bitfield);
  pieceStorage->setCompletedLength(80896);

  MockPeerStorageHandle peerStorage = new MockPeerStorage();
  TransferStat stat;
  stat.sessionUploadLength = 1024;
  peerStorage->setStat(stat);

  BtRuntimeHandle btRuntime = new BtRuntime();

  DefaultBtProgressInfoFile infoFile(btContext, &option);
  infoFile.setPieceStorage(pieceStorage);
  infoFile.setPeerStorage(peerStorage);
  infoFile.setBtRuntime(btRuntime);

  infoFile.save();

  // read and validate
  ifstream in(string(option.get(PREF_DIR)+"/"+btContext->getName()+".aria2").c_str());
  unsigned char infoHashRead[20];
  in.read((char*)infoHashRead, sizeof(infoHashRead));
  CPPUNIT_ASSERT_EQUAL(string("112233445566778899aabbccddeeff00ffffffff"),
		       Util::toHex(infoHashRead, sizeof(infoHashRead)));
  unsigned char bitfieldRead[10];
  in.read((char*)bitfieldRead, sizeof(bitfieldRead));
  CPPUNIT_ASSERT_EQUAL(string("fffffffffffffffffffe"),
		       Util::toHex(bitfieldRead, sizeof(bitfieldRead)));
  int64_t allTimeDownloadLengthRead = 0;
  in.read((char*)&allTimeDownloadLengthRead, sizeof(allTimeDownloadLengthRead));
  CPPUNIT_ASSERT_EQUAL((int64_t)80896, allTimeDownloadLengthRead);

  int64_t allTimeUploadLengthRead = 0;
  in.read((char*)&allTimeUploadLengthRead, sizeof(allTimeUploadLengthRead));
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, allTimeUploadLengthRead);
  
  string temp;
  getline(in, temp);
  CPPUNIT_ASSERT_EQUAL(string(""), temp);
  CPPUNIT_ASSERT(in.eof());
}
