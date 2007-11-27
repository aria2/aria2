#include "Xml2MetalinkProcessor.h"
#include "Exception.h"
#include "DefaultDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class Xml2MetalinkProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Xml2MetalinkProcessorTest);
  CPPUNIT_TEST(testParseFile);
  CPPUNIT_TEST(testParseFromBinaryStream);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
    xmlInitParser();
  }
  void tearDown() {
    xmlCleanupParser();
  }

  void testParseFile();
  void testParseFromBinaryStream();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Xml2MetalinkProcessorTest );

void Xml2MetalinkProcessorTest::testParseFile() {
  Xml2MetalinkProcessor proc;
  try {
    MetalinkerHandle metalinker = proc.parseFile("test.xml");

    MetalinkEntries::iterator entryItr = metalinker->entries.begin();
    MetalinkEntryHandle entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), entry1->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, entry1->getLength());
    CPPUNIT_ASSERT_EQUAL(string("0.5.2"), entry1->version);
    CPPUNIT_ASSERT_EQUAL(string("en-US"), entry1->language);
    CPPUNIT_ASSERT_EQUAL(string("Linux-x86"), entry1->os);
    CPPUNIT_ASSERT_EQUAL((int32_t)1, entry1->maxConnections);
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(string("a96cf3f0266b91d87d5124cf94326422800b627d"),
			 entry1->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL(string("sha1"), entry1->checksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST
    MetalinkResources::iterator resourceItr1 = entry1->resources.begin();
    MetalinkResourceHandle resource1 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, resource1->type);
    CPPUNIT_ASSERT_EQUAL(string("JP"), resource1->location);
    CPPUNIT_ASSERT_EQUAL((int32_t)100, resource1->preference);
    CPPUNIT_ASSERT_EQUAL(string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
			 resource1->url);
    CPPUNIT_ASSERT_EQUAL((int32_t)1, resource1->maxConnections);

    resourceItr1++;
    MetalinkResourceHandle resource2 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, resource2->type);
    CPPUNIT_ASSERT_EQUAL(string("US"), resource2->location);
    CPPUNIT_ASSERT_EQUAL((int32_t)100, resource2->preference);
    CPPUNIT_ASSERT_EQUAL(string("http://httphost/aria2-0.5.2.tar.bz2"),
			 resource2->url);
    CPPUNIT_ASSERT_EQUAL((int32_t)-1, resource2->maxConnections);

    entryItr++;

    MetalinkEntryHandle entry2 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.1.tar.bz2"), entry2->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)345689, entry2->getLength());
    CPPUNIT_ASSERT_EQUAL(string("0.5.1"), entry2->version);
    CPPUNIT_ASSERT_EQUAL(string("ja-JP"), entry2->language);
    CPPUNIT_ASSERT_EQUAL(string("Linux-m68k"), entry2->os);
    CPPUNIT_ASSERT_EQUAL((int32_t)-1, entry2->maxConnections);
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
			 entry2->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL((int32_t)2, entry2->chunkChecksum->countChecksum());
    CPPUNIT_ASSERT_EQUAL((int32_t)262144, entry2->chunkChecksum->getChecksumLength());
    CPPUNIT_ASSERT_EQUAL(string("179463a88d79cbf0b1923991708aead914f26142"),
			 entry2->chunkChecksum->getChecksum(0));
    CPPUNIT_ASSERT_EQUAL(string("fecf8bc9a1647505fe16746f94e97a477597dbf3"),
			 entry2->chunkChecksum->getChecksum(1));
    CPPUNIT_ASSERT_EQUAL(string("sha1"), entry2->checksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST

    entryItr++;

    // test case: verification hash is not provided
    MetalinkEntryHandle entry3 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("NoVerificationHash"), entry3->getPath());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT(entry3->checksum.isNull());
    CPPUNIT_ASSERT(entry3->chunkChecksum.isNull());
#endif // ENABLE_MESSAGE_DIGEST

    entryItr++;

    // test case: unsupported verification hash is included
    MetalinkEntryHandle entry4 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("UnsupportedVerificationHashTypeIncluded"), entry4->getPath());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(string("sha1"),
			 entry4->checksum->getAlgo());
    CPPUNIT_ASSERT_EQUAL(string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
			 entry4->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL(string("sha1"),
			 entry4->chunkChecksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST


  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void Xml2MetalinkProcessorTest::testParseFromBinaryStream() {
  Xml2MetalinkProcessor proc;
  try {
    DefaultDiskWriterHandle dw = new DefaultDiskWriter();
    dw->openExistingFile("test.xml");
    
    MetalinkerHandle metalinker = proc.parseFromBinaryStream(dw);

    dw->closeFile();

    MetalinkEntries::iterator entryItr = metalinker->entries.begin();
    MetalinkEntryHandle entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), entry1->getPath());
  } catch(Exception* e) {
    cerr << *e;
    CPPUNIT_FAIL(e->getMsg());
    delete e;
    throw;
  }
}
