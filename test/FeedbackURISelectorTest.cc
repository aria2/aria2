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
  CPPUNIT_TEST(testSelect_withUsedHosts);
  CPPUNIT_TEST(testSelect_skipErrorHost);
  CPPUNIT_TEST_SUITE_END();
private:
  FileEntry fileEntry_;

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
    std::vector<std::string> uris;
    uris.assign(vbegin(urisSrc), vend(urisSrc));
    
    fileEntry_.setUris(uris);

    ssm.reset(new ServerStatMan());
    sel.reset(new FeedbackURISelector(ssm));
  }

  void tearDown() {}

  void testSelect_withoutServerStat();
  
  void testSelect();

  void testSelect_withUsedHosts();

  void testSelect_skipErrorHost();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FeedbackURISelectorTest);

void FeedbackURISelectorTest::testSelect_withoutServerStat()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  // Without ServerStat and usedHosts, selector returns first URI
  std::string uri = sel->select(&fileEntry_, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), uri);
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry_.getRemainingUris().size());
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
  std::vector<std::pair<size_t, std::string> > usedHosts;

  ssm->add(bravo);
  ssm->add(alphaFTP);
  ssm->add(alphaHTTP);

  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry_.getRemainingUris().size());
  
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://alpha/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry_.getRemainingUris().size());
}

void FeedbackURISelectorTest::testSelect_withUsedHosts()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  usedHosts.push_back(std::make_pair(1, "bravo"));
  usedHosts.push_back(std::make_pair(2, "alpha"));

  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry_.getRemainingUris().size());

  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry_.getRemainingUris().size());

  CPPUNIT_ASSERT_EQUAL(std::string("ftp://alpha/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)0, fileEntry_.getRemainingUris().size());
}

void FeedbackURISelectorTest::testSelect_skipErrorHost()
{
  SharedHandle<ServerStat> alphaHTTP(new ServerStat("alpha", "http"));
  alphaHTTP->setError();
  SharedHandle<ServerStat> alphaFTP(new ServerStat("alpha", "ftp"));
  alphaFTP->setError();
  std::vector<std::pair<size_t, std::string> > usedHosts;

  ssm->add(alphaHTTP);
  ssm->add(alphaFTP);

  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"),
                       sel->select(&fileEntry_, usedHosts));
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry_.getRemainingUris().size());
}

} // namespace aria2
