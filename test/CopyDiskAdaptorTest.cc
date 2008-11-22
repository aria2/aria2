#include "CopyDiskAdaptor.h"
#include "FileEntry.h"
#include "Exception.h"
#include "a2io.h"
#include "array_fun.h"
#include "TestUtil.h"
#include <string>
#include <cerrno>
#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class CopyDiskAdaptorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(CopyDiskAdaptorTest);
  CPPUNIT_TEST(testUtime);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void testUtime();
};


CPPUNIT_TEST_SUITE_REGISTRATION( CopyDiskAdaptorTest );

void CopyDiskAdaptorTest::testUtime()
{
  std::string storeDir = "/tmp";
  std::string topDir = "aria2_CopyDiskAdaptorTest_testUtime";
  std::string prefix = storeDir+"/"+topDir;
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry("requested", 10, 0)),
    SharedHandle<FileEntry>(new FileEntry("notFound", 10, 10)),
    SharedHandle<FileEntry>(new FileEntry("notRequested", 10, 20)),
    SharedHandle<FileEntry>(new FileEntry("notExtracted", 10, 30)),
    SharedHandle<FileEntry>(new FileEntry("anotherRequested", 10, 40)),
  };

  std::deque<SharedHandle<FileEntry> > fileEntries
    (&entries[0], &entries[arrayLength(entries)]);
  CopyDiskAdaptor adaptor;
  adaptor.setStoreDir(storeDir);
  adaptor.setTopDir(topDir);
  adaptor.setFileEntries(fileEntries);

  entries[0]->setExtracted(true);
  entries[1]->setExtracted(true);
  entries[2]->setExtracted(true);
  entries[4]->setExtracted(true);
  
  entries[2]->setRequested(false);

  createFile(prefix+"/"+entries[0]->getPath(), entries[0]->getLength());
  File(prefix+"/"+entries[1]->getPath()).remove();
  createFile(prefix+"/"+entries[2]->getPath(), entries[2]->getLength());
  createFile(prefix+"/"+entries[3]->getPath(), entries[3]->getLength());
  createFile(prefix+"/"+entries[4]->getPath(), entries[4]->getLength());

  time_t atime = (time_t) 100000;
  time_t mtime = (time_t) 200000;
  
  CPPUNIT_ASSERT_EQUAL((size_t)2, adaptor.utime(Time(atime), Time(mtime)));
  
  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(prefix+"/"+entries[0]->getPath())
		       .getModifiedTime().getTime());

  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(prefix+"/"+entries[4]->getPath())
		       .getModifiedTime().getTime());

  CPPUNIT_ASSERT((time_t)mtime != File(prefix+"/"+entries[1]->getPath())
		 .getModifiedTime().getTime());
  CPPUNIT_ASSERT((time_t)mtime != File(prefix+"/"+entries[2]->getPath())
		 .getModifiedTime().getTime());
  CPPUNIT_ASSERT((time_t)mtime != File(prefix+"/"+entries[3]->getPath())
		 .getModifiedTime().getTime());

}

} // namespace aria2
