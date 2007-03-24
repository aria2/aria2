#include "GlowFileAllocator.h"
#include "File.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <cppunit/extensions/HelperMacros.h>

class GlowFileAllocatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GlowFileAllocatorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testAllocate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( GlowFileAllocatorTest );

void GlowFileAllocatorTest::testAllocate()
{
  string fn = "/tmp/aria2_GlowFileAllocatorTest_testAllocate";
  ofstream of(fn.c_str());

  of << "0123456789";

  of.close();

  File x("/tmp/aria2_GlowFileAllocatorTest_testAllocate");
  CPPUNIT_ASSERT_EQUAL((int64_t)10, x.size());

  int fd;
  if((fd = open(fn.c_str(), O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
    CPPUNIT_FAIL("cannot open file");
  }
  GlowFileAllocator allocator;
  allocator.allocate(fd, 4097);

  if(close(fd) < 0) {
    CPPUNIT_FAIL("cannot close file");
  }

  ifstream is(fn.c_str());

  char buf[11];
  is >> std::setw(sizeof(buf)) >> buf;

  CPPUNIT_ASSERT(strcmp("0123456789", buf) == 0);

  File f(fn);
  CPPUNIT_ASSERT_EQUAL((int64_t)4097, f.size());
}
