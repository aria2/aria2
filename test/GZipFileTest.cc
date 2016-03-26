#include "GZipFile.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "File.h"

namespace aria2 {

class GZipFileTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GZipFileTest);
  CPPUNIT_TEST(testOpen);
  CPPUNIT_TEST_SUITE_END();

public:
  void testOpen();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GZipFileTest);

void GZipFileTest::testOpen()
{
  File f(A2_TEST_OUT_DIR "/aria2_GZipFileTest_testOpen");
  f.remove();
  GZipFile fail(f.getPath().c_str(), GZipFile::READ);
  CPPUNIT_ASSERT(!fail);

  GZipFile wr(f.getPath().c_str(), GZipFile::WRITE);
  CPPUNIT_ASSERT(wr);
  std::string msg = "aria2 rules\nalpha\nbravo\ncharlie";
  wr.write(msg.data(), msg.size());
  wr.close();

  GZipFile rd(f.getPath().c_str(), GZipFile::READ);
  char buf[256];
  size_t len = rd.read(buf, 11);
  CPPUNIT_ASSERT_EQUAL((size_t)11, len);
  buf[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("aria2 rules"), std::string(buf));

  CPPUNIT_ASSERT(rd.gets(buf, sizeof(buf)));
  CPPUNIT_ASSERT_EQUAL(std::string("\n"), std::string(buf));

  CPPUNIT_ASSERT(rd.gets(buf, sizeof(buf)));
  CPPUNIT_ASSERT_EQUAL(std::string("alpha\n"), std::string(buf));

  CPPUNIT_ASSERT(rd.getsn(buf, sizeof(buf)));
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), std::string(buf));

  CPPUNIT_ASSERT(rd.getsn(buf, sizeof(buf)));
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), std::string(buf));

  CPPUNIT_ASSERT(rd.eof());
}

} // namespace aria2
