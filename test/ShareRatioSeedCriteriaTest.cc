#include "ShareRatioSeedCriteria.h"
#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

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
  std::string infoHashString = Util::toHex((const unsigned char*)infoHash.c_str(), infoHash.size());
  MockBtContext* mockBtContext = new MockBtContext();
  mockBtContext->setTotalLength(1000000);
  mockBtContext->setInfoHash((const unsigned char*)infoHash.c_str());
  SharedHandle<BtContext> btContext(mockBtContext);
  
  SharedHandle<BtRuntime> btRuntime(new BtRuntime());
  btRuntime->setUploadLengthAtStartup(500000);
  BtRegistry::registerBtRuntime(infoHashString, btRuntime);
  
  MockPeerStorage* mockPeerStorage = new MockPeerStorage();
  TransferStat stat;
  stat.setSessionDownloadLength(1000000);
  stat.setSessionUploadLength(500000);
  mockPeerStorage->setStat(stat);
  SharedHandle<PeerStorage> peerStorage(mockPeerStorage);
  BtRegistry::registerPeerStorage(infoHashString, peerStorage);

  MockPieceStorage* mockPieceStorage = new MockPieceStorage();
  mockPieceStorage->setCompletedLength(1000000);
  BtRegistry::registerPieceStorage(infoHashString,
				   SharedHandle<PieceStorage>(mockPieceStorage));

  ShareRatioSeedCriteria cri(1.0, btContext);
  CPPUNIT_ASSERT(cri.evaluate());
  
  cri.setRatio(2.0);
  CPPUNIT_ASSERT(!cri.evaluate());
  // check div by zero
  mockBtContext->setTotalLength(0);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
