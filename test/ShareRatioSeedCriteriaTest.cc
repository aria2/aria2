#include "ShareRatioSeedCriteria.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "FileEntry.h"

namespace aria2 {

class ShareRatioSeedCriteriaTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ShareRatioSeedCriteriaTest);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testEvaluate();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ShareRatioSeedCriteriaTest);

void ShareRatioSeedCriteriaTest::testEvaluate() {
  std::string infoHash = "01234567890123456789";
  std::string infoHashString =
    Util::toHex((const unsigned char*)infoHash.c_str(), infoHash.size());
  SharedHandle<MockBtContext> btContext(new MockBtContext());
  btContext->setTotalLength(1000000);
  btContext->setInfoHash((const unsigned char*)infoHash.c_str());
  
  SharedHandle<BtRuntime> btRuntime(new BtRuntime());
  btRuntime->setUploadLengthAtStartup(500000);
  
  SharedHandle<MockPeerStorage> peerStorage(new MockPeerStorage());
  TransferStat stat;
  stat.setSessionDownloadLength(1000000);
  stat.setSessionUploadLength(500000);
  peerStorage->setStat(stat);

  SharedHandle<MockPieceStorage> pieceStorage(new MockPieceStorage());
  pieceStorage->setCompletedLength(1000000);

  ShareRatioSeedCriteria cri(1.0, btContext);
  cri.setPeerStorage(peerStorage);
  cri.setPieceStorage(pieceStorage);
  cri.setBtRuntime(btRuntime);

  CPPUNIT_ASSERT(cri.evaluate());
  
  cri.setRatio(2.0);
  CPPUNIT_ASSERT(!cri.evaluate());
  // check div by zero
  btContext->setTotalLength(0);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
