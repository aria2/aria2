#include "GroupId.h"

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "array_fun.h"

namespace aria2 {

class GroupIdTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(GroupIdTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testToNumericId);
  CPPUNIT_TEST(testExpandUnique);
  CPPUNIT_TEST(testToHex);
  CPPUNIT_TEST(testToAbbrevHex);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { GroupId::clear(); }

  void testCreate();
  void testToNumericId();
  void testExpandUnique();
  void testToHex();
  void testToAbbrevHex();
};

CPPUNIT_TEST_SUITE_REGISTRATION(GroupIdTest);

void GroupIdTest::testCreate()
{
  std::shared_ptr<GroupId> gid = GroupId::create();
  CPPUNIT_ASSERT(gid);
  CPPUNIT_ASSERT(0 != gid->getNumericId());
  CPPUNIT_ASSERT(!GroupId::import(gid->getNumericId()));
  CPPUNIT_ASSERT(!GroupId::import(0));
}

void GroupIdTest::testToNumericId()
{
  a2_gid_t gid;
  std::string hex;
  hex = std::string(16, '0');
  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::toNumericId(gid, hex.c_str()));

  hex = std::string(16, 'f');
  CPPUNIT_ASSERT_EQUAL(0, GroupId::toNumericId(gid, hex.c_str()));
  CPPUNIT_ASSERT_EQUAL((a2_gid_t)UINT64_MAX, gid);

  CPPUNIT_ASSERT_EQUAL(0, GroupId::toNumericId(gid, "1234567890abcdef"));
  CPPUNIT_ASSERT_EQUAL((a2_gid_t)1311768467294899695LL, gid);

  hex = std::string(15, 'f');
  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::toNumericId(gid, hex.c_str()));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::toNumericId(gid, ""));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::toNumericId(gid, "1234567890abcdeg"));
}

void GroupIdTest::testExpandUnique()
{
  a2_gid_t gid;
  std::shared_ptr<GroupId> ids[] = {GroupId::import(0xff80000000010000LL),
                                    GroupId::import(0xff80000000020001LL),
                                    GroupId::import(0xfff8000000030000LL)};
  for (const auto& i : ids) {
    CPPUNIT_ASSERT(i);
  }

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_NOT_UNIQUE,
                       GroupId::expandUnique(gid, "ff8"));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::expandUnique(gid, "ffg"));

  CPPUNIT_ASSERT_EQUAL(
      (int)GroupId::ERR_INVALID,
      GroupId::expandUnique(gid, std::string(17, 'a').c_str()));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_INVALID,
                       GroupId::expandUnique(gid, ""));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_NOT_UNIQUE,
                       GroupId::expandUnique(gid, "ff800000000"));

  CPPUNIT_ASSERT_EQUAL(0, GroupId::expandUnique(gid, "ff8000000001"));
  CPPUNIT_ASSERT_EQUAL(std::string("ff80000000010000"), GroupId::toHex(gid));

  CPPUNIT_ASSERT_EQUAL(0, GroupId::expandUnique(gid, "ff8000000002"));
  CPPUNIT_ASSERT_EQUAL(std::string("ff80000000020001"), GroupId::toHex(gid));

  CPPUNIT_ASSERT_EQUAL(0, GroupId::expandUnique(gid, "fff800000003"));
  CPPUNIT_ASSERT_EQUAL(std::string("fff8000000030000"), GroupId::toHex(gid));

  CPPUNIT_ASSERT_EQUAL((int)GroupId::ERR_NOT_FOUND,
                       GroupId::expandUnique(gid, "ff80000000031"));
}

void GroupIdTest::testToHex()
{
  CPPUNIT_ASSERT_EQUAL(std::string("1234567890abcdef"),
                       GroupId::toHex(1311768467294899695LL));
  CPPUNIT_ASSERT_EQUAL(std::string("0000000000000001"),
                       GroupId::toHex(0000000000000000001LL));
}

void GroupIdTest::testToAbbrevHex()
{
  CPPUNIT_ASSERT_EQUAL(std::string("123456"),
                       GroupId::toAbbrevHex(1311768467294899695LL));
  CPPUNIT_ASSERT_EQUAL(std::string("000000"),
                       GroupId::toAbbrevHex(0000000000000000001LL));
}

} // namespace aria2
