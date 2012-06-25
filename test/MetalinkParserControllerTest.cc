#include "MetalinkParserController.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "FileEntry.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "Checksum.h"
# include "ChunkChecksum.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "Signature.h"

namespace aria2 {

class MetalinkParserControllerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkParserControllerTest);
  CPPUNIT_TEST(testEntryTransaction);
  CPPUNIT_TEST(testResourceTransaction);
  CPPUNIT_TEST(testResourceTransaction_withBaseUri);
  CPPUNIT_TEST(testMetaurlTransaction);
#ifdef ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testChecksumTransaction);
  CPPUNIT_TEST(testChunkChecksumTransaction);
  CPPUNIT_TEST(testChunkChecksumTransactionV4);
#endif // ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testSignatureTransaction);

  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void tearDown() {}

  void testEntryTransaction();
  void testResourceTransaction();
  void testResourceTransaction_withBaseUri();
  void testMetaurlTransaction();
#ifdef ENABLE_MESSAGE_DIGEST
  void testChecksumTransaction();
  void testChunkChecksumTransaction();
  void testChunkChecksumTransactionV4();
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
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    SharedHandle<MetalinkEntry> e = m->getEntries().front();
    CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), e->file->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)(1024*1024LL), e->file->getLength());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, e->file->getOffset());
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), e->version);
    CPPUNIT_ASSERT_EQUAL(std::string("ja_JP"), e->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux"), e->oses[0]);
  }
  ctrl.newEntryTransaction();
  ctrl.cancelEntryTransaction();
  CPPUNIT_ASSERT_EQUAL((size_t)1, ctrl.getResult()->getEntries().size());
}

void MetalinkParserControllerTest::testResourceTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.setURLOfResource("http://mirror/aria2.tar.bz2");
  ctrl.setTypeOfResource("http");
  ctrl.setLocationOfResource("US");
  ctrl.setPriorityOfResource(100);
  ctrl.setMaxConnectionsOfResource(1);
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().front()->resources.size());
    SharedHandle<MetalinkResource> res = m->getEntries().front()->resources[0];
    CPPUNIT_ASSERT_EQUAL(std::string("http://mirror/aria2.tar.bz2"), res->url);
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, res->type);
    CPPUNIT_ASSERT_EQUAL(std::string("US"), res->location);
    CPPUNIT_ASSERT_EQUAL(100, res->priority);
    CPPUNIT_ASSERT_EQUAL(1, res->maxConnections);
  }
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.cancelResourceTransaction();
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)2, m->getEntries().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries()[0]->resources.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->getEntries()[1]->resources.size());
  }
}

void MetalinkParserControllerTest::testResourceTransaction_withBaseUri()
{
  MetalinkParserController ctrl;
  ctrl.setBaseUri("http://base/dir/file");
  ctrl.newEntryTransaction();
  ctrl.newResourceTransaction();
  ctrl.setURLOfResource("aria2.tar.bz2");
  ctrl.commitResourceTransaction();
#ifdef ENABLE_BITTORRENT
  ctrl.newMetaurlTransaction();
  ctrl.setURLOfMetaurl("/meta/aria2.tar.bz2.torrent");
  ctrl.setMediatypeOfMetaurl("torrent");
  ctrl.commitMetaurlTransaction();
  ctrl.newMetaurlTransaction();
  ctrl.setURLOfMetaurl("magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c");
  ctrl.setMediatypeOfMetaurl("torrent");
  ctrl.commitMetaurlTransaction();
#endif // ENABLE_BITTORRENT
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries()[0]->resources.size());
    SharedHandle<MetalinkResource> res = m->getEntries()[0]->resources[0];
    CPPUNIT_ASSERT_EQUAL(std::string("http://base/dir/aria2.tar.bz2"),
                         res->url);
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, res->type);

#ifdef ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)2, m->getEntries()[0]->metaurls.size());
    SharedHandle<MetalinkMetaurl> metaurl = m->getEntries()[0]->metaurls[0];
    CPPUNIT_ASSERT_EQUAL(std::string("http://base/meta/aria2.tar.bz2.torrent"),
                         metaurl->url);

    metaurl = m->getEntries()[0]->metaurls[1];
    CPPUNIT_ASSERT_EQUAL(std::string("magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c"),
                         metaurl->url);
#endif // ENABLE_BITTORRENT
  }
}

void MetalinkParserControllerTest::testMetaurlTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newMetaurlTransaction();
  ctrl.setURLOfMetaurl("http://example.org/chocolate.torrent");
  ctrl.setMediatypeOfMetaurl("torrent");
  ctrl.setPriorityOfMetaurl(999);
  ctrl.setNameOfMetaurl("mybirthdaycake");
  ctrl.commitEntryTransaction();
#ifdef ENABLE_BITTORRENT
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries()[0]->metaurls.size());
    SharedHandle<MetalinkMetaurl> metaurl = m->getEntries()[0]->metaurls[0];
    CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/chocolate.torrent"),
                         metaurl->url);
    CPPUNIT_ASSERT_EQUAL(std::string("torrent"), metaurl->mediatype);
    CPPUNIT_ASSERT_EQUAL(std::string("mybirthdaycake"), metaurl->name);
    CPPUNIT_ASSERT_EQUAL(999, metaurl->priority);
  }
  ctrl.newEntryTransaction();
  ctrl.newMetaurlTransaction();
  ctrl.cancelMetaurlTransaction();
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)2, ctrl.getResult()->getEntries().size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries()[0]->metaurls.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->getEntries()[1]->metaurls.size());
  }
#else // !ENABLE_BITTORRENT
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, m->getEntries()[0]->metaurls.size());
  }
#endif // !ENABLE_BITTORRENT
}

#ifdef ENABLE_MESSAGE_DIGEST
void MetalinkParserControllerTest::testChecksumTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.setTypeOfChecksum("md5");
  ctrl.setHashOfChecksum("acbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    SharedHandle<Checksum> md = m->getEntries().front()->checksum;
    CPPUNIT_ASSERT_EQUAL(std::string("md5"), md->getHashType());
    CPPUNIT_ASSERT_EQUAL(std::string("acbd18db4cc2f85cedef654fccc4a4d8"),
                         util::toHex(md->getDigest()));
  }
  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.setTypeOfChecksum("md5");
  ctrl.setHashOfChecksum("badhash");
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[1]->checksum);

  ctrl.newEntryTransaction();
  ctrl.newChecksumTransaction();
  ctrl.cancelChecksumTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[2]->checksum);
}

void MetalinkParserControllerTest::testChunkChecksumTransaction()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.setTypeOfChunkChecksum("md5");
  ctrl.setLengthOfChunkChecksum(256*1024);
  ctrl.addHashOfChunkChecksum(4, "4cbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.addHashOfChunkChecksum(1, "1cbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.addHashOfChunkChecksum(3, "3cbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.addHashOfChunkChecksum(2, "2cbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.addHashOfChunkChecksum(5, "5cbd18db4cc2f85cedef654fccc4a4d8");
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    SharedHandle<ChunkChecksum> md = m->getEntries().front()->chunkChecksum;
    CPPUNIT_ASSERT_EQUAL(std::string("md5"), md->getHashType());
    CPPUNIT_ASSERT_EQUAL(256*1024, md->getPieceLength());
    CPPUNIT_ASSERT_EQUAL((size_t)5, md->countPieceHash());
    CPPUNIT_ASSERT_EQUAL(std::string("1cbd18db4cc2f85cedef654fccc4a4d8"),
                         md->getPieceHashes()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("2cbd18db4cc2f85cedef654fccc4a4d8"),
                         md->getPieceHashes()[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("3cbd18db4cc2f85cedef654fccc4a4d8"),
                         md->getPieceHashes()[2]);
    CPPUNIT_ASSERT_EQUAL(std::string("4cbd18db4cc2f85cedef654fccc4a4d8"),
                         md->getPieceHashes()[3]);
    CPPUNIT_ASSERT_EQUAL(std::string("5cbd18db4cc2f85cedef654fccc4a4d8"),
                         md->getPieceHashes()[4]);
  }
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.setTypeOfChunkChecksum("md5");
  ctrl.setLengthOfChunkChecksum(256*1024);
  ctrl.addHashOfChunkChecksum(1, "badhash");
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[1]->chunkChecksum);

  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransaction();
  ctrl.cancelChunkChecksumTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[2]->chunkChecksum);
}

void MetalinkParserControllerTest::testChunkChecksumTransactionV4()
{
  MetalinkParserController ctrl;
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransactionV4();
  ctrl.setTypeOfChunkChecksumV4("sha-1");
  ctrl.setLengthOfChunkChecksumV4(256*1024);

  ctrl.addHashOfChunkChecksumV4("5bd9f7248df0f3a6a86ab6c95f48787d546efa14");
  ctrl.addHashOfChunkChecksumV4("9413ee70957a09d55704123687478e07f18c7b29");
  ctrl.addHashOfChunkChecksumV4("44213f9f4d59b557314fadcd233232eebcac8012");
  ctrl.commitEntryTransaction();
  {
    SharedHandle<Metalinker> m = ctrl.getResult();
    SharedHandle<ChunkChecksum> md = m->getEntries().front()->chunkChecksum;
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), md->getHashType());
    CPPUNIT_ASSERT_EQUAL(256*1024, md->getPieceLength());
    CPPUNIT_ASSERT_EQUAL((size_t)3, md->countPieceHash());
    CPPUNIT_ASSERT_EQUAL
      (std::string("5bd9f7248df0f3a6a86ab6c95f48787d546efa14"),
       util::toHex(md->getPieceHashes()[0]));
    CPPUNIT_ASSERT_EQUAL
      (std::string("9413ee70957a09d55704123687478e07f18c7b29"),
       util::toHex(md->getPieceHashes()[1]));
    CPPUNIT_ASSERT_EQUAL
      (std::string("44213f9f4d59b557314fadcd233232eebcac8012"),
       util::toHex(md->getPieceHashes()[2]));
  }
  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransactionV4();
  ctrl.setTypeOfChunkChecksumV4("sha-1");
  ctrl.setLengthOfChunkChecksumV4(256*1024);
  ctrl.addHashOfChunkChecksumV4("5bd9f7248df0f3a6a86ab6c95f48787d546efa14");
  ctrl.addHashOfChunkChecksumV4("badhash");
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[1]->chunkChecksum);

  ctrl.newEntryTransaction();
  ctrl.newChunkChecksumTransactionV4();
  ctrl.cancelChunkChecksumTransactionV4();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[2]->chunkChecksum);
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
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
  SharedHandle<Signature> sig = m->getEntries().front()->getSignature();
  CPPUNIT_ASSERT_EQUAL(std::string("pgp"), sig->getType());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.sig"), sig->getFile());
  CPPUNIT_ASSERT_EQUAL(pgpSignature, sig->getBody());

  // See when signature transaction is canceled:
  ctrl.newEntryTransaction();
  ctrl.newSignatureTransaction();
  ctrl.cancelSignatureTransaction();
  ctrl.commitEntryTransaction();
  CPPUNIT_ASSERT(!ctrl.getResult()->getEntries()[1]->getSignature());
}

} // namespace aria2
