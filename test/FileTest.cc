#include "File.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class FileTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FileTest);
  CPPUNIT_TEST(testExists);
  CPPUNIT_TEST(testIsFile);
  CPPUNIT_TEST(testIsDir);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testSize);
  CPPUNIT_TEST(testMkdir);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testExists();
  void testIsFile();
  void testIsDir();
  void testRemove();
  void testSize();
  void testMkdir();
};


CPPUNIT_TEST_SUITE_REGISTRATION( FileTest );

void FileTest::testExists() {
  File f("FileTest.cc");
  CPPUNIT_ASSERT(f.exists());

  File f2("NonExistentFile");
  CPPUNIT_ASSERT(!f2.exists());

  File d1("../test");
  CPPUNIT_ASSERT(d1.exists());
}

void FileTest::testIsFile() {
  File f("FileTest.cc");
  CPPUNIT_ASSERT(f.isFile());

  File f2("NonExistentFile");
  CPPUNIT_ASSERT(!f2.isFile());

  File d1("../test");
  CPPUNIT_ASSERT(!d1.isFile());
}

void FileTest::testIsDir() {
  File f("FileTest.cc");
  CPPUNIT_ASSERT(!f.isDir());

  File f2("NonExistentFile");
  CPPUNIT_ASSERT(!f2.isDir());

  File d1("../test");
  CPPUNIT_ASSERT(d1.isDir());
}

void FileTest::testRemove() {
  int fd;
  string name = "/tmp/aria2test";
  unlink(name.c_str());
  if((fd = creat(name.c_str(), S_IRUSR|S_IWUSR)) < 0) {
    CPPUNIT_FAIL("cannot create test file");
  }
  close(fd);
  File f(name);
  CPPUNIT_ASSERT(f.isFile());
  CPPUNIT_ASSERT(f.remove());
  CPPUNIT_ASSERT(!f.exists());
  // delete the file again
  CPPUNIT_ASSERT(!f.remove());

  string dir = "/tmp/aria2testdir";
  mkdir(dir.c_str(), 0777);
  File d(dir);
  CPPUNIT_ASSERT(d.exists());
  CPPUNIT_ASSERT(d.remove());
  CPPUNIT_ASSERT(!d.exists());
  // delete the directory again
  CPPUNIT_ASSERT(!d.remove());
}

void FileTest::testSize() {
  File f("4096chunk.txt");
  CPPUNIT_ASSERT_EQUAL(4096, (int)f.size());
}

void FileTest::testMkdir() {
  string dir = "/tmp/aria2test2/test";
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());

  CPPUNIT_ASSERT(d.mkdirs());

  CPPUNIT_ASSERT(d.exists());
  // this test failes because d.mkdir returns false when the directory is
  // already exists.
  CPPUNIT_ASSERT(!d.mkdirs());
}
