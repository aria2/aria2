#include "Piece.h"

#include <string>

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"
#include "DirectDiskAdaptor.h"
#include "ByteArrayDiskWriter.h"
#include "WrDiskCache.h"

namespace aria2 {

class PieceTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PieceTest);
  CPPUNIT_TEST(testCompleteBlock);
  CPPUNIT_TEST(testGetCompletedLength);
  CPPUNIT_TEST(testFlushWrCache);
  CPPUNIT_TEST(testAppendWrCache);

  CPPUNIT_TEST(testGetDigestWithWrCache);
  CPPUNIT_TEST(testUpdateHash);

  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<DirectDiskAdaptor> adaptor_;
  ByteArrayDiskWriter* writer_;

public:
  void setUp()
  {
    adaptor_ = std::make_shared<DirectDiskAdaptor>();
    auto dw = make_unique<ByteArrayDiskWriter>();
    writer_ = dw.get();
    adaptor_->setDiskWriter(std::move(dw));
  }

  void testCompleteBlock();
  void testGetCompletedLength();
  void testFlushWrCache();
  void testAppendWrCache();

  void testGetDigestWithWrCache();
  void testUpdateHash();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PieceTest);

void PieceTest::testCompleteBlock()
{
  size_t blockLength = 32_k;
  Piece p(0, blockLength * 10, blockLength);

  p.completeBlock(5);

  CPPUNIT_ASSERT(p.hasBlock(5));
}

void PieceTest::testGetCompletedLength()
{
  int32_t blockLength = 16_k;
  Piece p(0, blockLength * 10 + 100, blockLength);

  p.completeBlock(1);
  p.completeBlock(2);
  p.completeBlock(9);
  p.completeBlock(10); // <-- 100 bytes

  CPPUNIT_ASSERT_EQUAL((int64_t)(blockLength * 3 + 100),
                       p.getCompletedLength());
}

void PieceTest::testFlushWrCache()
{
  unsigned char* data;
  Piece p(0, 1_k);
  WrDiskCache dc(64);
  p.initWrCache(&dc, adaptor_);
  data = new unsigned char[3];
  memcpy(data, "foo", 3);
  p.updateWrCache(&dc, data, 0, 3, 0);
  data = new unsigned char[4];
  memcpy(data, " bar", 4);
  p.updateWrCache(&dc, data, 0, 4, 3);
  p.flushWrCache(&dc);

  CPPUNIT_ASSERT_EQUAL(std::string("foo bar"), writer_->getString());

  data = new unsigned char[3];
  memcpy(data, "foo", 3);
  p.updateWrCache(&dc, data, 0, 3, 0);
  CPPUNIT_ASSERT_EQUAL((size_t)3, dc.getSize());
  p.clearWrCache(&dc);
  CPPUNIT_ASSERT_EQUAL((size_t)0, dc.getSize());
  p.releaseWrCache(&dc);
  CPPUNIT_ASSERT(!p.getWrDiskCacheEntry());
}

void PieceTest::testAppendWrCache()
{
  unsigned char* data;
  Piece p(0, 1_k);
  WrDiskCache dc(1_k);
  p.initWrCache(&dc, adaptor_);
  size_t capacity = 6;
  data = new unsigned char[capacity];
  memcpy(data, "foo", 3);
  p.updateWrCache(&dc, data, 0, 3, capacity, 0);
  size_t alen = p.appendWrCache(
      &dc, 3, reinterpret_cast<const unsigned char*>("barbaz"), 6);
  CPPUNIT_ASSERT_EQUAL((size_t)3, alen);
  p.flushWrCache(&dc);
  CPPUNIT_ASSERT_EQUAL(std::string("foobar"), writer_->getString());
}

void PieceTest::testGetDigestWithWrCache()
{
  unsigned char* data;
  Piece p(0, 26);
  p.setHashType("sha-1");
  WrDiskCache dc(64);
  //                  012345678901234567890123456
  writer_->setString("abcde...ijklmnopq...uvwx.z");
  p.initWrCache(&dc, adaptor_);
  data = new unsigned char[3];
  memcpy(data, "fgh", 3);
  p.updateWrCache(&dc, data, 0, 3, 5);
  data = new unsigned char[3];
  memcpy(data, "rst", 3);
  p.updateWrCache(&dc, data, 0, 3, 17);
  data = new unsigned char[1];
  memcpy(data, "y", 1);
  p.updateWrCache(&dc, data, 0, 1, 24);

  CPPUNIT_ASSERT_EQUAL(
      std::string("32d10c7b8cf96570ca04ce37f2a19d84240d3a89"),
      util::toHex(p.getDigestWithWrCache(p.getLength(), adaptor_)));
}

void PieceTest::testUpdateHash()
{
  Piece p(0, 16, 2_m);
  p.setHashType("sha-1");

  std::string spam("SPAM!");
  CPPUNIT_ASSERT(p.updateHash(
      0, reinterpret_cast<const unsigned char*>(spam.c_str()), spam.size()));
  CPPUNIT_ASSERT(!p.isHashCalculated());

  std::string spamspam("SPAM!SPAM!!");
  CPPUNIT_ASSERT(p.updateHash(
      spam.size(), reinterpret_cast<const unsigned char*>(spamspam.c_str()),
      spamspam.size()));
  CPPUNIT_ASSERT(p.isHashCalculated());

  CPPUNIT_ASSERT_EQUAL(std::string("d9189aff79e075a2e60271b9556a710dc1bc7de7"),
                       util::toHex(p.getDigest()));
}

} // namespace aria2
