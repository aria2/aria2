#include "BtDependency.h"
#include "SingleFileDownloadContext.h"
#include "DefaultPieceStorage.h"
#include "BtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "Exception.h"
#include "SegmentMan.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtDependencyTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtDependencyTest);
  CPPUNIT_TEST(testResolve);
  CPPUNIT_TEST(testResolve_loadError);
  CPPUNIT_TEST(testResolve_dependeeFailure);
  CPPUNIT_TEST(testResolve_dependeeInProgress);
  CPPUNIT_TEST_SUITE_END();

  SharedHandle<RequestGroup> createDependant(const Option* option)
  {
    SharedHandle<RequestGroup> dependant = new RequestGroup(option, std::deque<std::string>());
    SharedHandle<SingleFileDownloadContext> dctx =
      new SingleFileDownloadContext(0, 0, "");
    dctx->setDir("/tmp");
    dependant->setDownloadContext(dctx);
    return dependant;
  }

  SharedHandle<RequestGroup>
  createDependee(const Option* option, const std::string& torrentFile, int64_t length)
  {
    SharedHandle<RequestGroup> dependee = new RequestGroup(option, std::deque<std::string>());
    SharedHandle<SingleFileDownloadContext> dctx =
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
  std::string filename = "test.torrent";
  Option option;
  SharedHandle<RequestGroup> dependant = createDependant(&option);
  SharedHandle<RequestGroup> dependee = createDependee(&option, filename, File(filename).size());
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(dep.resolve());
  
  SharedHandle<BtContext> btContext = dependant->getDownloadContext();
  CPPUNIT_ASSERT(!btContext.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-test"), btContext->getActualBasePath());
}

void BtDependencyTest::testResolve_loadError()
{
  try {
    Option option;
    SharedHandle<RequestGroup> dependant = createDependant(&option);
    SharedHandle<RequestGroup> dependee = createDependee(&option, "notExist", 40);
    dependee->getPieceStorage()->markAllPiecesDone();
    
    BtDependency dep(dependant, dependee, &option);
    CPPUNIT_ASSERT(dep.resolve());
    
    SharedHandle<SingleFileDownloadContext> dctx = dependant->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/index.html"), dctx->getActualBasePath());
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
    CPPUNIT_FAIL("an exception was thrown.");
  }
}

void BtDependencyTest::testResolve_dependeeFailure()
{
  Option option;
  SharedHandle<RequestGroup> dependant = createDependant(&option);
  SharedHandle<RequestGroup> dependee = createDependee(&option, "notExist", 40);
    
  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(dep.resolve());
  
  SharedHandle<SingleFileDownloadContext> dctx = dependant->getDownloadContext();
  CPPUNIT_ASSERT(!dctx.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/index.html"), dctx->getActualBasePath());
}

void BtDependencyTest::testResolve_dependeeInProgress()
{
  std::string filename = "test.torrent";
  Option option;
  SharedHandle<RequestGroup> dependant = createDependant(&option);
  SharedHandle<RequestGroup> dependee = createDependee(&option, filename, File(filename).size());
  dependee->increaseNumCommand();

  BtDependency dep(dependant, dependee, &option);
  CPPUNIT_ASSERT(!dep.resolve());
}

} // namespace aria2
