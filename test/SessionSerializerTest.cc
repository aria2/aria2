#include "SessionSerializer.h"

#include <iostream>
#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "RequestGroupMan.h"
#include "array_fun.h"
#include "download_helper.h"
#include "prefs.h"
#include "Option.h"
#include "a2functional.h"
#include "FileEntry.h"
#include "SelectEventPoll.h"
#include "DownloadEngine.h"

namespace aria2 {

class SessionSerializerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SessionSerializerTest);
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST_SUITE_END();
public:
  void testSave();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SessionSerializerTest);

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
  result[4]->getOption()->put(PREF_PAUSE, A2_V_TRUE);
  option->put(PREF_MAX_DOWNLOAD_RESULT, "10");
  SharedHandle<RequestGroupMan> rgman
    (new RequestGroupMan(result, 1, option.get()));
  SessionSerializer s(rgman);
  SharedHandle<DownloadResult> drs[] = {
    // REMOVED downloads will not be saved.
    createDownloadResult(error_code::REMOVED, "http://removed"),
    createDownloadResult(error_code::TIME_OUT, "http://error"),
    createDownloadResult(error_code::FINISHED, "http://finished"),
    createDownloadResult(error_code::FINISHED, "http://force-save")
  };
  drs[3]->option->put(PREF_FORCE_SAVE, A2_V_TRUE);
  for(size_t i = 0; i < sizeof(drs)/sizeof(drs[0]); ++i) {
    rgman->addDownloadResult(drs[i]);
  }

  DownloadEngine e(SharedHandle<EventPoll>(new SelectEventPoll()));
  e.setOption(option.get());
  rgman->fillRequestGroupFromReserver(&e);
  CPPUNIT_ASSERT_EQUAL((size_t)1, rgman->getRequestGroups().size());

  std::string filename = A2_TEST_OUT_DIR"/aria2_SessionSerializerTest_testSave";
  s.save(filename);
  std::ifstream ss(filename.c_str(), std::ios::binary);
  std::string line;
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string("http://error\t"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(fmt(" gid=%s", drs[1]->gid->toHex().c_str()), line);
  std::getline(ss, line);
  // finished and force-save option
  CPPUNIT_ASSERT_EQUAL(std::string("http://force-save\t"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(fmt(" gid=%s", drs[3]->gid->toHex().c_str()), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" force-save=true"), line);
  // Check active download is also saved
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[0]+"\t"+uris[1]+"\t", line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(fmt(" gid=%s",
                           GroupId::toHex(result[0]->getGID()).c_str()),
                       line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[2], line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(fmt(" gid=%s",
                           GroupId::toHex(result[1]->getGID()).c_str()),
                       line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[3], line);
  std::getline(ss, line);
  // local metalink download does not save meaningful GID
  CPPUNIT_ASSERT(fmt(" gid=%s",
                     GroupId::toHex(result[2]->getGID()).c_str())
                 != line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(uris[4], line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(fmt(" gid=%s",
                           GroupId::toHex(result[4]->getGID()).c_str()),
                       line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" dir=/tmp"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(std::string(" pause=true"), line);
  std::getline(ss, line);
  CPPUNIT_ASSERT(!ss);
#endif // defined(ENABLE_BITTORRENT) && defined(ENABLE_METALINK)
}

} // namespace aria2
