#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

class FileEntryTest : public CppUnit::TestFixture {
  
  CPPUNIT_TEST_SUITE(FileEntryTest);
  CPPUNIT_TEST(testSetupDir);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  
  void testSetupDir();
};


CPPUNIT_TEST_SUITE_REGISTRATION( FileEntryTest );

void FileEntryTest::testSetupDir()
{
  string topDir = "/tmp";
  string dir = "aria2-FileEntryTest-testSetupDir";
  string filename = "filename";
  string path = topDir+"/"+dir+"/"+filename;
  File d(topDir+"/"+dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  FileEntry fileEntry(dir+"/"+filename, 0, 0);
  fileEntry.setupDir(topDir);
  CPPUNIT_ASSERT(d.isDir());
  File f(path);
  CPPUNIT_ASSERT(!f.exists());
}
