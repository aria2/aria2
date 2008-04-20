#include "MultiDiskAdaptor.h"
#include "FileEntry.h"
#include "Exception.h"
#include <string>
#include <cerrno>
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MultiDiskAdaptorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiDiskAdaptorTest);
  CPPUNIT_TEST(testWriteData);
  CPPUNIT_TEST(testReadData);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MultiDiskAdaptor> adaptor;
public:
  void setUp() {
    adaptor.reset(new MultiDiskAdaptor());
    adaptor->setPieceLength(2);
    adaptor->setStoreDir(".");
    adaptor->setTopDir(".");
  }

  void testWriteData();
  void testReadData();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiDiskAdaptorTest );

std::deque<SharedHandle<FileEntry> > createEntries() {
  SharedHandle<FileEntry> entry1(new FileEntry("file1.txt", 15, 0));
  SharedHandle<FileEntry> entry2(new FileEntry("file2.txt", 7, 15));
  SharedHandle<FileEntry> entry3(new FileEntry("file3.txt", 3, 22));
  unlink("file1.txt");
  unlink("file2.txt");
  unlink("file3.txt");
  std::deque<SharedHandle<FileEntry> > entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);
  return entries;
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
  try {
  adaptor->setFileEntries(createEntries());

  adaptor->openFile();
  std::string msg = "12345";
  adaptor->writeData((const unsigned char*)msg.c_str(), msg.size(), 0);
  adaptor->closeFile();

  char buf[128];
  readFile("file1.txt", buf, 5);
  buf[5] = '\0';
  CPPUNIT_ASSERT_EQUAL(msg, std::string(buf));

  adaptor->openFile();
  std::string msg2 = "67890ABCDEF";
  adaptor->writeData((const unsigned char*)msg2.c_str(), msg2.size(), 5);
  adaptor->closeFile();

  readFile("file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDE"), std::string(buf));
  readFile("file2.txt", buf, 1);
  buf[1] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("F"), std::string(buf));

  adaptor->openFile();
  std::string msg3 = "12345123456712";
  adaptor->writeData((const unsigned char*)msg3.c_str(), msg3.size(), 10);
  adaptor->closeFile();

  readFile("file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("123456789012345"), std::string(buf));
  readFile("file2.txt", buf, 7);
  buf[7] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567"), std::string(buf));
  readFile("file3.txt", buf, 2);
  buf[2] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("12"), std::string(buf));
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
  }
}

void MultiDiskAdaptorTest::testReadData() {
  SharedHandle<FileEntry> entry1(new FileEntry("file1r.txt", 15, 0));
  SharedHandle<FileEntry> entry2(new FileEntry("file2r.txt", 7, 15));
  SharedHandle<FileEntry> entry3(new FileEntry("file3r.txt", 3, 22));
  std::deque<SharedHandle<FileEntry> > entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);

  adaptor->setFileEntries(entries);

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

} // namespace aria2
