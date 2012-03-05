#include "SinkStreamFilter.h"

#include <cstdlib>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Segment.h"
#include "ByteArrayDiskWriter.h"
#include "SinkStreamFilter.h"
#include "MockSegment.h"

namespace aria2 {

class SinkStreamFilterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SinkStreamFilterTest);
  CPPUNIT_TEST(testTransform_with_length);
  CPPUNIT_TEST(testTransform_without_length);
  CPPUNIT_TEST_SUITE_END();

  class MockSegment2:public MockSegment {
  public:
    MockSegment2(int32_t length):length(length), writtenLength(0) {}

    virtual int32_t getLength() const
    {
      return length;
    }

    virtual int32_t getWrittenLength() const
    {
      return writtenLength;
    }

    virtual void updateWrittenLength(int32_t bytes)
    {
      writtenLength += bytes;
    }

    int32_t length;
    int32_t writtenLength;
  };

  SharedHandle<SinkStreamFilter> filter_;
  SharedHandle<SinkStreamFilter> sinkFilter_;
  SharedHandle<ByteArrayDiskWriter> writer_;
  SharedHandle<MockSegment2> segment_;

  void clearWriter()
  {
    writer_->setString("");
  }
public:
  void setUp()
  {
    writer_.reset(new ByteArrayDiskWriter());
    sinkFilter_.reset(new SinkStreamFilter());
    filter_.reset(new SinkStreamFilter(sinkFilter_));
    sinkFilter_->init();
    filter_->init();
    segment_.reset(new MockSegment2(16));
  }

  void testTransform_with_length();
  void testTransform_without_length();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SinkStreamFilterTest );

void SinkStreamFilterTest::testTransform_with_length()
{
  // If segment_->getLength() > 0, make sure that at most
  // segment_->getLength()-segment_->getWrittenLength() bytes are
  // written.
  std::string msg("01234567890123456");
  ssize_t r = filter_->transform
    (writer_, segment_,
     reinterpret_cast<const unsigned char*>(msg.c_str()), msg.size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)16, r);
}

void SinkStreamFilterTest::testTransform_without_length()
{
  // If segment_->getLength() == 0, all incoming bytes are written.
  segment_->length = 0;
  std::string msg("01234567890123456");
  ssize_t r = filter_->transform
    (writer_, segment_,
     reinterpret_cast<const unsigned char*>(msg.c_str()), msg.size());
  CPPUNIT_ASSERT_EQUAL((ssize_t)17, r);
}

} // namespace aria2
