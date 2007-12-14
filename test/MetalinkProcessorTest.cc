#include "MetalinkProcessorFactory.h"
#include "MetalinkParserStateMachine.h"
#include "Exception.h"
#include "DefaultDiskWriter.h"
#include "ByteArrayDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

class MetalinkProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkProcessorTest);
  CPPUNIT_TEST(testParseFile);
  CPPUNIT_TEST(testParseFromBinaryStream);
  CPPUNIT_TEST(testMalformedXML);
  CPPUNIT_TEST(testMalformedXML2);
  CPPUNIT_TEST(testBadSize);
  CPPUNIT_TEST(testBadMaxConn);
  CPPUNIT_TEST(testNoName);
  CPPUNIT_TEST(testBadURLPrefs);
  CPPUNIT_TEST(testBadURLMaxConn);
#ifdef ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST(testUnsupportedType);
  CPPUNIT_TEST(testMultiplePieces);
  CPPUNIT_TEST(testBadPieceNo);
  CPPUNIT_TEST(testBadPieceLength);
  CPPUNIT_TEST(testUnsupportedType_piece);
#endif // ENABLE_MESSAGE_DIGEST
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testParseFile();
  void testParseFromBinaryStream();
  void testMalformedXML();
  void testMalformedXML2();
  void testBadSize();
  void testBadMaxConn();
  void testNoName();
  void testBadURLPrefs();
  void testBadURLMaxConn();
#ifdef ENABLE_MESSAGE_DIGEST
  void testUnsupportedType();
  void testMultiplePieces();
  void testBadPieceNo();
  void testBadPieceLength();
  void testUnsupportedType_piece();
#endif // ENABLE_MESSAGE_DIGEST
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkProcessorTest );

void MetalinkProcessorTest::testParseFile()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  try {
    MetalinkerHandle metalinker = proc->parseFile("test.xml");

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

void MetalinkProcessorTest::testParseFromBinaryStream()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  DefaultDiskWriterHandle dw = new DefaultDiskWriter();
  dw->openExistingFile("test.xml");
  
  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);

    MetalinkEntries::iterator entryItr = m->entries.begin();
    MetalinkEntryHandle entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), entry1->getPath());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testMalformedXML()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink><files></file></metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
  }
}

void MetalinkProcessorTest::testMalformedXML2()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink><files></files>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e << endl;
    delete e;
  }
}

void MetalinkProcessorTest::testBadSize()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
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
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);

    MetalinkEntries::iterator entryItr = m->entries.begin();
    MetalinkEntryHandle e = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), e->getPath());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, e->getLength());
    CPPUNIT_ASSERT_EQUAL(string("0.5.2"), e->version);
    CPPUNIT_ASSERT_EQUAL(string("en-US"), e->language);
    CPPUNIT_ASSERT_EQUAL(string("Linux-x86"), e->os);

  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadMaxConn()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
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
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);

    MetalinkEntries::iterator entryItr = m->entries.begin();
    MetalinkEntryHandle e = *entryItr;
    CPPUNIT_ASSERT_EQUAL((int64_t)43743838, e->getLength());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testNoName()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
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
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.size());
    MetalinkEntries::iterator entryItr = m->entries.begin();
    MetalinkEntryHandle e = *entryItr;
    CPPUNIT_ASSERT_EQUAL(string("aria2-0.5.2.tar.bz2"), e->getPath());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadURLPrefs()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2-0.5.2.tar.bz2\">"
		"  <size>43743838</size>"
		"  <version>0.5.2</version>"
		"  <language>en-US</language>"
		"  <os>Linux-x86</os>"
		"  <resources>"
		"    <url type=\"ftp\" maxconnections=\"1\" preference=\"xyz\" location=\"JP\">ftp://mirror/</url>"
		"  </resources>"		
		"</file>"
		"</files>"
		"</metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    MetalinkResourceHandle r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(0, r->preference);
    CPPUNIT_ASSERT_EQUAL(1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(string("JP"), r->location);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadURLMaxConn()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2-0.5.2.tar.bz2\">"
		"  <size>43743838</size>"
		"  <version>0.5.2</version>"
		"  <language>en-US</language>"
		"  <os>Linux-x86</os>"
		"  <resources>"
		"    <url maxconnections=\"xyz\" type=\"ftp\" preference=\"100\" location=\"JP\">ftp://mirror/</url>"
		"  </resources>"		
		"</file>"
		"</files>"
		"</metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    MetalinkResourceHandle r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(100, r->preference);
    CPPUNIT_ASSERT_EQUAL(-1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(string("JP"), r->location);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

#ifdef ENABLE_MESSAGE_DIGEST
void MetalinkProcessorTest::testUnsupportedType()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
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
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    CPPUNIT_ASSERT_EQUAL((size_t)3, e->resources.size());
    MetalinkResourceHandle r1 = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r1->type);
    MetalinkResourceHandle r2 = e->resources[1];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_NOT_SUPPORTED, r2->type);
    MetalinkResourceHandle r3 = e->resources[2];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, r3->type);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testMultiplePieces()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2.tar.bz2\">"
		"  <verification>"
		"    <pieces length=\"512\" type=\"md5\">"
		"    </pieces>"
		"    <pieces length=\"1024\" type=\"sha1\">"
		"    </pieces>"
		"    <pieces length=\"2048\" type=\"sha256\">"
		"    </pieces>"
		"  </verification>"
		"</file>"
		"</files>"
		"</metalink>");

  try {
    // aria2 prefers sha1
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    ChunkChecksumHandle c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(string("sha1"), c->getAlgo());
    CPPUNIT_ASSERT_EQUAL(1024, c->getChecksumLength());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadPieceNo()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2.tar.bz2\">"
		"  <verification>"
		"    <pieces length=\"512\" type=\"sha1\">"
		"      <hash piece=\"0\">abc</hash>"
		"      <hash piece=\"xyz\">xyz</hash>"
		"    </pieces>"
		"    <pieces length=\"512\" type=\"sha256\">"
		"      <hash piece=\"0\">abc</hash>"
		"    </pieces>"
		"  </verification>"
		"</file>"
		"</files>"
		"</metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    ChunkChecksumHandle c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadPieceLength()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2.tar.bz2\">"
		"  <verification>"
		"    <pieces length=\"xyz\" type=\"sha1\">"
		"      <hash piece=\"0\">abc</hash>"
		"    </pieces>"
		"    <pieces length=\"1024\" type=\"sha256\">"
		"      <hash piece=\"0\">abc</hash>"
		"    </pieces>"
		"  </verification>"
		"</file>"
		"</files>"
		"</metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    ChunkChecksumHandle c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testUnsupportedType_piece()
{
  MetalinkProcessorHandle proc = MetalinkProcessorFactory::newInstance();
  ByteArrayDiskWriterHandle dw = new ByteArrayDiskWriter();
  dw->setString("<metalink>"
		"<files>"
		"<file name=\"aria2.tar.bz2\">"
		"  <verification>"
		"    <pieces length=\"512\" type=\"ARIA2\">"
		"      <hash piece=\"0\">abc</hash>"
		"    </pieces>"
		"    <pieces length=\"1024\" type=\"sha256\">"
		"      <hash piece=\"0\">abc</hash>"
		"    </pieces>"
		"  </verification>"
		"</file>"
		"</files>"
		"</metalink>");

  try {
    MetalinkerHandle m = proc->parseFromBinaryStream(dw);
    MetalinkEntryHandle e = m->entries[0];
    ChunkChecksumHandle c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}
#endif // ENABLE_MESSAGE_DIGEST
