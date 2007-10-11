#include "MultiFileAllocationIterator.h"
#include "File.h"
#include "MultiDiskAdaptor.h"
#include <cppunit/extensions/HelperMacros.h>

class MultiFileAllocationIteratorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiFileAllocationIteratorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testAllocate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiFileAllocationIteratorTest );

void MultiFileAllocationIteratorTest::testAllocate()
{
  string dir = "/tmp";
  string topDir = "aria2_MultiFileAllocationIteratorTest_testAllocate";
  string fname1 = "file1";
  string fname2 = "file2";
  string fname3 = "file3";
  string fname4 = "file4";
  string fname5 = "file5";
  string fname6 = "file6";
  int64_t length1 = 32769;
  int64_t length2 = 0;
  int64_t length3 = 8;
  int64_t length4 = 10;
  int64_t length5 = 20;
  int64_t length6 = 30;

  try {
  MultiDiskAdaptorHandle diskAdaptor = new MultiDiskAdaptor();
  diskAdaptor->setStoreDir(dir);
  diskAdaptor->setTopDir(topDir);

  int64_t offset = 0;
  FileEntryHandle fileEntry1 = new FileEntry(fname1,
					     length1,
					     offset);
  offset += length1;
  FileEntryHandle fileEntry2 = new FileEntry(fname2,
					     length2,
					     offset);

  offset += length2;
  FileEntryHandle fileEntry3 = new FileEntry(fname3,
					     length3,
					     offset);

  offset += length3;
  FileEntryHandle fileEntry4 = new FileEntry(fname4,
					     length4,
					     offset);
  fileEntry4->setRequested(false);

  offset += length4;
  FileEntryHandle fileEntry5 = new FileEntry(fname5,
					     length5,
					     offset);

  offset += length5;
  FileEntryHandle fileEntry6 = new FileEntry(fname6,
					     length6,
					     offset);
  fileEntry6->setRequested(false);

  FileEntries fs;
  fs.push_back(fileEntry1);
  fs.push_back(fileEntry2);
  fs.push_back(fileEntry3);
  fs.push_back(fileEntry4);
  fs.push_back(fileEntry5);
  fs.push_back(fileEntry6);
  diskAdaptor->setFileEntries(fs);

  // we have to open file first.
  diskAdaptor->initAndOpenFile();
  MultiFileAllocationIteratorHandle itr = diskAdaptor->fileAllocationIterator();
  while(!itr->finished()) {
    itr->allocateChunk();
  }
  CPPUNIT_ASSERT_EQUAL((int64_t)length1, File(dir+"/"+topDir+"/"+fname1).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)length2, File(dir+"/"+topDir+"/"+fname2).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)length3, File(dir+"/"+topDir+"/"+fname3).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)0, File(dir+"/"+topDir+"/"+fname4).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)length5, File(dir+"/"+topDir+"/"+fname5).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)0, File(dir+"/"+topDir+"/"+fname6).size());

  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
    CPPUNIT_FAIL("exception was thrown");
  }
}
