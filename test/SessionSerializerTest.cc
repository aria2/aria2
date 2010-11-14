#include "SessionSerializer.h"

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroupMan.h"
#include "array_fun.h"
#include "download_helper.h"
#include "prefs.h"
#include "Option.h"
#include "a2functional.h"

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
      "test.torrent",
      "serialize_session.meta4",
      "magnet:?xt=urn:btih:248D0A1CD08284299DE78D5C1ED359BB46717D8C"};
  std::vector<std::string> uris(vbegin(URIs), vend(URIs));
  std::vector<SharedHandle<RequestGroup> > result;
  SharedHandle<Option> option(new Option());
  option->put(PREF_DIR, "/tmp");
  createRequestGroupForUri(result, option, uris);
  CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
  SharedHandle<RequestGroupMan> rgman
    (new RequestGroupMan(result, 1, option.get()));
  SessionSerializer s(rgman);
  std::stringstream ss;
  s.save(ss);
  std::string line;
  std::getline(ss, line);
  CPPUNIT_ASSERT_EQUAL(strconcat(uris[0], "\t", uris[1], "\t"), line);
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
