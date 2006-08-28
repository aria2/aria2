#include "ShareRatioSeedCriteria.h"

#include <cppunit/extensions/HelperMacros.h>

class ShareRatioSeedCriteriaTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ShareRatioSeedCriteriaTest);
  CPPUNIT_TEST(testEvaluate);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testEvaluate();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ShareRatioSeedCriteriaTest);

void ShareRatioSeedCriteriaTest::testEvaluate() {
  TorrentMan torrentMan;
  torrentMan.setDownloadLength(4294967296LL);
  torrentMan.setUploadLength(4294967296LL);

  ShareRatioSeedCriteria cri(1.0, &torrentMan);
  CPPUNIT_ASSERT(cri.evaluate());
  
  cri.setRatio(2.0);
  CPPUNIT_ASSERT(!cri.evaluate());
  // check div by zero
  torrentMan.setDownloadLength(0);
  CPPUNIT_ASSERT(!cri.evaluate());
}
