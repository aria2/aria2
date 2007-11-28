#include "MultiFileAllocationIterator.h"
#include "File.h"
#include "MultiDiskAdaptor.h"
#include <cppunit/extensions/HelperMacros.h>

class MultiFileAllocationIteratorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiFileAllocationIteratorTest);
  CPPUNIT_TEST(testAllocate);
  CPPUNIT_TEST(testMakeDiskWriterEntries);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testAllocate();
  void testMakeDiskWriterEntries();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiFileAllocationIteratorTest );

void MultiFileAllocationIteratorTest::testMakeDiskWriterEntries()
{
  FileEntryHandle fs[] = {
    new FileEntry("file1", 1536, 0),
    new FileEntry("file2", 2048, 1536),
    new FileEntry("file3", 1024, 3584),
    new FileEntry("file4", 1024, 4608),
    new FileEntry("file5", 1024, 5632),
    new FileEntry("file6", 1024, 6656),
    new FileEntry("file7",  256, 7680),
    new FileEntry("file8",  768, 7936),
    new FileEntry("file9",  256, 8704),
    new FileEntry("fileA",  256, 8960),
  };
  fs[1]->setRequested(false);
  fs[3]->setRequested(false);
  fs[4]->setRequested(false);
  fs[5]->setRequested(false);
  fs[6]->setRequested(false);
  fs[8]->setRequested(false);
  fs[9]->setRequested(false);
  
  string storeDir = "/tmp/aria2_MultiFileAllocationIteratorTest_testMakeDiskWriterEntries";
  MultiDiskAdaptorHandle diskAdaptor = new MultiDiskAdaptor();
  diskAdaptor->setFileEntries(FileEntries(&fs[0], &fs[10]));
  diskAdaptor->setPieceLength(1024);
  diskAdaptor->setStoreDir(storeDir);
  diskAdaptor->openFile();

  MultiFileAllocationIteratorHandle itr = diskAdaptor->fileAllocationIterator();

  DiskWriterEntries entries = itr->getDiskWriterEntries();

  sort(entries.begin(), entries.end());

  CPPUNIT_ASSERT_EQUAL((size_t)6, entries.size());

  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file1"), entries[0]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file2"), entries[1]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file3"), entries[2]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file6"), entries[3]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file7"), entries[4]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+string("/file8"), entries[5]->getFilePath(storeDir));
}

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
