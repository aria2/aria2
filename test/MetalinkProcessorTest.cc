#include "MetalinkProcessorFactory.h"
#include "MetalinkParserStateMachine.h"
#include "Exception.h"
#include "DefaultDiskWriter.h"
#include "ByteArrayDiskWriter.h"
#include "MetalinkProcessor.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "MetalinkResource.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "ChunkChecksum.h"
# include "Checksum.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  CPPUNIT_TEST(testLargeFileSize);
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
  void testLargeFileSize();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkProcessorTest );

void MetalinkProcessorTest::testParseFile()
{
  SharedHandle<MetalinkProcessor> proc(MetalinkProcessorFactory::newInstance());
  try {
    SharedHandle<Metalinker> metalinker = proc->parseFile("test.xml");

    std::deque<SharedHandle<MetalinkEntry> >::iterator entryItr = metalinker->entries.begin();

    SharedHandle<MetalinkEntry> entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), entry1->getPath());
    CPPUNIT_ASSERT_EQUAL(0ULL, entry1->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), entry1->version);
    CPPUNIT_ASSERT_EQUAL(std::string("en-US"), entry1->language);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), entry1->os);
    CPPUNIT_ASSERT_EQUAL(1, entry1->maxConnections);
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(std::string("a96cf3f0266b91d87d5124cf94326422800b627d"),
			 entry1->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"), entry1->checksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST
    std::deque<SharedHandle<MetalinkResource> >::iterator resourceItr1 = entry1->resources.begin();
    SharedHandle<MetalinkResource> resource1 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, resource1->type);
    CPPUNIT_ASSERT_EQUAL(std::string("JP"), resource1->location);
    CPPUNIT_ASSERT_EQUAL(100, resource1->preference);
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
			 resource1->url);
    CPPUNIT_ASSERT_EQUAL(1, resource1->maxConnections);

    resourceItr1++;
    SharedHandle<MetalinkResource> resource2 = *resourceItr1;
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, resource2->type);
    CPPUNIT_ASSERT_EQUAL(std::string("US"), resource2->location);
    CPPUNIT_ASSERT_EQUAL(100, resource2->preference);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
			 resource2->url);
    CPPUNIT_ASSERT_EQUAL(-1, resource2->maxConnections);

    entryItr++;

    SharedHandle<MetalinkEntry> entry2 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.1.tar.bz2"), entry2->getPath());
    CPPUNIT_ASSERT_EQUAL(345689ULL, entry2->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.1"), entry2->version);
    CPPUNIT_ASSERT_EQUAL(std::string("ja-JP"), entry2->language);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-m68k"), entry2->os);
    CPPUNIT_ASSERT_EQUAL(-1, entry2->maxConnections);
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(std::string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
			 entry2->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL((size_t)2, entry2->chunkChecksum->countChecksum());
    CPPUNIT_ASSERT_EQUAL((size_t)262144, entry2->chunkChecksum->getChecksumLength());
    CPPUNIT_ASSERT_EQUAL(std::string("179463a88d79cbf0b1923991708aead914f26142"),
			 entry2->chunkChecksum->getChecksum(0));
    CPPUNIT_ASSERT_EQUAL(std::string("fecf8bc9a1647505fe16746f94e97a477597dbf3"),
			 entry2->chunkChecksum->getChecksum(1));
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"), entry2->checksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST

    entryItr++;

    // test case: verification hash is not provided
    SharedHandle<MetalinkEntry> entry3 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("NoVerificationHash"), entry3->getPath());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT(entry3->checksum.isNull());
    CPPUNIT_ASSERT(entry3->chunkChecksum.isNull());
#endif // ENABLE_MESSAGE_DIGEST

    entryItr++;

    // test case: unsupported verification hash is included
    SharedHandle<MetalinkEntry> entry4 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("UnsupportedVerificationHashTypeIncluded"), entry4->getPath());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"),
			 entry4->checksum->getAlgo());
    CPPUNIT_ASSERT_EQUAL(std::string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
			 entry4->checksum->getMessageDigest());
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"),
			 entry4->chunkChecksum->getAlgo());
#endif // ENABLE_MESSAGE_DIGEST


  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testParseFromBinaryStream()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  DefaultDiskWriterHandle dw(new DefaultDiskWriter());
  dw->openExistingFile("test.xml");
  
  try {
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);

    std::deque<SharedHandle<MetalinkEntry> >::iterator entryItr = m->entries.begin();
    SharedHandle<MetalinkEntry> entry1 = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), entry1->getPath());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testMalformedXML()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
  dw->setString("<metalink><files></file></metalink>");

  try {
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void MetalinkProcessorTest::testMalformedXML2()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
  dw->setString("<metalink><files></files>");

  try {
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void MetalinkProcessorTest::testBadSize()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);

    std::deque<SharedHandle<MetalinkEntry> >::iterator entryItr = m->entries.begin();
    SharedHandle<MetalinkEntry> e = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), e->getPath());
    CPPUNIT_ASSERT_EQUAL(0ULL, e->getLength());
    CPPUNIT_ASSERT_EQUAL(std::string("0.5.2"), e->version);
    CPPUNIT_ASSERT_EQUAL(std::string("en-US"), e->language);
    CPPUNIT_ASSERT_EQUAL(std::string("Linux-x86"), e->os);

  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadMaxConn()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);

    std::deque<SharedHandle<MetalinkEntry> >::iterator entryItr = m->entries.begin();
    SharedHandle<MetalinkEntry> e = *entryItr;
    CPPUNIT_ASSERT_EQUAL(43743838ULL, e->getLength());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testNoName()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    CPPUNIT_ASSERT_EQUAL((size_t)1, m->entries.size());
    std::deque<SharedHandle<MetalinkEntry> >::iterator entryItr = m->entries.begin();
    SharedHandle<MetalinkEntry> e = *entryItr;
    CPPUNIT_ASSERT_EQUAL(std::string("aria2-0.5.2.tar.bz2"), e->getPath());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadURLPrefs()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<MetalinkResource> r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(0, r->preference);
    CPPUNIT_ASSERT_EQUAL(1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(std::string("JP"), r->location);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadURLMaxConn()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<MetalinkResource> r = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r->type);
    CPPUNIT_ASSERT_EQUAL(100, r->preference);
    CPPUNIT_ASSERT_EQUAL(-1, r->maxConnections);
    CPPUNIT_ASSERT_EQUAL(std::string("JP"), r->location);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

#ifdef ENABLE_MESSAGE_DIGEST
void MetalinkProcessorTest::testUnsupportedType()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    CPPUNIT_ASSERT_EQUAL((size_t)3, e->resources.size());
    SharedHandle<MetalinkResource> r1 = e->resources[0];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_FTP, r1->type);
    SharedHandle<MetalinkResource> r2 = e->resources[1];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_NOT_SUPPORTED, r2->type);
    SharedHandle<MetalinkResource> r3 = e->resources[2];
    CPPUNIT_ASSERT_EQUAL(MetalinkResource::TYPE_HTTP, r3->type);
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testMultiplePieces()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<ChunkChecksum> c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"), c->getAlgo());
    CPPUNIT_ASSERT_EQUAL((size_t)1024, c->getChecksumLength());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadPieceNo()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<ChunkChecksum> c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(std::string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testBadPieceLength()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<ChunkChecksum> c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(std::string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}

void MetalinkProcessorTest::testUnsupportedType_piece()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    SharedHandle<ChunkChecksum> c = e->chunkChecksum;
 
    CPPUNIT_ASSERT_EQUAL(std::string("sha256"), c->getAlgo());
  } catch(Exception* e) {
    CPPUNIT_FAIL(e->getMsg());
    delete e;
  }
}
#endif // ENABLE_MESSAGE_DIGEST

void MetalinkProcessorTest::testLargeFileSize()
{
  SharedHandle<MetalinkProcessor> proc = MetalinkProcessorFactory::newInstance();
  SharedHandle<ByteArrayDiskWriter> dw(new ByteArrayDiskWriter());
  dw->setString("<metalink>"
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
    SharedHandle<Metalinker> m = proc->parseFromBinaryStream(dw);
    SharedHandle<MetalinkEntry> e = m->entries[0];
    CPPUNIT_ASSERT_EQUAL(9223372036854775807ULL, e->getLength());
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    std::string m = e->getMsg();
    delete e;
    CPPUNIT_FAIL(m);
  }
}

} // namespace aria2
