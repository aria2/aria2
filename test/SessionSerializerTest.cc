#include "SessionSerializer.h"

#include <iostream>
#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroupMan.h"
#include "array_fun.h"
#include "download_helper.h"
#include "prefs.h"
#include "Option.h"
#include "a2functional.h"
#include "FileEntry.h"

namespace aria2 {

class SessionSerializerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SessionSerializerTest);
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST_SUITE_END();
public:
  void testSave();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SessionSerializerTest);

namespace {
SharedHandle<DownloadResult> createDownloadResult
(error_code::Value result, const std::string& uri)
{
  std::vector<std::string> uris;
  uris.push_back(uri);
  SharedHandle<FileEntry> entry(new FileEntry("/tmp/path", 1, 0, uris));
  std::vector<SharedHandle<FileEntry> > entries;
  entries.push_back(entry);
  SharedHandle<DownloadResult> dr(new DownloadResult());
  dr->fileEntries = entries;
  dr->result = result;
  dr->belongsTo = 0;
  dr->inMemoryDownload = false;
  dr->option = SharedHandle<Option>(new Option());
  return dr;
}
} // namespace

void SessionSerializerTest::testSave()
{
#if defined(ENABLE_BITTORRENT) && defined(ENABLE_METALINK)
  const std::string URIs[] =
    { "http://localhost/file",
      "http://mirror/file",
      A2_TEST_DIR"/test.torrent",
      A2_TEST_DIR"/serialize_session.meta4",
      "magnet:?xt=urn:btih:248D0A1CD08284299DE78D5C1ED359BB46717D8C"};
  std::vector<std::string> uris(vbegin(URIs), vend(URIs));
  std::vector<SharedHandle<RequestGroup> > result;
  SharedHandle<Option> option(new Option());
  option->put(PREF_DIR, "/tmp");
  createRequestGroupForUri(result, option, uris);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
  option->put(PREF_MAX_DOWNLOAD_RESULT, "10");
  SharedHandle<RequestGroupMan> rgman
    (new RequestGroupMan(result, 1, option.get()));
  SessionSerializer s(rgman);
  // REMOVED downloads will not be saved.
  rgman->addDownloadResult
    (createDownloadResult(error_code::REMOVED, "http://removed"));
  rgman->addDownloadResult
    (createDownloadResult(error_code::TIME_OUT, "http://error"));
  std::string filename = A2_TEST_OUT_DIR"/aria2_SessionSerializerTest_testSave";
  s.save(filename);
  std::ifstream ss(filename.c_str(), std::ios::binary);
  std::string line;
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string("http://error\t"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[0]+"\t"+uris[1]+"\t", line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[2], line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[3], line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[4], line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT(!ss);
#endif // defined(ENABLE_BITTORRENT) && defined(ENABLE_METALINK)
}

} // namespace aria2
