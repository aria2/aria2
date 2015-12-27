#include "FallocFileAllocationIterator.h"

#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include "a2functional.h"
#include "File.h"
#include "DefaultDiskWriter.h"

namespace aria2 {

class FallocFileAllocationIteratorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FallocFileAllocationIteratorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testAllocate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FallocFileAllocationIteratorTest);

void FallocFileAllocationIteratorTest::testAllocate()
{
// When fallocate is used, test fails if file system does not
// support it. So skip it.
#ifndef HAVE_FALLOCATE
  std::string fn =
      A2_TEST_OUT_DIR "/aria2_FallocFileAllocationIteratorTest_testAllocate";
  std::ofstream of(fn.c_str(), std::ios::binary);
  of << "0123456789";
  of.close();

  File f(fn);
  CPPUNIT_ASSERT_EQUAL((int64_t)10, f.size());

  DefaultDiskWriter writer(fn);
  int64_t offset = 10;
  int64_t totalLength = 40_k;

  // we have to open file first.
  writer.openExistingFile();
  FallocFileAllocationIterator itr(&writer, offset, totalLength);

  itr.allocateChunk();
  CPPUNIT_ASSERT(itr.finished());

  CPPUNIT_ASSERT_EQUAL((int64_t)40_k, f.size());
#endif // !HAVE_FALLOCATE
}

} // namespace aria2
