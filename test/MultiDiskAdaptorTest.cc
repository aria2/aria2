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

namespace aria2 {

class MultiDiskAdaptorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiDiskAdaptorTest);
  CPPUNIT_TEST(testWriteData);
  CPPUNIT_TEST(testReadData);
  CPPUNIT_TEST(testCutTrailingGarbage);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testUtime);
  CPPUNIT_TEST(testResetDiskWriterEntries);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MultiDiskAdaptor> adaptor;
public:
  void setUp() {
    adaptor.reset(new MultiDiskAdaptor());
    adaptor->setPieceLength(2);
  }

  void testWriteData();
  void testReadData();
  void testCutTrailingGarbage();
  void testSize();
  void testUtime();
  void testResetDiskWriterEntries();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiDiskAdaptorTest );

std::vector<SharedHandle<FileEntry> > createEntries() {
  SharedHandle<FileEntry> array[] = {
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file0.txt", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file1.txt", 15, 0)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file2.txt", 7, 15)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file3.txt", 0, 22)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file4.txt", 2, 22)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file5.txt", 0, 24)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file6.txt", 3, 24)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file7.txt", 0, 27)),
    SharedHandle<FileEntry>(new FileEntry(A2_TEST_OUT_DIR"/file8.txt", 2, 27)),
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
  std::vector<SharedHandle<FileEntry> > entries(vbegin(array), vend(array));
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i = entries.begin();
      i != entries.end(); ++i) {
    File((*i)->getPath()).remove();
  }
  return entries;
}

void MultiDiskAdaptorTest::testResetDiskWriterEntries()
{
  {
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    fileEntries[1]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    for(size_t i = 5; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    for(size_t i = 1; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter());

    adaptor->closeFile();
  }
  {
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    for(size_t i = 2; i < 9; ++i) {
      fileEntries[i]->setRequested(false);
    }
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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
    std::vector<SharedHandle<FileEntry> > fileEntries = createEntries();
    for(size_t i = 0; i < 6; ++i) {
      fileEntries[i]->setRequested(false);
    }
    fileEntries[8]->setRequested(false);
    adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());
    adaptor->openFile();
    std::vector<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
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

void readFile(const std::string& filename, char* buf, int bufLength) {
  FILE* f = fopen(filename.c_str(), "r");
  if(f == NULL) {
    CPPUNIT_FAIL(strerror(errno));
  }
  int retval = fread(buf, bufLength, 1, f);
  fclose(f);
  if(retval != 1) {
    CPPUNIT_FAIL("return value is not 1");
  }
}

void MultiDiskAdaptorTest::testWriteData() {
  std::vector<SharedHandle<FileEntry> > fileEntries(createEntries());
  adaptor->setFileEntries(fileEntries.begin(), fileEntries.end());

  adaptor->openFile();
  std::string msg = "12345";
  adaptor->writeData((const unsigned char*)msg.c_str(), msg.size(), 0);
  adaptor->closeFile();

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR"/file0.txt").isFile());
  char buf[128];
  readFile(A2_TEST_OUT_DIR"/file1.txt", buf, 5);
  buf[5] = '\0';
  CPPUNIT_ASSERT_EQUAL(msg, std::string(buf));

  adaptor->openFile();
  std::string msg2 = "67890ABCDEF";
  adaptor->writeData((const unsigned char*)msg2.c_str(), msg2.size(), 5);
  adaptor->closeFile();

  readFile(A2_TEST_OUT_DIR"/file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDE"), std::string(buf));
  readFile(A2_TEST_OUT_DIR"/file2.txt", buf, 1);
  buf[1] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("F"), std::string(buf));

  adaptor->openFile();
  std::string msg3 = "12345123456712";
  adaptor->writeData((const unsigned char*)msg3.c_str(), msg3.size(), 10);
  adaptor->closeFile();

  readFile(A2_TEST_OUT_DIR"/file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("123456789012345"), std::string(buf));
  readFile(A2_TEST_OUT_DIR"/file2.txt", buf, 7);
  buf[7] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567"), std::string(buf));

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR"/file3.txt").isFile());

  readFile(A2_TEST_OUT_DIR"/file4.txt", buf, 2);
  buf[2] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("12"), std::string(buf));

  CPPUNIT_ASSERT(File(A2_TEST_OUT_DIR"/file5.txt").isFile());
}

void MultiDiskAdaptorTest::testReadData() {
  SharedHandle<FileEntry> entry1(new FileEntry(A2_TEST_DIR"/file1r.txt", 15, 0));
  SharedHandle<FileEntry> entry2(new FileEntry(A2_TEST_DIR"/file2r.txt", 7, 15));
  SharedHandle<FileEntry> entry3(new FileEntry(A2_TEST_DIR"/file3r.txt", 3, 22));
  std::vector<SharedHandle<FileEntry> > entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);

  adaptor->setFileEntries(entries.begin(), entries.end());
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
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDEFGHIJKLMNO"), std::string((char*)buf));
}

void MultiDiskAdaptorTest::testCutTrailingGarbage()
{
  std::string dir = A2_TEST_OUT_DIR;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testCutTrailingGarbage_";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(dir+"/"+prefix+"1", 256, 0)),
    SharedHandle<FileEntry>(new FileEntry(dir+"/"+prefix+"2", 512, 256))
  };
  for(size_t i = 0; i < A2_ARRAY_LEN(entries); ++i) {
    createFile(entries[i]->getPath(), entries[i]->getLength()+100);
  }
  std::vector<SharedHandle<FileEntry> > fileEntries
    (vbegin(entries), vend(entries));
  
  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(fileEntries.begin(), fileEntries.end());
  adaptor.setMaxOpenFiles(1);
  adaptor.setPieceLength(1);
  
  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((int64_t)256,
                       File(entries[0]->getPath()).size());
  CPPUNIT_ASSERT_EQUAL((int64_t)512,
                       File(entries[1]->getPath()).size());
}

void MultiDiskAdaptorTest::testSize()
{
  std::string dir = A2_TEST_OUT_DIR;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testSize_";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(dir+"/"+prefix+"1", 1, 0)),
    SharedHandle<FileEntry>(new FileEntry(dir+"/"+prefix+"2", 1, 1))
  };
  for(size_t i = 0; i < A2_ARRAY_LEN(entries); ++i) {
    createFile(entries[i]->getPath(), entries[i]->getLength());
  }
  std::vector<SharedHandle<FileEntry> > fileEntries
    (vbegin(entries), vend(entries));
  
  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(fileEntries.begin(), fileEntries.end());
  adaptor.setMaxOpenFiles(1);
  adaptor.setPieceLength(1);
  
  adaptor.openFile();

  CPPUNIT_ASSERT_EQUAL((int64_t)2, adaptor.size());
}

void MultiDiskAdaptorTest::testUtime()
{
  std::string storeDir = A2_TEST_OUT_DIR"/aria2_MultiDiskAdaptorTest_testUtime";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/requested", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/notFound", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/notRequested", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/anotherRequested", 0, 0)),
  };

  createFile(entries[0]->getPath(), entries[0]->getLength());
  File(entries[1]->getPath()).remove();
  createFile(entries[2]->getPath(), entries[2]->getLength());
  createFile(entries[3]->getPath(), entries[3]->getLength());

  entries[2]->setRequested(false);

  std::vector<SharedHandle<FileEntry> > fileEntries
    (vbegin(entries), vend(entries));
  MultiDiskAdaptor adaptor;
  adaptor.setFileEntries(fileEntries.begin(), fileEntries.end());

  time_t atime = (time_t) 100000;
  time_t mtime = (time_t) 200000;
  
  CPPUNIT_ASSERT_EQUAL((size_t)2, adaptor.utime(Time(atime), Time(mtime)));
  
  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
                       File(entries[0]->getPath()).getModifiedTime().getTime());

  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
                       File(entries[3]->getPath()).getModifiedTime().getTime());

  CPPUNIT_ASSERT((time_t)mtime !=
                 File(entries[2]->getPath()).getModifiedTime().getTime());
}

} // namespace aria2
