#include "SingleFileAllocationIterator.h"
#include "File.h"
#include "DefaultDiskWriter.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <cppunit/extensions/HelperMacros.h>

class SingleFileAllocationIteratorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SingleFileAllocationIteratorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testAllocate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SingleFileAllocationIteratorTest );

void SingleFileAllocationIteratorTest::testAllocate()
{
  string dir = "/tmp";
  string fname = "aria2_SingleFileAllocationIteratorTest_testAllocate";
  string fn = dir+"/"+fname;
  ofstream of(fn.c_str());
  of << "0123456789";
  of.close();

  File x(fn);
  CPPUNIT_ASSERT_EQUAL((int64_t)10, x.size());

  DefaultDiskWriter writer;
  int64_t offset = 10;
  int64_t totalLength = 16*1024*2+8*1024;

  // we have to open file first.
  writer.openExistingFile(fn);
  SingleFileAllocationIterator itr(&writer, offset, totalLength);
  itr.init();

  while(!itr.finished()) {
    itr.allocateChunk();
  }
  File f(fn);
  CPPUNIT_ASSERT_EQUAL((int64_t)40960, f.size());
}
