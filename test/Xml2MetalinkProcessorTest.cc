#include "Xml2MetalinkProcessor.h"
#include "Exception.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class Xml2MetalinkProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Xml2MetalinkProcessorTest);
  CPPUNIT_TEST(testParseFile);
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
};


CPPUNIT_TEST_SUITE_REGISTRATION( Xml2MetalinkProcessorTest );

void Xml2MetalinkProcessorTest::testParseFile() {
  Xml2MetalinkProcessor proc;
  try {
  MetalinkerHandle metalinker = proc.parseFile("test.xml");

  MetalinkEntries::iterator entryItr = metalinker->entries.begin();
  MetalinkEntryHandle entry1 = *entryItr;
  CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), entry1->filename);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, entry1->size);
  CPPUNIT_ASSERT_EQUAL(string("0.5.2"), entry1->version);
  CPPUNIT_ASSERT_EQUAL(string("en-US"), entry1->language);
  CPPUNIT_ASSERT_EQUAL(string("Linux-x86"), entry1->os);
  CPPUNIT_ASSERT_EQUAL(string("a96cf3f0266b91d87d5124cf94326422800b627d"),
		       entry1->checksum->getMessageDigest());
  CPPUNIT_ASSERT(DIGEST_ALGO_SHA1 == entry1->checksum->getDigestAlgo());

  MetalinkResources::iterator resourceItr1 = entry1->resources.begin();
  MetalinkResourceHandle resource1 = *resourceItr1;
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_FTP, resource1->type);
  CPPUNIT_ASSERT_EQUAL(string("jp"), resource1->location);
  CPPUNIT_ASSERT_EQUAL(100, resource1->preference);
  CPPUNIT_ASSERT_EQUAL(string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
		       resource1->url);
  resourceItr1++;
  MetalinkResourceHandle resource2 = *resourceItr1;
  CPPUNIT_ASSERT_EQUAL((int)MetalinkResource::TYPE_HTTP, resource2->type);
  CPPUNIT_ASSERT_EQUAL(string("us"), resource2->location);
  CPPUNIT_ASSERT_EQUAL(100, resource2->preference);
  CPPUNIT_ASSERT_EQUAL(string("http://httphost/aria2-0.5.2.tar.bz2"),
		       resource2->url);

  entryItr++;

  MetalinkEntryHandle entry2 = *entryItr;
  CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.1.tar.bz2"), entry2->filename);
  CPPUNIT_ASSERT_EQUAL((int64_t)345689, entry2->size);
  CPPUNIT_ASSERT_EQUAL(string("0.5.1"), entry2->version);
  CPPUNIT_ASSERT_EQUAL(string("ja-JP"), entry2->language);
  CPPUNIT_ASSERT_EQUAL(string("Linux-m68k"), entry2->os);
  CPPUNIT_ASSERT_EQUAL(string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
		       entry2->checksum->getMessageDigest());
  CPPUNIT_ASSERT_EQUAL((size_t)2, entry2->chunkChecksum->pieceHashes.size());
  CPPUNIT_ASSERT_EQUAL(262144, entry2->chunkChecksum->pieceLength);
  CPPUNIT_ASSERT_EQUAL(string("179463a88d79cbf0b1923991708aead914f26142"),
		       entry2->chunkChecksum->pieceHashes.at(0));
  CPPUNIT_ASSERT_EQUAL(string("fecf8bc9a1647505fe16746f94e97a477597dbf3"),
		       entry2->chunkChecksum->pieceHashes.at(1));

  CPPUNIT_ASSERT(DIGEST_ALGO_SHA1 == entry2->checksum->getDigestAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}
