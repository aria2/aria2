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
    adaptor->setStoreDir(".");
    adaptor->setTopDir(".");
  }

  void testWriteData();
  void testReadData();
  void testCutTrailingGarbage();
  void testSize();
  void testUtime();
  void testResetDiskWriterEntries();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MultiDiskAdaptorTest );

std::deque<SharedHandle<FileEntry> > createEntries() {
  SharedHandle<FileEntry> array[] = {
    SharedHandle<FileEntry>(new FileEntry("file1.txt", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry("file2.txt", 15, 0)),
    SharedHandle<FileEntry>(new FileEntry("file3.txt", 7, 15)),
    SharedHandle<FileEntry>(new FileEntry("file4.txt", 0, 22)),
    SharedHandle<FileEntry>(new FileEntry("file5.txt", 2, 22)),
    SharedHandle<FileEntry>(new FileEntry("file6.txt", 0, 24)),
  };
  std::deque<SharedHandle<FileEntry> > entries(&array[0],
					       &array[arrayLength(array)]);
  for(std::deque<SharedHandle<FileEntry> >::const_iterator i = entries.begin();
      i != entries.end(); ++i) {
    File((*i)->getPath()).remove();
  }
  return entries;
}

void MultiDiskAdaptorTest::testResetDiskWriterEntries()
{
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    // Because entries[1] spans entries[0]
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[0]->setRequested(false);
    fileEntries[1]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(entries[0]->getDiskWriter().isNull());
    // Because entries[2] spans entries[1]
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    // Because entries[4] spans entries[3]
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    // Because entries[3] spans entries[4]
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[3]->setRequested(false);
    fileEntries[4]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[5]->getDiskWriter().isNull());

    adaptor->closeFile();
  }
  {
    std::deque<SharedHandle<FileEntry> > fileEntries = createEntries();
    fileEntries[5]->setRequested(false);
    adaptor->setFileEntries(fileEntries);
    // In openFile(), resetDiskWriterEntries() are called.
    adaptor->openFile();
    
    std::deque<SharedHandle<DiskWriterEntry> > entries =
      adaptor->getDiskWriterEntries();
    CPPUNIT_ASSERT(!entries[0]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[1]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[2]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[3]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(!entries[4]->getDiskWriter().isNull());
    CPPUNIT_ASSERT(entries[5]->getDiskWriter().isNull());

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
  try {
  adaptor->setFileEntries(createEntries());

  adaptor->openFile();
  std::string msg = "12345";
  adaptor->writeData((const unsigned char*)msg.c_str(), msg.size(), 0);
  adaptor->closeFile();

  CPPUNIT_ASSERT(File("file1.txt").isFile());
  char buf[128];
  readFile("file2.txt", buf, 5);
  buf[5] = '\0';
  CPPUNIT_ASSERT_EQUAL(msg, std::string(buf));

  adaptor->openFile();
  std::string msg2 = "67890ABCDEF";
  adaptor->writeData((const unsigned char*)msg2.c_str(), msg2.size(), 5);
  adaptor->closeFile();

  readFile("file2.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890ABCDE"), std::string(buf));
  readFile("file3.txt", buf, 1);
  buf[1] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("F"), std::string(buf));

  adaptor->openFile();
  std::string msg3 = "12345123456712";
  adaptor->writeData((const unsigned char*)msg3.c_str(), msg3.size(), 10);
  adaptor->closeFile();

  readFile("file2.txt", buf, 15);
  buf[15] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("123456789012345"), std::string(buf));
  readFile("file3.txt", buf, 7);
  buf[7] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("1234567"), std::string(buf));

  CPPUNIT_ASSERT(File("file4.txt").isFile());

  readFile("file5.txt", buf, 2);
  buf[2] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("12"), std::string(buf));

  CPPUNIT_ASSERT(File("file6.txt").isFile());

  } catch(Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
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

void MultiDiskAdaptorTest::testCutTrailingGarbage()
{
  std::string dir = "/tmp";
  std::string topDir = ".";
  std::string topDirPath = dir+"/"+topDir;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testCutTrailingGarbage_";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(prefix+"1", 256, 0)),
    SharedHandle<FileEntry>(new FileEntry(prefix+"2", 512, 256))
  };
  for(size_t i = 0; i < arrayLength(entries); ++i) {
    createFile(topDirPath+"/"+entries[i]->getPath(),
	       entries[i]->getLength()+100);
  }
  std::deque<SharedHandle<FileEntry> > fileEntries
    (&entries[0], &entries[arrayLength(entries)]);
  
  MultiDiskAdaptor adaptor;
  adaptor.setStoreDir(dir);
  adaptor.setTopDir(topDir);
  adaptor.setFileEntries(fileEntries);
  adaptor.setMaxOpenFiles(1);
  adaptor.setPieceLength(1);
  
  adaptor.openFile();

  adaptor.cutTrailingGarbage();

  CPPUNIT_ASSERT_EQUAL((uint64_t)256,
		       File(topDirPath+"/"+entries[0]->getPath()).size());
  CPPUNIT_ASSERT_EQUAL((uint64_t)512,
		       File(topDirPath+"/"+entries[1]->getPath()).size());
}

void MultiDiskAdaptorTest::testSize()
{
  std::string dir = "/tmp";
  std::string topDir = ".";
  std::string topDirPath = dir+"/"+topDir;
  std::string prefix = "aria2_MultiDiskAdaptorTest_testSize_";
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry(prefix+"1", 1, 0)),
    SharedHandle<FileEntry>(new FileEntry(prefix+"2", 1, 1))
  };
  for(size_t i = 0; i < arrayLength(entries); ++i) {
    createFile(topDirPath+"/"+entries[i]->getPath(), entries[i]->getLength());
  }
  std::deque<SharedHandle<FileEntry> > fileEntries
    (&entries[0], &entries[arrayLength(entries)]);
  
  MultiDiskAdaptor adaptor;
  adaptor.setStoreDir(dir);
  adaptor.setTopDir(topDir);
  adaptor.setFileEntries(fileEntries);
  adaptor.setMaxOpenFiles(1);
  adaptor.setPieceLength(1);
  
  adaptor.openFile();

  CPPUNIT_ASSERT_EQUAL((uint64_t)2, adaptor.size());
}

void MultiDiskAdaptorTest::testUtime()
{
  std::string storeDir = "/tmp";
  std::string topDir = "aria2_MultiDiskAdaptorTest_testUtime";
  std::string prefix = storeDir+"/"+topDir;
  SharedHandle<FileEntry> entries[] = {
    SharedHandle<FileEntry>(new FileEntry("requested", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry("notFound", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry("notRequested", 0, 0)),
    SharedHandle<FileEntry>(new FileEntry("anotherRequested", 0, 0)),
  };

  createFile(prefix+"/"+entries[0]->getPath(), entries[0]->getLength());
  File(prefix+"/"+entries[1]->getPath()).remove();
  createFile(prefix+"/"+entries[2]->getPath(), entries[2]->getLength());
  createFile(prefix+"/"+entries[3]->getPath(), entries[3]->getLength());

  entries[2]->setRequested(false);

  std::deque<SharedHandle<FileEntry> > fileEntries
    (&entries[0], &entries[arrayLength(entries)]);
  MultiDiskAdaptor adaptor;
  adaptor.setStoreDir(storeDir);
  adaptor.setTopDir(topDir);
  adaptor.setFileEntries(fileEntries);

  time_t atime = (time_t) 100000;
  time_t mtime = (time_t) 200000;
  
  CPPUNIT_ASSERT_EQUAL((size_t)2, adaptor.utime(Time(atime), Time(mtime)));
  
  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(prefix+"/"+entries[0]->getPath())
		       .getModifiedTime().getTime());

  CPPUNIT_ASSERT_EQUAL((time_t)mtime,
		       File(prefix+"/"+entries[3]->getPath())
		       .getModifiedTime().getTime());

  CPPUNIT_ASSERT((time_t)mtime != File(prefix+"/"+entries[2]->getPath())
		 .getModifiedTime().getTime());
}

} // namespace aria2
