#include "ShareRatioSeedCriteria.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "BtRuntime.h"
#include "MockPieceStorage.h"
#include "FileEntry.h"

namespace aria2 {

class ShareRatioSeedCriteriaTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ShareRatioSeedCriteriaTest);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testEvaluate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ShareRatioSeedCriteriaTest);

void ShareRatioSeedCriteriaTest::testEvaluate()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext(1_m, 1000000));
  std::shared_ptr<BtRuntime> btRuntime(new BtRuntime());
  btRuntime->setUploadLengthAtStartup(1000000);

  std::shared_ptr<MockPieceStorage> pieceStorage(new MockPieceStorage());
  pieceStorage->setCompletedLength(1000000);

  ShareRatioSeedCriteria cri(1.0, dctx);
  cri.setBtRuntime(btRuntime);
  cri.setPieceStorage(pieceStorage);

  CPPUNIT_ASSERT(cri.evaluate());

  cri.setRatio(2.0);
  CPPUNIT_ASSERT(!cri.evaluate());
  // check div by zero
  dctx->getFirstFileEntry()->setLength(0);
  CPPUNIT_ASSERT(!cri.evaluate());
}

} // namespace aria2
