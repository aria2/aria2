#include "SinkStreamFilter.h"

#include <cstdlib>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Segment.h"
#include "ByteArrayDiskWriter.h"
#include "SinkStreamFilter.h"
#include "MockSegment.h"

namespace aria2 {

class SinkStreamFilterTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SinkStreamFilterTest);
  CPPUNIT_TEST(testTransform_with_length);
  CPPUNIT_TEST(testTransform_without_length);
  CPPUNIT_TEST_SUITE_END();

  class MockSegment2 : public MockSegment {
  public:
    MockSegment2(int32_t length) : length(length), writtenLength(0) {}

    virtual int64_t getLength() const CXX11_OVERRIDE { return length; }

    virtual int64_t getWrittenLength() const CXX11_OVERRIDE
    {
      return writtenLength;
    }

    virtual void updateWrittenLength(int64_t bytes) CXX11_OVERRIDE
    {
      writtenLength += bytes;
    }

    int64_t length;
    int64_t writtenLength;
  };

  std::shared_ptr<SinkStreamFilter> filter_;
  std::shared_ptr<ByteArrayDiskWriter> writer_;
  std::shared_ptr<MockSegment2> segment_;

  void clearWriter() { writer_->setString(""); }

public:
  void setUp()
  {
    writer_.reset(new ByteArrayDiskWriter());
    filter_.reset(new SinkStreamFilter());
    filter_->init();
    segment_.reset(new MockSegment2(16));
  }

  void testTransform_with_length();
  void testTransform_without_length();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SinkStreamFilterTest);

void SinkStreamFilterTest::testTransform_with_length()
{
  // If segment_->getLength() > 0, make sure that at most
  // segment_->getLength()-segment_->getWrittenLength() bytes are
  // written.
  std::string msg("01234567890123456");
  ssize_t r = filter_->transform(
      writer_, segment_, reinterpret_cast<const unsigned char*>(msg.c_str()),
      msg.size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, r);
}

void SinkStreamFilterTest::testTransform_without_length()
{
  // If segment_->getLength() == 0, all incoming bytes are written.
  segment_->length = 0;
  std::string msg("01234567890123456");
  ssize_t r = filter_->transform(
      writer_, segment_, reinterpret_cast<const unsigned char*>(msg.c_str()),
      msg.size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)17, r);
}

} // namespace aria2
