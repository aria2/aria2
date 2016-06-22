#include "MultiDiskAdaptor.h"

#include <string>
#include <cerrno>
#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "FileEntry.h"
#include "Exception.h"
#include "a2io.h"
#include "array_fun.h"
#include "TestUtil.h"
#include "DiskWriter.h"
#include "WrDiskCacheEntry.h"

namespace aria2 {

class MultiDiskAdaptorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiDiskAdaptorTest);
  CPPUNIT_TEST(testWriteData);
  CPPUNIT_TEST(testReadData);
  CPPUNIT_TEST(testCutTrailingGarbage);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testUtime);
  CPPUNIT_TEST(testResetDiskWriterEntries);
  CPPUNIT_TEST(testWriteCache);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<MultiDiskAdaptor> adaptor;

public:
  void setUp()
  {
    adaptor = make_unique<MultiDiskAdaptor>();
    adaptor->setPieceLength(2);
  }

  void testWriteData();
  void testReadData();
  void testCutTrailingGarbage();
  void testSize();
  void testUtime();
  void testResetDiskWriterEntries();
  void testWriteCache();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MultiDiskAdaptorTest);

std::vector<std::shared_ptr<FileEntry>> createEntries()
{
  std::vector<std::shared_ptr<FileEntry>> entries{
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file0.txt", 0, 0),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file1.txt", 15, 0),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file2.txt", 7, 15),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file3.txt", 0, 22),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file4.txt", 2, 22),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file5.txt", 0, 24),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file6.txt", 3, 24),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file7.txt", 0, 27),
      std::make_shared<FileEntry>(A2_TEST_OUT_DIR "/file8.txt", 2, 27),
  };
  //           1    1    2    2    3
  // 0....5....0....5....0....5....0
  // ++--++--++--++--++--++--++--++--
  // | file0
  // *************** file1
  //                ******* file2
  //                       | file3
  //                       ** flie4
  //                         | file5
  //                         *** file6
  //                            |file7
  //                            ** file8
  for (const auto& i : entries) {
    File(i->getPath()).remove();
  }
  return entries;
}

void MultiDiskAdaptorTest::testResetDiskWriterEntries()
{
  {
    auto fileEntries = createEntries();
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    // Because entries[1] spans entries[0]
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    fileEntries[1]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter());
    // Because entries[2] spans entries[1]
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->needsFileAllocation());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    // Because entries[4] spans entries[3]
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->needsFileAllocation());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    // entries[3] is 0 length. No overrap with entries[4]
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    for (size_t i = 5; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();

    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    for (size_t i = 1; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    for (size_t i = 2; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    // entries[1] spans entries[2]
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[2]->needsFileAllocation());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    auto fileEntries = createEntries();
    for (size_t i = 0; i < 6; ++i) {
      fileEntries[i]->setRequested(false);
    }
    fileEntries[8]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    auto& entries = adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    // entries[6] spans entries[5] in the current implementation.
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());
    CPPUNIT_ASSERT(entries[6]->getDiskWriter());
    CPPUNIT_ASSERT(entries[7]->getDiskWriter());
    // entries[6] spans entries[8]
    CPPUNIT_ASSERT(entries[8]->getDiskWriter());
    adaptor->closeFile();
  }
}

namespace {
void readFile(const std::string& filename, char* buf, int bufLength)
{
  FILE* f = fopen(filename.c_str(), "r");
  if (f == nullptr) {
    CPPUNIT_FAIL(strerror(errno));
  }
  int retval = fread(buf, 1, bufLength, f);
  fclose(f);
  if (retval != bufLength) {
    CPPUNIT_FAIL("return value is not 1");
  }
}
} // namespace

void MultiDiskAdaptorTest::testWriteData()
{
  auto fileEntries = createEntries();
  adaptor->setFileEntries(std::begin(fileEntries), std::end(fileEntries));

  adaptor->openFile();
  std::string msg = "12345";
  adaptor->writeData((const unsigned char*)msg.c_str(), msg.size(), 0);
  adaptor->closeFile();

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR "/file0.txt").isFile());
  char buf[128];
  readFile(A2_TEST_OUT_DIR "/file1.txt", buf, 5);
  buf[5] = '\0';
  CPPUNIT_ASSERT_EQUAL(msg, std::string(buf));

  adaptor->openFile();
  std::string msg2 = "67890ABCDEF";
  adaptor->writeData((const unsigned char*)msg2.c_str(), msg2.size(), 5);
  adaptor->closeFile();

  readFile(A2_TEST_OUT_DIR "/file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDE"), std::string(buf));
  readFile(A2_TEST_OUT_DIR "/file2.txt", buf, 1);
  buf[1] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("F"), std::string(buf));

  adaptor->openFile();
  std::string msg3 = "12345123456712";
  adaptor->writeData((const unsigned char*)msg3.c_str(), msg3.size(), 10);
  adaptor->closeFile();

  readFile(A2_TEST_OUT_DIR "/file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("123456789012345"), std::string(buf));
  readFile(A2_TEST_OUT_DIR "/file2.txt", buf, 7);
  buf[7] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567"), std::string(buf));

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR "/file3.txt").isFile());

  readFile(A2_TEST_OUT_DIR "/file4.txt", buf, 2);
  buf[2] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("12"), std::string(buf));

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR "/file5.txt").isFile());
}

void MultiDiskAdaptorTest::testReadData()
{
  auto entries = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(A2_TEST_DIR "/file1r.txt", 15, 0),
      std::make_shared<FileEntry>(A2_TEST_DIR "/file2r.txt", 7, 15),
      std::make_shared<FileEntry>(A2_TEST_DIR "/file3r.txt", 3, 22)};

  adaptor->setFileEntries(std::begin(entries), std::end(entries));
  adaptor->enableReadOnly();
  adaptor->openFile();
  unsigned char buf[128];
  adaptor->readData(buf, 15, 0);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDE"), std::string((char*)buf));
  adaptor->readData(buf, 10, 6);
  buf[10] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("7890ABCDEF"), std::string((char*)buf));
  adaptor->readData(buf, 4, 20);
  buf[4] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("KLMN"), std::string((char*)buf));
  adaptor->readData(buf, 25, 0);
  buf[25] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDEFGHIJKLMNO"),
                       std::string((char*)buf));
}

void MultiDiskAdaptorTest::testCutTrailingGarbage()
{
  std::string dir = A2_TEST_OUT_DIR;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testCutTrailingGarbage_";
  auto fileEntries = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(dir + "/" + prefix + "1", 256, 0),
      std::make_shared<FileEntry>(dir + "/" + prefix + "2", 512, 256)};
  for (const auto& i : fileEntries) {
    createFile(i->getPath(), i->getLength() + 100);
  }

  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(std::begin(fileEntries), std::end(fileEntries));
  adaptor.setPieceLength(1);

  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((int64_t)256, File(fileEntries[0]->getPath()).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)512, File(fileEntries[1]->getPath()).size());
}

void MultiDiskAdaptorTest::testSize()
{
  std::string dir = A2_TEST_OUT_DIR;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testSize_";
  auto fileEntries = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(dir + "/" + prefix + "1", 1, 0),
      std::make_shared<FileEntry>(dir + "/" + prefix + "2", 1, 1)};
  for (const auto& i : fileEntries) {
    createFile(i->getPath(), i->getLength());
  }

  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(std::begin(fileEntries), std::end(fileEntries));
  adaptor.setPieceLength(1);

  adaptor.openFile();

  CPPUNIT_ASSERT_EQUAL((int64_t)2, adaptor.size());
}

void MultiDiskAdaptorTest::testUtime()
{
  std::string storeDir =
      A2_TEST_OUT_DIR "/aria2_MultiDiskAdaptorTest_testUtime";
  auto entries = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(storeDir + "/requested", 0, 0),
      std::make_shared<FileEntry>(storeDir + "/notFound", 0, 0),
      std::make_shared<FileEntry>(storeDir + "/notRequested", 0, 0),
      std::make_shared<FileEntry>(storeDir + "/anotherRequested", 0, 0),
  };

  createFile(entries[0]->getPath(), entries[0]->getLength());
  File(entries[1]->getPath()).remove();
  createFile(entries[2]->getPath(), entries[2]->getLength());
  createFile(entries[3]->getPath(), entries[3]->getLength());

  entries[2]->setRequested(false);

  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(std::begin(entries), std::end(entries));

  time_t atime = (time_t)100000;
  time_t mtime = (time_t)200000;

  CPPUNIT_ASSERT_EQUAL((size_t)2, adaptor.utime(Time(atime), Time(mtime)));

  CPPUNIT_ASSERT_EQUAL(
      (time_t)mtime,
      File(entries[0]->getPath()).getModifiedTime().getTimeFromEpoch());

  CPPUNIT_ASSERT_EQUAL(
      (time_t)mtime,
      File(entries[3]->getPath()).getModifiedTime().getTimeFromEpoch());

  CPPUNIT_ASSERT(
      (time_t)mtime !=
      File(entries[2]->getPath()).getModifiedTime().getTimeFromEpoch());
}

void MultiDiskAdaptorTest::testWriteCache()
{
  std::string storeDir =
      A2_TEST_OUT_DIR "/aria2_MultiDiskAdaptorTest_testWriteCache";
  auto entries = std::vector<std::shared_ptr<FileEntry>>{
      std::make_shared<FileEntry>(storeDir + "/file1", 16385, 0),
      std::make_shared<FileEntry>(storeDir + "/file2", 4098, 16385)};
  for (const auto& i : entries) {
    File(i->getPath()).remove();
  }
  auto adaptor = std::make_shared<MultiDiskAdaptor>();
  adaptor->setFileEntries(std::begin(entries), std::end(entries));
  WrDiskCacheEntry cache{adaptor};
  std::string data1(16383, '1'), data2(100, '2'), data3(4000, '3');
  cache.cacheData(createDataCell(0, data1.c_str()));
  cache.cacheData(createDataCell(data1.size(), data2.c_str()));
  cache.cacheData(createDataCell(data1.size() + data2.size(), data3.c_str()));
  adaptor->openFile();
  adaptor->writeCache(&cache);
  for (int i = 0; i < 2; ++i) {
    CPPUNIT_ASSERT_EQUAL(entries[i]->getLength(),
                         File(entries[i]->getPath()).size());
  }
  CPPUNIT_ASSERT_EQUAL(data1 + data2.substr(0, 2),
                       readFile(entries[0]->getPath()));
  CPPUNIT_ASSERT_EQUAL(data2.substr(2) + data3,
                       readFile(entries[1]->getPath()));

  adaptor->closeFile();
  for (int i = 0; i < 2; ++i) {
    File(entries[i]->getPath()).remove();
  }
  cache.clear();
  cache.cacheData(createDataCell(123, data2.c_str()));
  adaptor->openFile();
  adaptor->writeCache(&cache);
  CPPUNIT_ASSERT_EQUAL((int64_t)(123 + data2.size()),
                       File(entries[0]->getPath()).size());
  CPPUNIT_ASSERT_EQUAL(data2, readFile(entries[0]->getPath()).substr(123));
}

} // namespace aria2
