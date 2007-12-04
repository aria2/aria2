#include "MetalinkParserController.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "Checksum.h"
#include "ChunkChecksum.h"
#include <cppunit/extensions/HelperMacros.h>

class MetalinkParserControllerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkParserControllerTest);
  CPPUNIT_TEST(testEntryTransaction);
  CPPUNIT_TEST(testResourceTransaction);
  CPPUNIT_TEST(testChecksumTransaction);
  CPPUNIT_TEST(testChunkChecksumTransaction);

  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void tearDown() {}

  void testEntryTransaction();
  void testResourceTransaction();
  void testChecksumTransaction();
  void testChunkChecksumTransaction();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkParserControllerTest );

void MetalinkParserControllerTest::testEntryTransaction()
{
  MetalinkParserController ctrl;

  ctrl.newEntryTransaction();
  ctrl.setFileNameOfEntry("aria2.tar.bz2");
  ctrl.setFileLengthOfEntry(1024*1024);
  ctrl.setVersionOfEntry("1.0");
  ctrl.setLanguageOfEntry("ja_JP");
  ctrl.setOSOfEntry("Linux");
  ctrl.commitEntryTransaction();
  {
    MetalinkerHandle m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.size());
    MetalinkEntryHandle e = m->entries.front();
    CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), e->file->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)1024*1024, e->file->getLength());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, e->file->getOffset());
    CPPUNIT_ASSERT_EQUAL(string("1.0"), e->version);
    CPPUNIT_ASSERT_EQUAL(string("ja_JP"), e->language);
    CPPUNIT_ASSERT_EQUAL(string("Linux"), e->os);
  }
  ctrl.newEntryTransaction();
  ctrl.cancelEntryTransaction();
  CPPUNIT_ASSERT_EQUAL((size_t)1, ctrl.getResult()->entries.size());
}

void MetalinkParserControllerTest::testResourceTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.setURLOfResource("http://mirror/aria2.tar.bz2");
  ctrl.setTypeOfResource("http");
  ctrl.setLocationOfResource("US");
  ctrl.setPreferenceOfResource(100);
  ctrl.setMaxConnectionsOfResource(1);
  ctrl.commitEntryTransaction();
  {
    MetalinkerHandle m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.front()->resources.size());
    MetalinkResourceHandle res = m->entries.front()->resources[0];
    CPPUNIT_ASSERT_EQUAL(string("http://mirror/aria2.tar.bz2"), res->url);
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, res->type);
    CPPUNIT_ASSERT_EQUAL(string("US"), res->location);
    CPPUNIT_ASSERT_EQUAL(100, res->preference);
    CPPUNIT_ASSERT_EQUAL(1, res->maxConnections);
  }
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.cancelResourceTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT_EQUAL((size_t)1, ctrl.getResult()->entries.front()->resources.size());
}

void MetalinkParserControllerTest::testChecksumTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.setTypeOfChecksum("md5");
  ctrl.setHashOfChecksum("hash");
  ctrl.commitEntryTransaction();
  {
    MetalinkerHandle m = ctrl.getResult();
    ChecksumHandle md = m->entries.front()->checksum;
    CPPUNIT_ASSERT_EQUAL(string("md5"), md->getAlgo());
    CPPUNIT_ASSERT_EQUAL(string("hash"), md->getMessageDigest());
  }
  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.cancelChecksumTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(ctrl.getResult()->entries[1]->checksum.isNull());
}

void MetalinkParserControllerTest::testChunkChecksumTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.setTypeOfChunkChecksum("md5");
  ctrl.setLengthOfChunkChecksum(256*1024);
  ctrl.addHashOfChunkChecksum(4, "hash4");
  ctrl.addHashOfChunkChecksum(1, "hash1");
  ctrl.addHashOfChunkChecksum(3, "hash3");
  ctrl.addHashOfChunkChecksum(2, "hash2");
  ctrl.addHashOfChunkChecksum(5, "hash5");
  ctrl.commitEntryTransaction();
  {
    MetalinkerHandle m = ctrl.getResult();
    ChunkChecksumHandle md = m->entries.front()->chunkChecksum;
    CPPUNIT_ASSERT_EQUAL(string("md5"), md->getAlgo());
    CPPUNIT_ASSERT_EQUAL(256*1024, md->getChecksumLength());
    CPPUNIT_ASSERT_EQUAL(5, md->countChecksum());
    CPPUNIT_ASSERT_EQUAL(string("hash1"), md->getChecksums()[0]);
    CPPUNIT_ASSERT_EQUAL(string("hash2"), md->getChecksums()[1]);
    CPPUNIT_ASSERT_EQUAL(string("hash3"), md->getChecksums()[2]);
    CPPUNIT_ASSERT_EQUAL(string("hash4"), md->getChecksums()[3]);
    CPPUNIT_ASSERT_EQUAL(string("hash5"), md->getChecksums()[4]);
  }
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.cancelChunkChecksumTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(ctrl.getResult()->entries[1]->chunkChecksum.isNull());
}
