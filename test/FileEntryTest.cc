#include "FileEntry.h"

#include <cppunit/extensions/HelperMacros.h>

#include "InOrderURISelector.h"
#include "util.h"

namespace aria2 {

class FileEntryTest : public CppUnit::TestFixture {
  
  CPPUNIT_TEST_SUITE(FileEntryTest);
  CPPUNIT_TEST(testSetupDir);
  CPPUNIT_TEST(testRemoveURIWhoseHostnameIs);
  CPPUNIT_TEST(testExtractURIResult);
  CPPUNIT_TEST(testGetRequest);
  CPPUNIT_TEST(testGetRequest_withoutUriReuse);
  CPPUNIT_TEST(testGetRequest_withUniqueProtocol);
  CPPUNIT_TEST(testReuseUri);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testAddUris);
  CPPUNIT_TEST(testInsertUri);
  CPPUNIT_TEST(testRemoveUri);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  
  void testSetupDir();
  void testRemoveURIWhoseHostnameIs();
  void testExtractURIResult();
  void testGetRequest();
  void testGetRequest_withoutUriReuse();
  void testGetRequest_withUniqueProtocol();
  void testReuseUri();
  void testAddUri();
  void testAddUris();
  void testInsertUri();
  void testRemoveUri();
};


CPPUNIT_TEST_SUITE_REGISTRATION( FileEntryTest );

static SharedHandle<FileEntry> createFileEntry()
{
  const char* uris[] = { "http://localhost/aria2.zip",
                         "ftp://localhost/aria2.zip",
                         "http://mirror/aria2.zip" };
  SharedHandle<FileEntry> fileEntry(new FileEntry());
  fileEntry->setUris(std::vector<std::string>(&uris[0], &uris[3]));
  return fileEntry;
}

void FileEntryTest::testSetupDir()
{
  std::string dir = "./aria2-FileEntryTest-testSetupDir";
  std::string filename = "filename";
  std::string path = dir+"/"+filename;
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  FileEntry fileEntry(path, 0, 0);
  fileEntry.setupDir();
  CPPUNIT_ASSERT(d.isDir());
  File f(path);
  CPPUNIT_ASSERT(!f.exists());
}

void FileEntryTest::testRemoveURIWhoseHostnameIs()
{
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->removeURIWhoseHostnameIs("localhost");
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
                       fileEntry->getRemainingUris()[0]);
}


void FileEntryTest::testExtractURIResult()
{
  FileEntry fileEntry;
  fileEntry.addURIResult("http://timeout/file", downloadresultcode::TIME_OUT);
  fileEntry.addURIResult("http://finished/file", downloadresultcode::FINISHED);
  fileEntry.addURIResult("http://timeout/file2", downloadresultcode::TIME_OUT);
  fileEntry.addURIResult("http://unknownerror/file", downloadresultcode::UNKNOWN_ERROR);

  std::deque<URIResult> res;
  fileEntry.extractURIResult(res, downloadresultcode::TIME_OUT);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://timeout/file"), res[0].getURI());
  CPPUNIT_ASSERT_EQUAL(std::string("http://timeout/file2"), res[1].getURI());

  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry.getURIResults().size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://finished/file"),
                       fileEntry.getURIResults()[0].getURI());
  CPPUNIT_ASSERT_EQUAL(std::string("http://unknownerror/file"),
                       fileEntry.getURIResults()[1].getURI());

  res.clear();

  fileEntry.extractURIResult(res, downloadresultcode::TIME_OUT);
  CPPUNIT_ASSERT(res.empty());
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry.getURIResults().size());
}

void FileEntryTest::testGetRequest()
{
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  SharedHandle<InOrderURISelector> selector(new InOrderURISelector());
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<Request> req =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());
  fileEntry->poolRequest(req);

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req3rd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req3rd->getProtocol());

  SharedHandle<Request> req4th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(req4th.isNull());

  fileEntry->setMaxConnectionPerServer(2);
  
  SharedHandle<Request> req5th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req5th->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req5th->getProtocol());

  SharedHandle<Request> req6th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req6th->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req6th->getProtocol());

  SharedHandle<Request> req7th =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(req7th.isNull());
}

void FileEntryTest::testGetRequest_withoutUriReuse()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setMaxConnectionPerServer(2);
  SharedHandle<InOrderURISelector> selector(new InOrderURISelector());
  SharedHandle<Request> req = fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req3rd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req3rd->getProtocol());

  SharedHandle<Request> req4th =
    fileEntry->getRequest(selector, false, usedHosts);
  CPPUNIT_ASSERT(req4th.isNull());
}

void FileEntryTest::testGetRequest_withUniqueProtocol()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setUniqueProtocol(true);
  SharedHandle<InOrderURISelector> selector(new InOrderURISelector());
  SharedHandle<Request> req =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req->getProtocol());

  SharedHandle<Request> req2nd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT_EQUAL(std::string("mirror"), req2nd->getHost());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), req2nd->getProtocol());

  SharedHandle<Request> req3rd =
    fileEntry->getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(req3rd.isNull());

  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"),
                       fileEntry->getRemainingUris()[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"),
                       fileEntry->getRemainingUris()[1]);
}

void FileEntryTest::testReuseUri()
{
  SharedHandle<InOrderURISelector> selector(new InOrderURISelector());
  SharedHandle<FileEntry> fileEntry = createFileEntry();
  fileEntry->setMaxConnectionPerServer(3);
  size_t numUris = fileEntry->getRemainingUris().size();
  std::vector<std::pair<size_t, std::string> > usedHosts;
  for(size_t i = 0; i < numUris; ++i) {
    fileEntry->getRequest(selector, false, usedHosts);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)0, fileEntry->getRemainingUris().size());
  fileEntry->addURIResult("http://localhost/aria2.zip",
                          downloadresultcode::UNKNOWN_ERROR);
  std::vector<std::string> ignore;
  fileEntry->reuseUri(ignore);
  CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntry->getRemainingUris().size());
  std::deque<std::string> uris = fileEntry->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.zip"), uris[1]);
  for(size_t i = 0; i < 2; ++i) {
    fileEntry->getRequest(selector, false, usedHosts);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)0, fileEntry->getRemainingUris().size());
  ignore.clear();
  ignore.push_back("mirror");
  fileEntry->reuseUri(ignore);
  CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntry->getRemainingUris().size());
  uris = fileEntry->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.zip"), uris[0]);
}

void FileEntryTest::testAddUri()
{
  FileEntry file;
  CPPUNIT_ASSERT(file.addUri("http://good"));
  CPPUNIT_ASSERT(!file.addUri("bad"));
}

void FileEntryTest::testAddUris()
{
  FileEntry file;
  std::string uris[] = {"bad", "http://good"};
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.addUris(&uris[0], &uris[2]));
}

void FileEntryTest::testInsertUri()
{
  FileEntry file;
  CPPUNIT_ASSERT(file.insertUri("http://example.org/1", 0));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/2", 0));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/3", 1));
  CPPUNIT_ASSERT(file.insertUri("http://example.org/4", 5));
  std::deque<std::string> uris = file.getRemainingUris();
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/2"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/3"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/1"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/4"), uris[3]);
}

void FileEntryTest::testRemoveUri()
{
  std::vector<std::pair<size_t, std::string> > usedHosts;
  SharedHandle<InOrderURISelector> selector(new InOrderURISelector());
  FileEntry file;
  file.addUri("http://example.org/");
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT(file.getRemainingUris().empty());
  CPPUNIT_ASSERT(!file.removeUri("http://example.org/"));

  file.addUri("http://example.org/");
  SharedHandle<Request> exampleOrgReq =
    file.getRequest(selector, true, usedHosts);
  CPPUNIT_ASSERT(!exampleOrgReq->removalRequested());
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.getSpentUris().size());
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT(file.getSpentUris().empty());
  CPPUNIT_ASSERT(exampleOrgReq->removalRequested());
  file.poolRequest(exampleOrgReq);
  CPPUNIT_ASSERT_EQUAL((size_t)0, file.countPooledRequest());

  file.addUri("http://example.org/");
  exampleOrgReq = file.getRequest(selector, true, usedHosts);
  file.poolRequest(exampleOrgReq);
  CPPUNIT_ASSERT_EQUAL((size_t)1, file.countPooledRequest());
  CPPUNIT_ASSERT(file.removeUri("http://example.org/"));
  CPPUNIT_ASSERT_EQUAL((size_t)0, file.countPooledRequest());
  CPPUNIT_ASSERT(file.getSpentUris().empty());

  file.addUri("http://example.org/");
  CPPUNIT_ASSERT(!file.removeUri("http://example.net"));
}

} // namespace aria2
