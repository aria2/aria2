#include "SingleFileAllocationIterator.h"
#include "File.h"
#include "DefaultDiskWriter.h"
#include "DirectDiskAdaptor.h"
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


  DefaultDiskWriterHandle writer = new DefaultDiskWriter();
  DirectDiskAdaptorHandle diskAdaptor = new DirectDiskAdaptor();
  diskAdaptor->setDiskWriter(writer);
  diskAdaptor->setTotalLength(16*1024*2+8*1024);
  diskAdaptor->setStoreDir(dir);
  FileEntryHandle fileEntry = new FileEntry(fname,
					    diskAdaptor->getTotalLength(),
					    0);
  FileEntries fs;
  fs.push_back(fileEntry);
  diskAdaptor->setFileEntries(fs);

  // we have to open file first.
  diskAdaptor->openFile();
  SingleFileAllocationIteratorHandle itr = diskAdaptor->fileAllocationIterator();

  itr->allocateChunk();
  CPPUNIT_ASSERT(!itr->finished());
  itr->allocateChunk();
  CPPUNIT_ASSERT(!itr->finished());
  itr->allocateChunk();
  CPPUNIT_ASSERT(itr->finished());

  File f(fn);
  CPPUNIT_ASSERT_EQUAL((int64_t)40960, f.size());
}
