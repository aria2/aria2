#include "FeedbackURISelector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "array_fun.h"
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "FileEntry.h"

namespace aria2 {

class FeedbackURISelectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FeedbackURISelectorTest);
  CPPUNIT_TEST(testSelect_withoutServerStat);
  CPPUNIT_TEST(testSelect);
  CPPUNIT_TEST(testSelect_skipErrorHost);
  CPPUNIT_TEST_SUITE_END();
private:
  FileEntry _fileEntry;

  SharedHandle<ServerStatMan> ssm;

  SharedHandle<FeedbackURISelector> sel;
  
public:
  void setUp()
  {
    static const char* urisSrc[] = {
      "http://alpha/file",
      "ftp://alpha/file",
      "http://bravo/file"
    };
    std::deque<std::string> uris;
    uris.assign(&urisSrc[0], &urisSrc[arrayLength(urisSrc)]);
    
    _fileEntry.setUris(uris);

    ssm.reset(new ServerStatMan());
    sel.reset(new FeedbackURISelector(ssm));
  }

  void tearDown() {}

  void testSelect_withoutServerStat();
  
  void testSelect();

  void testSelect_skipErrorHost();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FeedbackURISelectorTest);

void FeedbackURISelectorTest::testSelect_withoutServerStat()
{
  // Without ServerStat, selector returns first URI
  std::string uri = sel->select(&_fileEntry);
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), uri);
  CPPUNIT_ASSERT_EQUAL((size_t)2, _fileEntry.getRemainingUris().size());
}

void FeedbackURISelectorTest::testSelect()
{
  SharedHandle<ServerStat> bravo(new ServerStat("bravo", "http"));
  bravo->updateDownloadSpeed(100000);
  SharedHandle<ServerStat> alphaFTP(new ServerStat("alpha", "ftp"));
  alphaFTP->updateDownloadSpeed(80000);
  SharedHandle<ServerStat> alphaHTTP(new ServerStat("alpha", "http"));
  alphaHTTP->updateDownloadSpeed(180000);
  alphaHTTP->setError();

  ssm->add(bravo);
  ssm->add(alphaFTP);
  ssm->add(alphaHTTP);

  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&_fileEntry));
  CPPUNIT_ASSERT_EQUAL((size_t)2, _fileEntry.getRemainingUris().size());
  
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://alpha/file"),
                       sel->select(&_fileEntry));
  CPPUNIT_ASSERT_EQUAL((size_t)1, _fileEntry.getRemainingUris().size());
}

void FeedbackURISelectorTest::testSelect_skipErrorHost()
{
  SharedHandle<ServerStat> alphaHTTP(new ServerStat("alpha", "http"));
  alphaHTTP->setError();
  SharedHandle<ServerStat> alphaFTP(new ServerStat("alpha", "ftp"));
  alphaFTP->setError();

  ssm->add(alphaHTTP);
  ssm->add(alphaFTP);

  // See error URIs are removed from URI List.
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&_fileEntry));
  CPPUNIT_ASSERT_EQUAL((size_t)0, _fileEntry.getRemainingUris().size());
}

} // namespace aria2
