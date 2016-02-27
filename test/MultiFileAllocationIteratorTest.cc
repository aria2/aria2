#include "MultiFileAllocationIterator.h"

#include <algorithm>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "File.h"
#include "MultiDiskAdaptor.h"
#include "FileEntry.h"
#include "Exception.h"
#include "array_fun.h"
#include "TestUtil.h"
#include "DiskWriter.h"

namespace aria2 {

class MultiFileAllocationIteratorTest : public CppUnit::TestFixture {

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

CPPUNIT_TEST_SUITE_REGISTRATION(MultiFileAllocationIteratorTest);

void MultiFileAllocationIteratorTest::testMakeDiskWriterEntries()
{
  std::string storeDir =
      A2_TEST_OUT_DIR "/aria2_MultiFileAllocationIteratorTest"
                      "_testMakeDiskWriterEntries";

  auto fs = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(storeDir + "/file1", 1536, 0),
      std::make_shared<FileEntry>(storeDir + "/file2", 2048, 1536), // req no
      std::make_shared<FileEntry>(storeDir + "/file3", 1024, 3584),
      std::make_shared<FileEntry>(storeDir + "/file4", 1024, 4608), // req no
      std::make_shared<FileEntry>(storeDir + "/file5", 1024, 5632), // req no
      std::make_shared<FileEntry>(storeDir + "/file6", 1024, 6656), // req no
      std::make_shared<FileEntry>(storeDir + "/file7", 256, 7680),  // req no
      std::make_shared<FileEntry>(storeDir + "/file8", 255, 7936),
      std::make_shared<FileEntry>(storeDir + "/file9", 1025, 8191), // req no
      std::make_shared<FileEntry>(storeDir + "/fileA", 1024, 9216), // req no
      std::make_shared<FileEntry>(storeDir + "/fileB", 1024, 10240)};
  fs[1]->setRequested(false); // file2
  fs[3]->setRequested(false); // file4
  fs[4]->setRequested(false); // file5
  fs[5]->setRequested(false); // file6
  fs[6]->setRequested(false); // file7
  fs[8]->setRequested(false); // file9
  fs[9]->setRequested(false); // fileA

  for (auto& fe : fs) {
    File{fe->getPath()}.remove();
  }

  // create empty file4
  createFile(storeDir + std::string("/file4"), 0);

  MultiDiskAdaptor diskAdaptor;
  diskAdaptor.setFileEntries(std::begin(fs), std::end(fs));
  diskAdaptor.setPieceLength(1_k);
  diskAdaptor.openFile();

  auto allocitr = diskAdaptor.fileAllocationIterator();
  auto itr = dynamic_cast<MultiFileAllocationIterator*>(allocitr.get());

  auto& entries = itr->getDiskWriterEntries();
  CPPUNIT_ASSERT_EQUAL((size_t)11, entries.size());

  // file1
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file1"),
                       entries[0]->getFilePath());
  CPPUNIT_ASSERT(entries[0]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[0]->getDiskWriter());
  // file2
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file2"),
                       entries[1]->getFilePath());
  CPPUNIT_ASSERT(entries[1]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[1]->getDiskWriter());
  // file3
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file3"),
                       entries[2]->getFilePath());
  CPPUNIT_ASSERT(entries[2]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[2]->getDiskWriter());
  // file4, diskWriter is not null, because file exists.
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file4"),
                       entries[3]->getFilePath());
  CPPUNIT_ASSERT(!entries[3]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[3]->getDiskWriter());
  // file5
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file5"),
                       entries[4]->getFilePath());
  CPPUNIT_ASSERT(!entries[4]->needsFileAllocation());
  CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
  // file6
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file6"),
                       entries[5]->getFilePath());
  CPPUNIT_ASSERT(entries[5]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[5]->getDiskWriter());
  // file7
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file7"),
                       entries[6]->getFilePath());
  CPPUNIT_ASSERT(entries[6]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[6]->getDiskWriter());
  // file8
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file8"),
                       entries[7]->getFilePath());
  CPPUNIT_ASSERT(entries[7]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[7]->getDiskWriter());
  // file9
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/file9"),
                       entries[8]->getFilePath());
  CPPUNIT_ASSERT(!entries[8]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[8]->getDiskWriter());
  // fileA
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/fileA"),
                       entries[9]->getFilePath());
  CPPUNIT_ASSERT(!entries[9]->needsFileAllocation());
  CPPUNIT_ASSERT(!entries[9]->getDiskWriter());
  // fileB
  CPPUNIT_ASSERT_EQUAL(storeDir + std::string("/fileB"),
                       entries[10]->getFilePath());
  CPPUNIT_ASSERT(entries[10]->needsFileAllocation());
  CPPUNIT_ASSERT(entries[10]->getDiskWriter());
}

void MultiFileAllocationIteratorTest::testAllocate()
{
  std::string storeDir =
      A2_TEST_OUT_DIR "/aria2_MultiFileAllocationIteratorTest_testAllocate";

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
    MultiDiskAdaptor diskAdaptor;
    diskAdaptor.setPieceLength(1);

    int64_t offset = 0;
    auto fileEntry1 =
        std::make_shared<FileEntry>(storeDir + "/" + fname1, length1, offset);
    offset += length1;
    auto fileEntry2 =
        std::make_shared<FileEntry>(storeDir + "/" + fname2, length2, offset);
    offset += length2;
    auto fileEntry3 =
        std::make_shared<FileEntry>(storeDir + "/" + fname3, length3, offset);

    offset += length3;
    auto fileEntry4 =
        std::make_shared<FileEntry>(storeDir + "/" + fname4, length4, offset);
    fileEntry4->setRequested(false);
    offset += length4;
    auto fileEntry5 =
        std::make_shared<FileEntry>(storeDir + "/" + fname5, length5, offset);
    offset += length5;
    auto fileEntry6 =
        std::make_shared<FileEntry>(storeDir + "/" + fname6, length6, offset);
    fileEntry6->setRequested(false);

    auto fs = std::vector<std::shared_ptr<FileEntry>>{
        fileEntry1, fileEntry2, fileEntry3, fileEntry4, fileEntry5, fileEntry6};
    diskAdaptor.setFileEntries(std::begin(fs), std::end(fs));

    for (auto& fe : fs) {
      File{fe->getPath()}.remove();
    }

    // we have to open file first.
    diskAdaptor.initAndOpenFile();
    auto allocitr = diskAdaptor.fileAllocationIterator();
    auto itr = dynamic_cast<MultiFileAllocationIterator*>(allocitr.get());
    while (!itr->finished()) {
      itr->allocateChunk();
    }
    CPPUNIT_ASSERT_EQUAL((int64_t)length1, File(fileEntry1->getPath()).size());
    CPPUNIT_ASSERT_EQUAL((int64_t)length2, File(fileEntry2->getPath()).size());
    CPPUNIT_ASSERT_EQUAL((int64_t)length3, File(fileEntry3->getPath()).size());
    CPPUNIT_ASSERT(!File(fileEntry4->getPath()).isFile());
    CPPUNIT_ASSERT_EQUAL((int64_t)length5, File(fileEntry5->getPath()).size());
    CPPUNIT_ASSERT(!File(fileEntry6->getPath()).isFile());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

} // namespace aria2
