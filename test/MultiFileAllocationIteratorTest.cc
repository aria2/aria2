#include "MultiFileAllocationIterator.h"
#include "File.h"
#include "MultiDiskAdaptor.h"
#include "FileEntry.h"
#include "Exception.h"
#include <algorithm>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  SharedHandle<FileEntry> fs[] = {
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
  
  std::string storeDir = "/tmp/aria2_MultiFileAllocationIteratorTest_testMakeDiskWriterEntries";
  SharedHandle<MultiDiskAdaptor> diskAdaptor = new MultiDiskAdaptor();
  diskAdaptor->setFileEntries(std::deque<SharedHandle<FileEntry> >(&fs[0], &fs[10]));
  diskAdaptor->setPieceLength(1024);
  diskAdaptor->setStoreDir(storeDir);
  diskAdaptor->openFile();

  SharedHandle<MultiFileAllocationIterator> itr = diskAdaptor->fileAllocationIterator();

  DiskWriterEntries entries = itr->getDiskWriterEntries();

  std::sort(entries.begin(), entries.end());

  CPPUNIT_ASSERT_EQUAL((size_t)6, entries.size());

  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file1"), entries[0]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file2"), entries[1]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file3"), entries[2]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file6"), entries[3]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file7"), entries[4]->getFilePath(storeDir));
  CPPUNIT_ASSERT_EQUAL(storeDir+std::string("/file8"), entries[5]->getFilePath(storeDir));
}

void MultiFileAllocationIteratorTest::testAllocate()
{
  std::string dir = "/tmp";
  std::string topDir = "aria2_MultiFileAllocationIteratorTest_testAllocate";
  std::string fname1 = "file1";
  std::string fname2 = "file2";
  std::string fname3 = "file3";
  std::string fname4 = "file4";
  std::string fname5 = "file5";
  std::string fname6 = "file6";
  int64_t length1 = 32769;
  int64_t length2 = 0;
  int64_t length3 = 8;
  int64_t length4 = 10;
  int64_t length5 = 20;
  int64_t length6 = 30;

  try {
    SharedHandle<MultiDiskAdaptor> diskAdaptor = new MultiDiskAdaptor();
    diskAdaptor->setStoreDir(dir);
    diskAdaptor->setTopDir(topDir);

    int64_t offset = 0;
    SharedHandle<FileEntry> fileEntry1 = new FileEntry(fname1,
						       length1,
						       offset);
    offset += length1;
    SharedHandle<FileEntry> fileEntry2 = new FileEntry(fname2,
						       length2,
						       offset);

    offset += length2;
    SharedHandle<FileEntry> fileEntry3 = new FileEntry(fname3,
						       length3,
						       offset);

    offset += length3;
    SharedHandle<FileEntry> fileEntry4 = new FileEntry(fname4,
						       length4,
						       offset);
    fileEntry4->setRequested(false);

    offset += length4;
    SharedHandle<FileEntry> fileEntry5 = new FileEntry(fname5,
						       length5,
						       offset);

    offset += length5;
    SharedHandle<FileEntry> fileEntry6 = new FileEntry(fname6,
						       length6,
						       offset);
    fileEntry6->setRequested(false);

    std::deque<SharedHandle<FileEntry> > fs;
    fs.push_back(fileEntry1);
    fs.push_back(fileEntry2);
    fs.push_back(fileEntry3);
    fs.push_back(fileEntry4);
    fs.push_back(fileEntry5);
    fs.push_back(fileEntry6);
    diskAdaptor->setFileEntries(fs);

    // we have to open file first.
    diskAdaptor->initAndOpenFile();
    SharedHandle<MultiFileAllocationIterator> itr = diskAdaptor->fileAllocationIterator();
    while(!itr->finished()) {
      itr->allocateChunk();
    }
    CPPUNIT_ASSERT_EQUAL((uint64_t)length1, File(dir+"/"+topDir+"/"+fname1).size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)length2, File(dir+"/"+topDir+"/"+fname2).size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)length3, File(dir+"/"+topDir+"/"+fname3).size());
    CPPUNIT_ASSERT_EQUAL(0ULL, File(dir+"/"+topDir+"/"+fname4).size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)length5, File(dir+"/"+topDir+"/"+fname5).size());
    CPPUNIT_ASSERT_EQUAL(0ULL, File(dir+"/"+topDir+"/"+fname6).size());

  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    std::string m = e->getMsg();
    delete e;
    CPPUNIT_FAIL(m);
  }
}

} // namespace aria2
