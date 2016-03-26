#include "ChunkedDecodingStreamFilter.h"

#include <cstdlib>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "DlAbortEx.h"
#include "Segment.h"
#include "ByteArrayDiskWriter.h"
#include "SinkStreamFilter.h"
#include "MockSegment.h"
#include "a2functional.h"

namespace aria2 {

class ChunkedDecodingStreamFilterTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ChunkedDecodingStreamFilterTest);
  CPPUNIT_TEST(testTransform);
  CPPUNIT_TEST(testTransform_withoutTrailer);
  CPPUNIT_TEST(testTransform_with2Trailers);
  CPPUNIT_TEST(testTransform_largeChunkSize);
  CPPUNIT_TEST(testTransform_tooLargeChunkSize);
  CPPUNIT_TEST(testTransform_chunkSizeMismatch);
  CPPUNIT_TEST(testGetName);
  CPPUNIT_TEST_SUITE_END();

  std::unique_ptr<ChunkedDecodingStreamFilter> filter_;
  std::shared_ptr<ByteArrayDiskWriter> writer_;
  std::shared_ptr<Segment> segment_;

  void clearWriter() { writer_->setString(""); }

public:
  void setUp()
  {
    writer_ = std::make_shared<ByteArrayDiskWriter>();
    auto sinkFilter = make_unique<SinkStreamFilter>();
    sinkFilter->init();
    filter_ = make_unique<ChunkedDecodingStreamFilter>(std::move(sinkFilter));
    filter_->init();
    segment_ = std::make_shared<MockSegment>();
  }

  void testTransform();
  void testTransform_withoutTrailer();
  void testTransform_with2Trailers();
  void testTransform_largeChunkSize();
  void testTransform_tooLargeChunkSize();
  void testTransform_chunkSizeMismatch();
  void testGetName();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChunkedDecodingStreamFilterTest);

void ChunkedDecodingStreamFilterTest::testTransform()
{
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("a\r\n1234567890\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)10, r);
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890"), writer_->getString());
    CPPUNIT_ASSERT_EQUAL((size_t)15, filter_->getBytesProcessed());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  try {
    // Feed extension; see it is ignored.
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("3;extensionIgnored\r\n123\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)3, r);
    CPPUNIT_ASSERT_EQUAL(std::string("123"), writer_->getString());
    CPPUNIT_ASSERT_EQUAL((size_t)25, filter_->getBytesProcessed());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }

  clearWriter();
  // Feed 2extensions; see it is ignored.
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>(
            "3;extension1;extension2;\r\n123\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)3, r);
    CPPUNIT_ASSERT_EQUAL(std::string("123"), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  // Not all chunk size is available
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("1");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("0\r\n1234567890123456\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)16, r);
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890123456"), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  // Not all chunk data is available
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("10\r\n1234567890");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)10, r);
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890"), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("123456\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)6, r);
    CPPUNIT_ASSERT_EQUAL(std::string("123456"), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  // no trailing CR LF.
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("10\r\n1234567890123456");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)16, r);
    CPPUNIT_ASSERT_EQUAL(std::string("1234567890123456"), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  clearWriter();
  // feed only CR
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("\r");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  // feed next LF
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
    CPPUNIT_ASSERT_EQUAL(std::string(""), writer_->getString());
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  // feed 0 CR LF.
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("0\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  // feed trailer
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("trailer\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  // feed final CRLF
  try {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("\r\n");
    ssize_t r = filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_ASSERT_EQUAL((ssize_t)0, r);
  }
  catch (DlAbortEx& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
  // input is over
  CPPUNIT_ASSERT(filter_->finished());
}

void ChunkedDecodingStreamFilterTest::testTransform_withoutTrailer()
{
  CPPUNIT_ASSERT_EQUAL(
      (ssize_t)0, filter_->transform(
                      writer_, segment_,
                      reinterpret_cast<const unsigned char*>("0\r\n\r\n"), 5));
  CPPUNIT_ASSERT(filter_->finished());
}

void ChunkedDecodingStreamFilterTest::testTransform_with2Trailers()
{
  CPPUNIT_ASSERT_EQUAL(
      (ssize_t)0,
      filter_->transform(
          writer_, segment_,
          reinterpret_cast<const unsigned char*>("0\r\nt1\r\nt2\r\n\r\n"), 13));
  CPPUNIT_ASSERT(filter_->finished());
}

void ChunkedDecodingStreamFilterTest::testTransform_largeChunkSize()
{
  // chunkSize should be under 2^63-1
  {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("7fffffffffffffff\r\n");
    filter_->transform(writer_, segment_, msg.data(), msg.size());
  }
}

void ChunkedDecodingStreamFilterTest::testTransform_tooLargeChunkSize()
{
  // chunkSize 2^64 causes error
  {
    std::basic_string<unsigned char> msg =
        reinterpret_cast<const unsigned char*>("ffffffffffffffff\r\n");
    try {
      filter_->transform(writer_, segment_, msg.data(), msg.size());
      CPPUNIT_FAIL("exception must be thrown.");
    }
    catch (DlAbortEx& e) {
      // success
    }
  }
}

void ChunkedDecodingStreamFilterTest::testTransform_chunkSizeMismatch()
{
  std::basic_string<unsigned char> msg =
      reinterpret_cast<const unsigned char*>("3\r\n1234\r\n");
  try {
    filter_->transform(writer_, segment_, msg.data(), msg.size());
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlAbortEx& e) {
    // success
  }
}

void ChunkedDecodingStreamFilterTest::testGetName()
{
  CPPUNIT_ASSERT_EQUAL(std::string("ChunkedDecodingStreamFilter"),
                       filter_->getName());
}

} // namespace aria2
