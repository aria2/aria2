#include "GZipDecodingStreamFilter.h"

#include <cassert>
#include <iostream>
#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "Segment.h"
#include "ByteArrayDiskWriter.h"
#include "SinkStreamFilter.h"
#include "MockSegment.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

namespace aria2 {

class GZipDecodingStreamFilterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GZipDecodingStreamFilterTest);
  CPPUNIT_TEST(testTransform);
  CPPUNIT_TEST_SUITE_END();

  class MockSegment2:public MockSegment {
  private:
    int64_t positionToWrite_;
  public:
    MockSegment2():positionToWrite_(0) {}

    virtual void updateWrittenLength(int32_t bytes)
    {
      positionToWrite_ += bytes;
    }

    virtual int64_t getPositionToWrite() const
    {
      return positionToWrite_;
    }
  };

  SharedHandle<GZipDecodingStreamFilter> filter_;
  SharedHandle<SinkStreamFilter> sinkFilter_;
  SharedHandle<ByteArrayDiskWriter> writer_;
  SharedHandle<MockSegment2> segment_;
public:
  void setUp()
  {
    writer_.reset(new ByteArrayDiskWriter());
    sinkFilter_.reset(new SinkStreamFilter());
    filter_.reset(new GZipDecodingStreamFilter(sinkFilter_));
    sinkFilter_->init();
    filter_->init();
    segment_.reset(new MockSegment2());
  }

  void testTransform();
};


CPPUNIT_TEST_SUITE_REGISTRATION(GZipDecodingStreamFilterTest);

void GZipDecodingStreamFilterTest::testTransform()
{
  unsigned char buf[4096];
  std::ifstream in(A2_TEST_DIR"/gzip_decode_test.gz", std::ios::binary);
  while(in) {
    in.read(reinterpret_cast<char*>(buf), sizeof(buf));
    filter_->transform(writer_, segment_, buf, in.gcount());
  }
  CPPUNIT_ASSERT(filter_->finished());
#ifdef ENABLE_MESSAGE_DIGEST
  std::string data = writer_->getString();
  SharedHandle<MessageDigest> sha1(MessageDigest::sha1());
  sha1->update(data.data(), data.size());
  CPPUNIT_ASSERT_EQUAL(std::string("8b577b33c0411b2be9d4fa74c7402d54a8d21f96"),
                       util::toHex(sha1->digest()));
#endif // ENABLE_MESSAGE_DIGEST
}

} // namespace aria2
