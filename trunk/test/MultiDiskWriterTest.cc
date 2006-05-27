#include "MultiDiskWriter.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class MultiDiskWriterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MultiDiskWriterTest);
  CPPUNIT_TEST(testWriteData);
  CPPUNIT_TEST(testReadData);
  CPPUNIT_TEST(testSha1Sum);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testWriteData();
  void testReadData();
  void testSha1Sum();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiDiskWriterTest );

FileEntries createEntries() {
  FileEntry entry1("file1.txt", 15, 0);
  FileEntry entry2("file2.txt", 7, 15);
  FileEntry entry3("file3.txt", 3, 22);
  unlink("file1.txt");
  unlink("file2.txt");
  unlink("file3.txt");
  FileEntries entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);
  return entries;
}

void readFile(const string& filename, char* buf, int bufLength) {
  FILE* f = fopen(filename.c_str(), "r");
  if(f == NULL) {
    abort();
  }
  int retval = fread(buf, bufLength, 1, f);
  fclose(f);
  if(retval != 1) {
    abort();
  }
}

void MultiDiskWriterTest::testWriteData() {
  MultiDiskWriter dw(2);
  dw.setFileEntries(createEntries());

  dw.openFile(".");
  string msg = "12345";
  dw.writeData(msg.c_str(), msg.size(), 0);
  dw.closeFile();

  char buf[128];
  readFile("file1.txt", buf, 5);
  buf[5] = '\0';
  CPPUNIT_ASSERT_EQUAL(msg, string(buf));

  dw.openFile(".");
  string msg2 = "67890ABCDEF";
  dw.writeData(msg2.c_str(), msg2.size(), 5);
  dw.closeFile();

  readFile("file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("1234567890ABCDE"), string(buf));
  readFile("file2.txt", buf, 1);
  buf[1] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("F"), string(buf));

  dw.openFile(".");
  string msg3 = "12345123456712";
  dw.writeData(msg3.c_str(), msg3.size(), 10);
  dw.closeFile();

  readFile("file1.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("123456789012345"), string(buf));
  readFile("file2.txt", buf, 7);
  buf[7] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("1234567"), string(buf));
  readFile("file3.txt", buf, 2);
  buf[2] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("12"), string(buf));
}

void MultiDiskWriterTest::testReadData() {
  FileEntry entry1("file1r.txt", 15, 0);
  FileEntry entry2("file2r.txt", 7, 15);
  FileEntry entry3("file3r.txt", 3, 22);
  FileEntries entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);
  MultiDiskWriter dw(2);
  dw.setFileEntries(entries);

  dw.openFile(".");
  char buf[128];
  dw.readData(buf, 15, 0);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("1234567890ABCDE"), string(buf));
  dw.readData(buf, 10, 6);
  buf[10] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("7890ABCDEF"), string(buf));
  dw.readData(buf, 4, 20);
  buf[4] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("KLMN"), string(buf));
  dw.readData(buf, 25, 0);
  buf[25] = '\0';
  CPPUNIT_ASSERT_EQUAL(string("1234567890ABCDEFGHIJKLMNO"), string(buf));
}

void MultiDiskWriterTest::testSha1Sum() {
  FileEntry entry1("file1r.txt", 15, 0);
  FileEntry entry2("file2r.txt", 7, 15);
  FileEntry entry3("file3r.txt", 3, 22);
  FileEntries entries;
  entries.push_back(entry1);
  entries.push_back(entry2);
  entries.push_back(entry3);
  MultiDiskWriter dw(2);
  dw.setFileEntries(entries);

  dw.openFile(".");
  string sha1sum = dw.sha1Sum(0, 25);
  CPPUNIT_ASSERT_EQUAL(string("76495faf71ca63df66dce99547d2c58da7266d9e"), sha1sum);
  sha1sum = dw.sha1Sum(15, 7);
  CPPUNIT_ASSERT_EQUAL(string("737660d816fb23c2d5bc74f62d9b01b852b2aaca"), sha1sum);
  sha1sum = dw.sha1Sum(10, 14);
  CPPUNIT_ASSERT_EQUAL(string("6238bf61dd8df8f77156b2378e9e39cd3939680c"), sha1sum);
  dw.closeFile();
}
