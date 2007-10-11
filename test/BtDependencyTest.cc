#include "BtDependency.h"
#include "SingleFileDownloadContext.h"
#include "DefaultPieceStorage.h"
#include "BtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "Exception.h"
#include "SegmentMan.h"
#include <cppunit/extensions/HelperMacros.h>

class BtDependencyTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtDependencyTest);
  CPPUNIT_TEST(testResolve);
  CPPUNIT_TEST(testResolve_loadError);
  CPPUNIT_TEST(testResolve_dependeeFailure);
  CPPUNIT_TEST(testResolve_dependeeInProgress);
  CPPUNIT_TEST_SUITE_END();

  RequestGroupHandle createDependant(const Option* option)
  {
    RequestGroupHandle dependant = new RequestGroup(option, Strings());
    SingleFileDownloadContextHandle dctx =
      new SingleFileDownloadContext(0, 0, "");
    dctx->setDir("/tmp");
    dependant->setDownloadContext(dctx);
    return dependant;
  }

  RequestGroupHandle createDependee(const Option* option, const string& torrentFile, int64_t length)
  {
    RequestGroupHandle dependee = new RequestGroup(option, Strings());
    SingleFileDownloadContextHandle dctx =
      new SingleFileDownloadContext(1024*1024, length, torrentFile);
    dctx->setDir(".");
    dependee->setDownloadContext(dctx);
    DefaultPieceStorageHandle ps = new DefaultPieceStorage(dctx, option);
    dependee->setPieceStorage(ps);
    ps->initStorage();
    dependee->initSegmentMan();
    return dependee;
  }

public:
  void setUp() {}

  void testResolve();
  void testResolve_loadError();
  void testResolve_dependeeFailure();
  void testResolve_dependeeInProgress();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtDependencyTest );

void BtDependencyTest::testResolve()
{
  string filename = "test.torrent";
  Option option;
  RequestGroupHandle dependant = createDependant(&option);
  RequestGroupHandle dependee = createDependee(&option, filename, File(filename).size());
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(dep.resolve());
  
  BtContextHandle btContext = dependant->getDownloadContext();
  CPPUNIT_ASSERT(!btContext.isNull());
  CPPUNIT_ASSERT_EQUAL(string("/tmp/aria2-test"), btContext->getActualBasePath());
}

void BtDependencyTest::testResolve_loadError()
{
  try {
    Option option;
    RequestGroupHandle dependant = createDependant(&option);
    RequestGroupHandle dependee = createDependee(&option, "notExist", 40);
    dependee->getPieceStorage()->markAllPiecesDone();
    
    BtDependency dep(dependant, dependee, &option);
    CPPUNIT_ASSERT(dep.resolve());
    
    SingleFileDownloadContextHandle dctx = dependant->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL(string("/tmp/index.html"), dctx->getActualBasePath());
  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
    CPPUNIT_FAIL("an exception was thrown.");
  }
}

void BtDependencyTest::testResolve_dependeeFailure()
{
  Option option;
  RequestGroupHandle dependant = createDependant(&option);
  RequestGroupHandle dependee = createDependee(&option, "notExist", 40);
    
  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(dep.resolve());
  
  SingleFileDownloadContextHandle dctx = dependant->getDownloadContext();
  CPPUNIT_ASSERT(!dctx.isNull());
  CPPUNIT_ASSERT_EQUAL(string("/tmp/index.html"), dctx->getActualBasePath());
}

void BtDependencyTest::testResolve_dependeeInProgress()
{
  string filename = "test.torrent";
  Option option;
  RequestGroupHandle dependant = createDependant(&option);
  RequestGroupHandle dependee = createDependee(&option, filename, File(filename).size());
  dependee->increaseNumCommand();

  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(!dep.resolve());
}
