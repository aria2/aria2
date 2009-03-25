#include "FallocFileAllocationIterator.h"

#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include "File.h"
#include "DefaultDiskWriter.h"

namespace aria2 {

class FallocFileAllocationIteratorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FallocFileAllocationIteratorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testAllocate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( FallocFileAllocationIteratorTest );

void FallocFileAllocationIteratorTest::testAllocate()
{
  std::string dir = "/tmp";
  std::string fname = "aria2_FallocFileAllocationIteratorTest_testAllocate";
  std::string fn = dir+"/"+fname;
  std::ofstream of(fn.c_str(), std::ios::binary);
  of << "0123456789";
  of.close();

  File f(fn);
  CPPUNIT_ASSERT_EQUAL((uint64_t)10, f.size());

  DefaultDiskWriter writer;
  int64_t offset = 10;
  int64_t totalLength = 40960;

  // we have to open file first.
  writer.openExistingFile(fn);
  FallocFileAllocationIterator itr(&writer, offset, totalLength);

  itr.allocateChunk();
  CPPUNIT_ASSERT(itr.finished());

  CPPUNIT_ASSERT_EQUAL((uint64_t)40960, f.size());
}

} // namespace aria2
