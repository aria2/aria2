#include "metalink_helper.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "MetalinkParserStateMachine.h"
#include "Exception.h"
#include "DefaultDiskWriter.h"
#include "ByteArrayDiskWriter.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#include "MetalinkMetaurl.h"
#include "MessageDigest.h"
#include "ChunkChecksum.h"
#include "Checksum.h"
#include "Signature.h"
#include "fmt.h"
#include "RecoverableException.h"
#include "util.h"

namespace aria2 {

class MetalinkProcessorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkProcessorTest);
  CPPUNIT_TEST(testParseFileV4);
  CPPUNIT_TEST(testParseFileV4_attrs);
  CPPUNIT_TEST(testParseFile);
  CPPUNIT_TEST(testParseFile_dirtraversal);
  CPPUNIT_TEST(testParseBinaryStream);
  CPPUNIT_TEST(testMalformedXML);
  CPPUNIT_TEST(testMalformedXML2);
  CPPUNIT_TEST(testBadSize);
  CPPUNIT_TEST(testBadSizeV4);
  CPPUNIT_TEST(testBadMaxConn);
  CPPUNIT_TEST(testNoName);
  CPPUNIT_TEST(testBadURLPrefs);
  CPPUNIT_TEST(testBadURLMaxConn);
  CPPUNIT_TEST(testUnsupportedType);
  CPPUNIT_TEST(testMultiplePieces);
  CPPUNIT_TEST(testBadPieceNo);
  CPPUNIT_TEST(testBadPieceLength);
  CPPUNIT_TEST(testUnsupportedType_piece);
  CPPUNIT_TEST(testLargeFileSize);
  CPPUNIT_TEST(testXmlPrefixV3);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testParseFileV4();
  void testParseFileV4_attrs();
  void testParseFile();
  void testParseFile_dirtraversal();
  void testParseBinaryStream();
  void testMalformedXML();
  void testMalformedXML2();
  void testBadSize();
  void testBadSizeV4();
  void testBadMaxConn();
  void testNoName();
  void testBadURLPrefs();
  void testBadURLMaxConn();
  void testUnsupportedType();
  void testMultiplePieces();
  void testBadPieceNo();
  void testBadPieceLength();
  void testUnsupportedType_piece();
  void testLargeFileSize();
  void testXmlPrefixV3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MetalinkProcessorTest);

void MetalinkProcessorTest::testParseFileV4()
{
  auto m = metalink::parseFile(A2_TEST_DIR "/metalink4.xml");
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
  auto& e = m->getEntries()[0];
  CPPUNIT_ASSERT_EQUAL(std::string("example.ext"), e->getPath());
  CPPUNIT_ASSERT_EQUAL((int64_t)786430LL, e->getLength());
  CPPUNIT_ASSERT_EQUAL(-1, e->maxConnections);
  CPPUNIT_ASSERT_EQUAL(std::string("0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33"),
                       util::toHex(e->checksum->getDigest()));
  CPPUNIT_ASSERT(e->checksum);
  CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), e->checksum->getHashType());
  CPPUNIT_ASSERT(e->chunkChecksum);
  if (MessageDigest::supports("sha-256")) {
    CPPUNIT_ASSERT_EQUAL(std::string("sha-256"),
                         e->chunkChecksum->getHashType());
    CPPUNIT_ASSERT_EQUAL(262144, e->chunkChecksum->getPieceLength());
    CPPUNIT_ASSERT_EQUAL((size_t)3, e->chunkChecksum->countPieceHash());
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "0245178074fd042e19b7c3885b360fc21064b30e73f5626c7e3b005d048069c5"),
        util::toHex(e->chunkChecksum->getPieceHash(0)));
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "487ba2299be7f759d7c7bf6a4ac3a32cee81f1bb9332fc485947e32918864fb2"),
        util::toHex(e->chunkChecksum->getPieceHash(1)));
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "37290d74ac4d186e3a8e5785d259d2ec04fac91ae28092e7620ec8bc99e830aa"),
        util::toHex(e->chunkChecksum->getPieceHash(2)));
  }
  else {
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), e->chunkChecksum->getHashType());
    CPPUNIT_ASSERT_EQUAL(262144, e->chunkChecksum->getPieceLength());
    CPPUNIT_ASSERT_EQUAL((size_t)3, e->chunkChecksum->countPieceHash());
    CPPUNIT_ASSERT_EQUAL(
        std::string("5bd9f7248df0f3a6a86ab6c95f48787d546efa14"),
        util::toHex(e->chunkChecksum->getPieceHash(0)));
    CPPUNIT_ASSERT_EQUAL(
        std::string("9413ee70957a09d55704123687478e07f18c7b29"),
        util::toHex(e->chunkChecksum->getPieceHash(1)));
    CPPUNIT_ASSERT_EQUAL(
        std::string("44213f9f4d59b557314fadcd233232eebcac8012"),
        util::toHex(e->chunkChecksum->getPieceHash(2)));
  }
  CPPUNIT_ASSERT(e->getSignature());
  CPPUNIT_ASSERT_EQUAL(std::string("application/pgp-signature"),
                       e->getSignature()->getType());
  CPPUNIT_ASSERT_EQUAL(std::string("a signature"),
                       e->getSignature()->getBody());

  CPPUNIT_ASSERT_EQUAL((size_t)2, e->resources.size());
  auto& r = e->resources[0];
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftp.example.com/example.ext"),
                       r->url);
  CPPUNIT_ASSERT_EQUAL(std::string("de"), r->location);
  CPPUNIT_ASSERT_EQUAL(1, r->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("ftp"),
                       MetalinkResource::getTypeString(r->type));
  CPPUNIT_ASSERT_EQUAL(-1, r->maxConnections);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)1, e->metaurls.size());
  auto& mu = e->metaurls[0];
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.com/example.ext.torrent"),
                       mu->url);
  CPPUNIT_ASSERT_EQUAL(2, mu->priority);
  CPPUNIT_ASSERT_EQUAL(std::string("torrent"), mu->mediatype);
#else  // !ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)0, e->metaurls.size());
#endif // !ENABLE_BITTORRENT
}

void MetalinkProcessorTest::testParseFileV4_attrs()
{
  std::unique_ptr<Metalinker> m;
  ByteArrayDiskWriter dw;
  {
    // Testing file@name
    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"%s\">"
                       "<url>http://example.org</url>"
                       "</file>"
                       "</metalink>";
    dw.setString(fmt(tmpl, "foo"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    // empty name
    dw.setString(fmt(tmpl, ""));
    try {
      metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }

    // dir traversing
    dw.setString(fmt(tmpl, "../doughnuts"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
  }
  {
    // Testing url@priority
    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<url priority=\"%s\">http://example.org</url>"
                       "</file>"
                       "</metalink>";
    dw.setString(fmt(tmpl, "0"));
    try {
      metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }

    dw.setString(fmt(tmpl, "1"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "100"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "999999"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "1000000"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
    dw.setString(fmt(tmpl, "A"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
  {
    // Testing metaurl@priority
    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<metaurl priority=\"%s\" "
                       "mediatype=\"torrent\">http://example.org</metaurl>"
                       "</file>"
                       "</metalink>";
    dw.setString(fmt(tmpl, "0"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }

    dw.setString(fmt(tmpl, "1"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "100"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "999999"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    dw.setString(fmt(tmpl, "1000000"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
    dw.setString(fmt(tmpl, "A"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
  {
    // Testing metaurl@mediatype

    // no mediatype
    dw.setString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                 "<file name=\"example.ext\">"
                 "<metaurl>http://example.org</metaurl>"
                 "</file>"
                 "</metalink>");
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }

    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<metaurl mediatype=\"%s\">http://example.org</metaurl>"
                       "</file>"
                       "</metalink>";

    dw.setString(fmt(tmpl, "torrent"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    // empty mediatype
    dw.setString(fmt(tmpl, ""));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
  }
  {
    // Testing metaurl@name
    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<metaurl mediatype=\"torrent\" "
                       "name=\"%s\">http://example.org</metaurl>"
                       "</file>"
                       "</metalink>";

    dw.setString(fmt(tmpl, "foo"));
    m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());

    // dir traversing
    dw.setString(fmt(tmpl, "../doughnuts"));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
    // empty name
    dw.setString(fmt(tmpl, ""));
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
      // success
    }
  }
  {
    // Testing pieces@length
    // No pieces@length
    dw.setString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                 "<file name=\"example.ext\">"
                 "<url>http://example.org</url>"
                 "<pieces type=\"sha-1\">"
                 "<hash>0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
                 "</pieces>"
                 "</file>"
                 "</metalink>");
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }

    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<url>http://example.org</url>"
                       "<pieces length=\"%s\" type=\"sha-1\">"
                       "<hash>0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
                       "</pieces>"
                       "</file>"
                       "</metalink>";

    dw.setString(fmt(tmpl, "262144"));
    m = metalink::parseBinaryStream(&dw);
    // empty
    try {
      dw.setString(fmt(tmpl, ""));
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
    // not a number
    try {
      dw.setString(fmt(tmpl, "A"));
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
  {
    // Testing pieces@type
    // No pieces@type
    dw.setString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                 "<file name=\"example.ext\">"
                 "<url>http://example.org</url>"
                 "<pieces length=\"262144\">"
                 "<hash>0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
                 "</pieces>"
                 "</file>"
                 "</metalink>");
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }

    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<url>http://example.org</url>"
                       "<pieces length=\"262144\" type=\"%s\">"
                       "<hash>0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
                       "</pieces>"
                       "</file>"
                       "</metalink>";

    dw.setString(fmt(tmpl, "sha-1"));
    m = metalink::parseBinaryStream(&dw);
    // empty
    try {
      dw.setString(fmt(tmpl, ""));
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
  {
    // Testing hash@type
    // No hash@type
    dw.setString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                 "<file name=\"example.ext\">"
                 "<url>http://example.org</url>"
                 "<hash>0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
                 "</file>"
                 "</metalink>");
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }

    const char* tmpl =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
        "<file name=\"example.ext\">"
        "<url>http://example.org</url>"
        "<hash type=\"%s\">0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33</hash>"
        "</file>"
        "</metalink>";

    dw.setString(fmt(tmpl, "sha-1"));
    m = metalink::parseBinaryStream(&dw);
    // empty
    try {
      dw.setString(fmt(tmpl, ""));
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
  {
    // Testing signature@mediatype
    // No hash@type
    dw.setString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                 "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                 "<file name=\"example.ext\">"
                 "<url>http://example.org</url>"
                 "<signature>sig</signature>"
                 "</file>"
                 "</metalink>");
    try {
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }

    const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                       "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                       "<file name=\"example.ext\">"
                       "<url>http://example.org</url>"
                       "<signature mediatype=\"%s\">sig</signature>"
                       "</file>"
                       "</metalink>";

    dw.setString(fmt(tmpl, "application/pgp-signature"));
    m = metalink::parseBinaryStream(&dw);
    // empty
    try {
      dw.setString(fmt(tmpl, ""));
      m = metalink::parseBinaryStream(&dw);
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (RecoverableException& e) {
    }
  }
}

void MetalinkProcessorTest::testParseFile()
{
  try {
    auto metalinker = metalink::parseFile(A2_TEST_DIR "/test.xml");
    auto entryItr = std::begin(metalinker->getEntries());

    auto& entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), entry1->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)0LL, entry1->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), entry1->version);
    CPPUNIT_ASSERT_EQUAL(std::string("en-US"), entry1->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), entry1->oses[0]);
    CPPUNIT_ASSERT_EQUAL(1, entry1->maxConnections);
    CPPUNIT_ASSERT_EQUAL(
        std::string("a96cf3f0266b91d87d5124cf94326422800b627d"),
        util::toHex(entry1->checksum->getDigest()));
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), entry1->checksum->getHashType());
    CPPUNIT_ASSERT(entry1->getSignature());
    CPPUNIT_ASSERT_EQUAL(std::string("pgp"), entry1->getSignature()->getType());
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2.sig"),
                         entry1->getSignature()->getFile());
    // Note that we don't strip anything
    CPPUNIT_ASSERT_EQUAL(
        std::string(
            "\n-----BEGIN PGP SIGNATURE-----\n"
            "Version: GnuPG v1.4.9 (GNU/Linux)\n"
            "\n"
            "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n"
            "ffffffffffffffffffffffff\n"
            "fffff\n"
            "-----END PGP SIGNATURE-----\n"
            "\t"),
        entry1->getSignature()->getBody());

    auto resourceItr1 = std::begin(entry1->resources);
    auto& resource1 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, resource1->type);
    CPPUNIT_ASSERT_EQUAL(std::string("jp"), resource1->location);
    CPPUNIT_ASSERT_EQUAL(1, resource1->priority);
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
                         resource1->url);
    CPPUNIT_ASSERT_EQUAL(1, resource1->maxConnections);

    ++resourceItr1;
    auto& resource2 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, resource2->type);
    CPPUNIT_ASSERT_EQUAL(std::string("us"), resource2->location);
    CPPUNIT_ASSERT_EQUAL(1, resource2->priority);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
                         resource2->url);
    CPPUNIT_ASSERT_EQUAL(-1, resource2->maxConnections);

    ++entryItr;

    auto& entry2 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.1.tar.bz2"), entry2->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)345689LL, entry2->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.1"), entry2->version);
    CPPUNIT_ASSERT_EQUAL(std::string("ja-JP"), entry2->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-m68k"), entry2->oses[0]);
    CPPUNIT_ASSERT_EQUAL(-1, entry2->maxConnections);
    CPPUNIT_ASSERT_EQUAL(
        std::string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
        util::toHex(entry2->checksum->getDigest()));
    CPPUNIT_ASSERT_EQUAL((size_t)2, entry2->chunkChecksum->countPieceHash());
    CPPUNIT_ASSERT_EQUAL(262144, entry2->chunkChecksum->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(
        std::string("179463a88d79cbf0b1923991708aead914f26142"),
        util::toHex(entry2->chunkChecksum->getPieceHash(0)));
    CPPUNIT_ASSERT_EQUAL(
        std::string("fecf8bc9a1647505fe16746f94e97a477597dbf3"),
        util::toHex(entry2->chunkChecksum->getPieceHash(1)));
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), entry2->checksum->getHashType());
    // See that signature is null
    CPPUNIT_ASSERT(!entry2->getSignature());

    ++entryItr;

    // test case: verification hash is not provided
    auto& entry3 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("NoVerificationHash"), entry3->getPath());
    CPPUNIT_ASSERT(!entry3->checksum);
    CPPUNIT_ASSERT(!entry3->chunkChecksum);

    ++entryItr;

    // test case: unsupported verification hash is included
    auto& entry4 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("UnsupportedVerificationHashTypeIncluded"),
                         entry4->getPath());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), entry4->checksum->getHashType());
    CPPUNIT_ASSERT_EQUAL(
        std::string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
        util::toHex(entry4->checksum->getDigest()));
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"),
                         entry4->chunkChecksum->getHashType());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testParseFile_dirtraversal()
{
  auto metalinker =
      metalink::parseFile(A2_TEST_DIR "/metalink3-dirtraversal.xml");
  CPPUNIT_ASSERT_EQUAL((size_t)1, metalinker->getEntries().size());
  auto& e = metalinker->getEntries()[0];
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.3.tar.bz2"), e->getPath());
  CPPUNIT_ASSERT(e->getSignature());
  CPPUNIT_ASSERT_EQUAL(std::string(""), e->getSignature()->getFile());
}

void MetalinkProcessorTest::testParseBinaryStream()
{
  DefaultDiskWriter dw(A2_TEST_DIR "/test.xml");
  dw.enableReadOnly();
  dw.openExistingFile();

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& entry1 = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), entry1->getPath());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testMalformedXML()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" "
               "xmlns=\"http://www.metalinker.org/\"><files></file></"
               "metalink>");

  try {
    metalink::parseBinaryStream(&dw);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void MetalinkProcessorTest::testMalformedXML2()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" "
               "xmlns=\"http://www.metalinker.org/\"><files></files>");

  try {
    metalink::parseBinaryStream(&dw);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void MetalinkProcessorTest::testBadSizeV4()
{
  ByteArrayDiskWriter dw;

  const char* tmpl = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                     "<metalink xmlns=\"urn:ietf:params:xml:ns:metalink\">"
                     "<file name=\"foo\">"
                     "<size>%s</size>"
                     "<url>http://example.org</url>"
                     "</file>"
                     "</metalink>";

  dw.setString(fmt(tmpl, "9223372036854775807"));
  metalink::parseBinaryStream(&dw);

  dw.setString(fmt(tmpl, "-1"));
  try {
    metalink::parseBinaryStream(&dw);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (RecoverableException& e) {
  }
}

void MetalinkProcessorTest::testBadSize()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>abc</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), e->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)0LL, e->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), e->version);
    CPPUNIT_ASSERT_EQUAL(std::string("en-US"), e->languages[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), e->oses[0]);
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testBadMaxConn()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>43743838</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "  <resources maxconnections=\"abc\"/>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL((int64_t)43743838LL, e->getLength());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testNoName()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file>"
               "  <size>1024</size>"
               "  <version>0.0.1</version>"
               "  <language>GB</language>"
               "  <os>Linux-x64</os>"
               "</file>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>43743838</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), e->getPath());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testBadURLPrefs()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>43743838</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "  <resources>"
               "    <url type=\"ftp\" maxconnections=\"1\" preference=\"xyz\""
               "         location=\"jp\">ftp://mirror/</url>"
               "  </resources>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    auto& r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::getLowestPriority(), r->priority);
    CPPUNIT_ASSERT_EQUAL(1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(std::string("jp"), r->location);
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testBadURLMaxConn()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>43743838</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "  <resources>"
               "    <url maxconnections=\"xyz\" type=\"ftp\""
               "         preference=\"100\""
               "         location=\"jp\">ftp://mirror/</url>"
               "  </resources>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    auto& r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(1, r->priority);
    CPPUNIT_ASSERT_EQUAL(-1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(std::string("jp"), r->location);
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testUnsupportedType()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2-0.5.2.tar.bz2\">"
               "  <size>43743838</size>"
               "  <version>0.5.2</version>"
               "  <language>en-US</language>"
               "  <os>Linux-x86</os>"
               "  <resources>"
               "    <url type=\"ftp\">ftp://mirror/</url>"
               "    <url type=\"magnet\">magnet:xt=XYZ</url>"
               "    <url type=\"http\">http://mirror/</url>"
               "  </resources>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL((size_t)3, e->resources.size());
    auto& r1 = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r1->type);
    auto& r2 = e->resources[1];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_NOT_SUPPORTED, r2->type);
    auto& r3 = e->resources[2];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, r3->type);
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testMultiplePieces()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"aria2.tar.bz2\">"
               "  <verification>"
               "    <pieces length=\"1024\" type=\"sha1\">"
               "    </pieces>"
               "    <pieces length=\"512\" type=\"md5\">"
               "    </pieces>"
               "  </verification>"
               "</file>"
               "</files>"
               "</metalink>");

  try {
    // aria2 prefers sha1
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    auto& c = e->chunkChecksum;
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c->getHashType());
    CPPUNIT_ASSERT_EQUAL((int32_t)1_k, c->getPieceLength());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testBadPieceNo()
{
  ByteArrayDiskWriter dw;
  dw.setString(
      "<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
      "<files>"
      "<file name=\"aria2.tar.bz2\">"
      "  <verification>"
      "    <pieces length=\"512\" type=\"sha1\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "      <hash "
      "piece=\"xyz\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "    <pieces length=\"1024\" type=\"sha1\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "  </verification>"
      "</file>"
      "</files>"
      "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    auto& c = e->chunkChecksum;
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((int32_t)1_k, c->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c->getHashType());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testBadPieceLength()
{
  ByteArrayDiskWriter dw;
  dw.setString(
      "<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
      "<files>"
      "<file name=\"aria2.tar.bz2\">"
      "  <verification>"
      "    <pieces length=\"xyz\" type=\"sha1\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "    <pieces length=\"1024\" type=\"sha1\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "  </verification>"
      "</file>"
      "</files>"
      "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    auto& e = m->getEntries()[0];
    auto& c = e->chunkChecksum;
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((int32_t)1_k, c->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c->getHashType());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testUnsupportedType_piece()
{
  ByteArrayDiskWriter dw;
  dw.setString(
      "<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
      "<files>"
      "<file name=\"aria2.tar.bz2\">"
      "  <verification>"
      "    <pieces length=\"512\" type=\"ARIA2\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "    <pieces length=\"1024\" type=\"sha1\">"
      "      <hash piece=\"0\">44213f9f4d59b557314fadcd233232eebcac8012</hash>"
      "    </pieces>"
      "  </verification>"
      "</file>"
      "</files>"
      "</metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    auto& c = e->chunkChecksum;
    CPPUNIT_ASSERT(c);
    CPPUNIT_ASSERT_EQUAL((int32_t)1_k, c->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), c->getHashType());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testLargeFileSize()
{
  ByteArrayDiskWriter dw;
  dw.setString("<metalink version=\"3.0\" xmlns=\"http://www.metalinker.org/\">"
               "<files>"
               "<file name=\"dvd.iso\">"
               "  <size>9223372036854775807</size>"
               "  <resources>"
               "    <url type=\"http\">ftp://mirror/</url>"
               "  </resources>"
               "</file>"
               "</files>"
               "</metalink>");
  try {
    auto m = metalink::parseBinaryStream(&dw);
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL, e->getLength());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void MetalinkProcessorTest::testXmlPrefixV3()
{
  ByteArrayDiskWriter dw;
  dw.setString(
      "<m:metalink version=\"3.0\" xmlns:m=\"http://www.metalinker.org/\">"
      "<m:files>"
      "<m:file name=\"dvd.iso\">"
      "  <m:size>9223372036854775807</m:size>"
      "  <m:resources>"
      "    <m:url type=\"http\">ftp://mirror/</m:url>"
      "  </m:resources>"
      "</m:file>"
      "</m:files>"
      "</m:metalink>");

  try {
    auto m = metalink::parseBinaryStream(&dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->getEntries().size());
    auto& e = m->getEntries()[0];
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL, e->getLength());
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

} // namespace aria2
