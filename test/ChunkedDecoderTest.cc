#include "ChunkedDecoder.h"
#include "DlAbortEx.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ChunkedDecoderTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ChunkedDecoderTest);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST(testDecode_tooLargeChunkSize);
  CPPUNIT_TEST(testDecode_chunkSizeMismatch);
  CPPUNIT_TEST(testGetName);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void testDecode();
  void testDecode_tooLargeChunkSize();
  void testDecode_chunkSizeMismatch();
  void testGetName();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ChunkedDecoderTest );

void ChunkedDecoderTest::testDecode()
{
  ChunkedDecoder decoder;
  decoder.init();

  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("a\r\n1234567890\r\n");
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // Feed extension; see it is ignored.
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>
      ("3;extensionIgnored\r\n123\r\n");
    CPPUNIT_ASSERT_EQUAL(std::string("123"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // Not all chunk size is available
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("1");
    CPPUNIT_ASSERT_EQUAL(std::string(),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("0\r\n1234567890123456\r\n");
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890123456"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // Not all chunk data is available
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("10\r\n1234567890");
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("123456\r\n");
    CPPUNIT_ASSERT_EQUAL(std::string("123456"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // no trailing CR LF.
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>
      ("10\r\n1234567890123456");
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890123456"),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // feed only CR
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>
      ("\r");
    CPPUNIT_ASSERT_EQUAL(std::string(),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // feed next LF
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>
      ("\n");
    CPPUNIT_ASSERT_EQUAL(std::string(),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // feed 0 CR LF.
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>
      ("0\r\n");
    CPPUNIT_ASSERT_EQUAL(std::string(),
			 decoder.decode(msg.c_str(), msg.size()));
  }
  // input is over
  CPPUNIT_ASSERT(decoder.finished());

  decoder.release();
}

void ChunkedDecoderTest::testDecode_tooLargeChunkSize()
{
  // chunkSize should be under 2^64-1
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("ffffffffffffffff\r\n");
    ChunkedDecoder decoder;
    decoder.decode(msg.c_str(), msg.size());
  }
  // chunkSize 2^64 causes error
  {
    std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("10000000000000000\r\n");
    ChunkedDecoder decoder;
    try {
      decoder.decode(msg.c_str(), msg.size());
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(DlAbortEx& e) {
      // success
    }
  }
}

void ChunkedDecoderTest::testDecode_chunkSizeMismatch()
{
  std::basic_string<unsigned char> msg =
    reinterpret_cast<const unsigned char*>("3\r\n1234\r\n");

  ChunkedDecoder decoder;
  try {
    decoder.decode(msg.c_str(), msg.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    // success
  }
}

void ChunkedDecoderTest::testGetName()
{
  ChunkedDecoder decoder;
  CPPUNIT_ASSERT_EQUAL(std::string("ChunkedDecoder"), decoder.getName());
}

} // namespace aria2
