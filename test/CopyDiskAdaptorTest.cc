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
  std::string storeDir = "/tmp/aria2_CopyDiskAdaptorTest_testUtime";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/requested", 10, 0)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/notFound", 10, 10)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/notRequested", 10, 20)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/notExtracted", 10, 30)),
    SharedHandle<FileEntry>(new FileEntry(storeDir+"/anotherRequested", 10, 40)),
  };

  std::deque<SharedHandle<FileEntry> > fileEntries
    (&entries[0], &entries[arrayLength(entries)]);
  CopyDiskAdaptor adaptor;
  adaptor.setStoreDir(storeDir);
  adaptor.setFileEntries(fileEntries);

  entries[0]->setExtracted(true);
  entries[1]->setExtracted(true);
  entries[2]->setExtracted(true);
  entries[4]->setExtracted(true);
  
  entries[2]->setRequested(false);

  createFile(entries[0]->getPath(), entries[0]->getLength());
  File(entries[1]->getPath()).remove();
  createFile(entries[2]->getPath(), entries[2]->getLength());
  createFile(entries[3]->getPath(), entries[3]->getLength());
  createFile(entries[4]->getPath(), entries[4]->getLength());

  time_t atime = (time_t) 100000;
  time_t mtime = (time_t) 200000;
  
  CPPUNIT_ASSERT_EQUAL((size_t)2, adaptor.utime(Time(atime), Time(mtime)));
  
  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(entries[0]->getPath()).getModifiedTime().getTime());

  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(entries[4]->getPath()).getModifiedTime().getTime());

  CPPUNIT_ASSERT((time_t)mtime !=
		 File(entries[1]->getPath()).getModifiedTime().getTime());
  CPPUNIT_ASSERT((time_t)mtime !=
		 File(entries[2]->getPath()).getModifiedTime().getTime());
  CPPUNIT_ASSERT((time_t)mtime !=
		 File(entries[3]->getPath()).getModifiedTime().getTime());

}

} // namespace aria2
