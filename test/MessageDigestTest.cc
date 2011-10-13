#include "MessageDigest.h"

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"

namespace aria2 {

class MessageDigestTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MessageDigestTest);
  CPPUNIT_TEST(testDigest);
  CPPUNIT_TEST(testSupports);
  CPPUNIT_TEST(testGetDigestLength);
  CPPUNIT_TEST(testIsStronger);
  CPPUNIT_TEST(testIsValidHash);
  CPPUNIT_TEST(testGetCanonicalHashType);
  CPPUNIT_TEST_SUITE_END();

  SharedHandle<MessageDigest> sha1_;
public:
  void setUp()
  {
    sha1_ = MessageDigest::sha1();
  }

  void testDigest();
  void testSupports();
  void testGetDigestLength();
  void testIsStronger();
  void testIsValidHash();
  void testGetCanonicalHashType();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MessageDigestTest );

void MessageDigestTest::testDigest()
{
  sha1_->update("aria2", 5);
  CPPUNIT_ASSERT_EQUAL(std::string("f36003f22b462ffa184390533c500d8989e9f681"),
                       util::toHex(sha1_->digest()));
}

void MessageDigestTest::testSupports()
{
  CPPUNIT_ASSERT(MessageDigest::supports("sha-1"));
  // Fails because sha1 is not valid name.
  CPPUNIT_ASSERT(!MessageDigest::supports("sha1"));
}

void MessageDigestTest::testGetDigestLength()
{
  CPPUNIT_ASSERT_EQUAL((size_t)20, MessageDigest::getDigestLength("sha-1"));
  CPPUNIT_ASSERT_EQUAL((size_t)20, sha1_->getDigestLength());
}

void MessageDigestTest::testIsStronger()
{
  CPPUNIT_ASSERT(MessageDigest::isStronger("sha-1", "md5"));
  CPPUNIT_ASSERT(!MessageDigest::isStronger("md5", "sha-1"));
  CPPUNIT_ASSERT(!MessageDigest::isStronger("unknown", "sha-1"));
  CPPUNIT_ASSERT(!MessageDigest::isStronger("sha-1", "unknown"));
}

void MessageDigestTest::testIsValidHash()
{
  CPPUNIT_ASSERT(MessageDigest::isValidHash
                 ("sha-1", "f36003f22b462ffa184390533c500d8989e9f681"));
  CPPUNIT_ASSERT(!MessageDigest::isValidHash
                 ("sha-1", "f36003f22b462ffa184390533c500d89"));
}

void MessageDigestTest::testGetCanonicalHashType()
{
  CPPUNIT_ASSERT_EQUAL(std::string("sha-1"),
                       MessageDigest::getCanonicalHashType("sha1"));
  CPPUNIT_ASSERT_EQUAL(std::string("sha-256"),
                       MessageDigest::getCanonicalHashType("sha256"));
  CPPUNIT_ASSERT_EQUAL(std::string("unknown"),
                       MessageDigest::getCanonicalHashType("unknown"));
}

} // namespace aria2
