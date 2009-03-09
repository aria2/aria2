#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  std::string dir = "/tmp/aria2-FileEntryTest-testSetupDir";
  std::string filename = "filename";
  std::string path = dir+"/"+filename;
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  FileEntry fileEntry(path, 0, 0);
  fileEntry.setupDir();
  CPPUNIT_ASSERT(d.isDir());
  File f(path);
  CPPUNIT_ASSERT(!f.exists());
}

} // namespace aria2
