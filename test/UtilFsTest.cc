#include "util.h"

#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#ifdef _WIN32
static char* mkdtemp(char* tpl)
{
  char* dn = mktemp(tpl);
  if (!dn) {
    return dn;
  }
  if (mkdir(dn)) {
    return nullptr;
  }
  return dn;
}
#endif // _WIN32

namespace aria2 {

class UtilFsTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilFsTest);
  CPPUNIT_TEST(testSpace);
  CPPUNIT_TEST(testSpacePwd);
  CPPUNIT_TEST(testSpaceDownwardsFile);
  CPPUNIT_TEST(testSpaceDownwardsDir);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testSpace();
  void testSpacePwd();
  void testSpaceDownwardsFile();
  void testSpaceDownwardsDir();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilFsTest);

void UtilFsTest::testSpace()
{
  const char* tmpl = "aria2.test.tmp.XXXXXX";
  char* tpl = strdup(tmpl); // lets just leak this
  CPPUNIT_ASSERT(tpl);
  char* tmp = mkdtemp(tpl);
  CPPUNIT_ASSERT(tmp);
  std::error_code ec;
  util::filesystem::space(tmp, ec);
  CPPUNIT_ASSERT(!ec);
  rmdir(tmp);
  auto rv = util::filesystem::space(tmp, ec);
  CPPUNIT_ASSERT(ec);
  CPPUNIT_ASSERT_EQUAL(rv.available, static_cast<uintmax_t>(-1));
  CPPUNIT_ASSERT_EQUAL(rv.capacity, static_cast<uintmax_t>(-1));
  CPPUNIT_ASSERT_EQUAL(rv.free, static_cast<uintmax_t>(-1));
}

void UtilFsTest::testSpacePwd()
{
  std::error_code ec;
  util::filesystem::space(nullptr, ec);
  CPPUNIT_ASSERT(!ec);
  util::filesystem::space("", ec);
  CPPUNIT_ASSERT(!ec);
  util::filesystem::space(".", ec);
  CPPUNIT_ASSERT(!ec);
  util::filesystem::space("doesnotexit", ec);
  CPPUNIT_ASSERT(ec);
  util::filesystem::space_downwards("doesnotexit", ec);
  CPPUNIT_ASSERT(!ec);
}

void UtilFsTest::testSpaceDownwardsFile()
{
  const char* tmpl = "aria2.test.tmp.XXXXXX";
  char* tpl = strdup(tmpl); // lets just leak this
  CPPUNIT_ASSERT(tpl);
  char* tmp = mkdtemp(tpl);
  CPPUNIT_ASSERT(tmp);
  std::string tn(tmp);
  tn += "/aria2.tmp";
  {
    std::ofstream s(tn);
    std::error_code ec;
    std::string tn2(tn);
    tn2 += "/something.else.entirely";
    util::filesystem::space(tn2.c_str(), ec);
    CPPUNIT_ASSERT_MESSAGE(tn2, ec);
    util::filesystem::space_downwards(tn2.c_str(), ec);
    CPPUNIT_ASSERT_MESSAGE(tn2, !ec);
  }
  unlink(tn.c_str());
  rmdir(tmp);
}

void UtilFsTest::testSpaceDownwardsDir()
{
  const char* tmpl = "aria2.test.tmp.XXXXXX";
  char* tpl = strdup(tmpl); // lets just leak this
  CPPUNIT_ASSERT(tpl);
  char* tmp = mkdtemp(tpl);
  CPPUNIT_ASSERT(tmp);
  std::string tn(tmp);
  tn += "/something.else.entirely";
  std::error_code ec;
  auto rv = util::filesystem::space(tn.c_str(), ec);
  CPPUNIT_ASSERT_MESSAGE(tn, ec);
  rv = util::filesystem::space_downwards(tn.c_str(), ec);
  rmdir(tmp);
  CPPUNIT_ASSERT_MESSAGE(tn, !ec);
}

} // namespace aria2
