#include "MetalinkParserController.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "FileEntry.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "Signature.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class MetalinkParserControllerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkParserControllerTest);
  CPPUNIT_TEST(testEntryTransaction);
  CPPUNIT_TEST(testResourceTransaction);
#ifdef ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testChecksumTransaction);
  CPPUNIT_TEST(testChunkChecksumTransaction);
#endif // ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testSignatureTransaction);

  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void tearDown() {}

  void testEntryTransaction();
  void testResourceTransaction();
#ifdef ENABLE_MESSAGE_DIGEST
  void testChecksumTransaction();
  void testChunkChecksumTransaction();
#endif // ENABLE_MESSAGE_DIGEST
  void testSignatureTransaction();
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
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.size());
    SharedHandle<MetalinkEntry> e = m->entries.front();
    CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), e->file->getPath());
    CPPUNIT_ASSERT_EQUAL(1024*1024ULL, e->file->getLength());
    CPPUNIT_ASSERT_EQUAL((off_t)0, e->file->getOffset());
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), e->version);
    CPPUNIT_ASSERT_EQUAL(std::string("ja_JP"), e->language);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux"), e->os);
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
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.front()->resources.size());
    SharedHandle<MetalinkResource> res = m->entries.front()->resources[0];
    CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.tar.bz2"), res->url);
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, res->type);
    CPPUNIT_ASSERT_EQUAL(std::string("US"), res->location);
    CPPUNIT_ASSERT_EQUAL(100, res->preference);
    CPPUNIT_ASSERT_EQUAL(1, res->maxConnections);
  }
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.cancelResourceTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT_EQUAL((size_t)1, ctrl.getResult()->entries.front()->resources.size());
}

#ifdef ENABLE_MESSAGE_DIGEST
void MetalinkParserControllerTest::testChecksumTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.setTypeOfChecksum("md5");
  ctrl.setHashOfChecksum("hash");
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    SharedHandle<Checksum> md = m->entries.front()->checksum;
    CPPUNIT_ASSERT_EQUAL(std::string("md5"), md->getAlgo());
    CPPUNIT_ASSERT_EQUAL(std::string("hash"), md->getMessageDigest());
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
    SharedHandle<Metalinker> m = ctrl.getResult();
    SharedHandle<ChunkChecksum> md = m->entries.front()->chunkChecksum;
    CPPUNIT_ASSERT_EQUAL(std::string("md5"), md->getAlgo());
    CPPUNIT_ASSERT_EQUAL((size_t)256*1024, md->getChecksumLength());
    CPPUNIT_ASSERT_EQUAL((size_t)5, md->countChecksum());
    CPPUNIT_ASSERT_EQUAL(std::string("hash1"), md->getChecksums()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("hash2"), md->getChecksums()[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("hash3"), md->getChecksums()[2]);
    CPPUNIT_ASSERT_EQUAL(std::string("hash4"), md->getChecksums()[3]);
    CPPUNIT_ASSERT_EQUAL(std::string("hash5"), md->getChecksums()[4]);
  }
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.cancelChunkChecksumTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(ctrl.getResult()->entries[1]->chunkChecksum.isNull());
}
#endif // ENABLE_MESSAGE_DIGEST

void MetalinkParserControllerTest::testSignatureTransaction()
{
  static std::string pgpSignature =
    "-----BEGIN PGP SIGNATURE-----\n"
    "Version: GnuPG v1.4.9 (GNU/Linux)\n"
    "\n"
    "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n"
    "ffffffffffffffffffffffff\n"
    "fffff\n"
    "-----END PGP SIGNATURE-----\n";

  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();

  ctrl.newSignatureTransaction();
  ctrl.setTypeOfSignature("pgp");
  ctrl.setFileOfSignature("aria2.sig");
  ctrl.setBodyOfSignature(pgpSignature);
  // commitEntryTransaction also commits signature transaction.
  ctrl.commitEntryTransaction();

  SharedHandle<Metalinker> m = ctrl.getResult();
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.size());
  SharedHandle<Signature> sig = m->entries.front()->getSignature();
  CPPUNIT_ASSERT_EQUAL(std::string("pgp"), sig->getType());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sig"), sig->getFile());
  CPPUNIT_ASSERT_EQUAL(pgpSignature, sig->getBody());

  // See when signature transaction is canceled:
  ctrl.newEntryTransaction();
  ctrl.newSignatureTransaction();
  ctrl.cancelSignatureTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(ctrl.getResult()->entries[1]->getSignature().isNull());
}

} // namespace aria2
