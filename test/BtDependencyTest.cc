#include "BtDependency.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "DefaultPieceStorage.h"
#include "DownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "Exception.h"
#include "SegmentMan.h"
#include "FileEntry.h"
#include "PieceSelector.h"
#include "bittorrent_helper.h"

namespace aria2 {

class BtDependencyTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtDependencyTest);
  CPPUNIT_TEST(testResolve);
  CPPUNIT_TEST(testResolve_loadError);
  CPPUNIT_TEST(testResolve_dependeeFailure);
  CPPUNIT_TEST(testResolve_dependeeInProgress);
  CPPUNIT_TEST_SUITE_END();

  SharedHandle<RequestGroup> createDependant(const SharedHandle<Option>& option)
  {
    SharedHandle<RequestGroup> dependant(new RequestGroup(option));
    SharedHandle<DownloadContext> dctx
      (new DownloadContext(0, 0, "/tmp/outfile.path"));
    dctx->setDir("/tmp");
    dependant->setDownloadContext(dctx);
    return dependant;
  }

  SharedHandle<RequestGroup>
  createDependee
  (const SharedHandle<Option>& option,
   const std::string& torrentFile,
   int64_t length)
  {
    SharedHandle<RequestGroup> dependee(new RequestGroup(option));
    SharedHandle<DownloadContext> dctx
      (new DownloadContext(1024*1024, length, torrentFile));
    dctx->setDir(".");
    dependee->setDownloadContext(dctx);
    DefaultPieceStorageHandle ps(new DefaultPieceStorage(dctx, option.get()));
    dependee->setPieceStorage(ps);
    ps->initStorage();
    dependee->initSegmentMan();
    return dependee;
  }

  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
  }

  void testResolve();
  void testResolve_loadError();
  void testResolve_dependeeFailure();
  void testResolve_dependeeInProgress();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtDependencyTest );

void BtDependencyTest::testResolve()
{
  std::string filename = "single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(_option);
  SharedHandle<RequestGroup> dependee =
    createDependee(_option, filename, File(filename).size());
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant, dependee);
  CPPUNIT_ASSERT(dep.resolve());

  CPPUNIT_ASSERT_EQUAL
    (std::string("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf"),
     bittorrent::getInfoHashString(dependant->getDownloadContext()));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
		       dependant->getFirstFilePath());
}

void BtDependencyTest::testResolve_loadError()
{
  SharedHandle<RequestGroup> dependant = createDependant(_option);
  SharedHandle<RequestGroup> dependee =
    createDependee(_option, "notExist", 40);
  dependee->getPieceStorage()->markAllPiecesDone();
    
  BtDependency dep(dependant, dependee);
  CPPUNIT_ASSERT(dep.resolve());
    
  CPPUNIT_ASSERT
    (!dependant->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
		       dependant->getFirstFilePath());
}

void BtDependencyTest::testResolve_dependeeFailure()
{
  SharedHandle<RequestGroup> dependant = createDependant(_option);
  SharedHandle<RequestGroup> dependee = createDependee(_option, "notExist", 40);
    
  BtDependency dep(dependant, dependee);
  CPPUNIT_ASSERT(dep.resolve());
  
  CPPUNIT_ASSERT
    (!dependant->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
		       dependant->getFirstFilePath());
}

void BtDependencyTest::testResolve_dependeeInProgress()
{
  std::string filename = "single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(_option);
  SharedHandle<RequestGroup> dependee =
    createDependee(_option, filename, File(filename).size());
  dependee->increaseNumCommand();

  BtDependency dep(dependant, dependee);
  CPPUNIT_ASSERT(!dep.resolve());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
		       dependant->getFirstFilePath());
}

} // namespace aria2
