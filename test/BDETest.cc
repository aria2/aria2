#include "BDE.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "util.h"

namespace aria2 {

class BDETest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BDETest);
  CPPUNIT_TEST(testString);
  CPPUNIT_TEST(testInteger);
  CPPUNIT_TEST(testDict);
  CPPUNIT_TEST(testDictIter);
  CPPUNIT_TEST(testList);
  CPPUNIT_TEST(testListIter);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testString();
  void testInteger();
  void testDict();
  void testDictIter();
  void testList();
  void testListIter();
  void testDecode();
  void testEncode();
};

CPPUNIT_TEST_SUITE_REGISTRATION( BDETest );

void BDETest::testString()
{
  BDE s(std::string("aria2"));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());

  unsigned char dataWithNull[] = { 0xf0, '\0', 0x0f };
  BDE sWithNull(dataWithNull, sizeof(dataWithNull));
  CPPUNIT_ASSERT(memcmp(dataWithNull, sWithNull.s().c_str(),
			sizeof(dataWithNull)) == 0);

  BDE zero("");
  CPPUNIT_ASSERT_EQUAL(std::string(""), zero.s());

  const unsigned char uc[] = { 0x08, 0x19, 0x2a, 0x3b };
  BDE data(uc, sizeof(uc));
  CPPUNIT_ASSERT_EQUAL(util::toHex(uc, sizeof(uc)),
		       util::toHex(data.uc(), data.s().size()));
}

void BDETest::testInteger()
{
  BDE integer(INT64_MAX);
  CPPUNIT_ASSERT_EQUAL(INT64_MAX, integer.i());
}

void BDETest::testDict()
{
  BDE dict = BDE::dict();
  CPPUNIT_ASSERT(dict.empty());

  dict["ki"] = 7;
  dict["ks"] = std::string("abc");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("ki"));
  CPPUNIT_ASSERT_EQUAL(static_cast<BDE::Integer>(7), dict["ki"].i());
  CPPUNIT_ASSERT(dict.containsKey("ks"));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), dict["ks"].s());

  CPPUNIT_ASSERT(dict["kn"].isNone()); // This adds kn key with default value.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("kn"));

  const BDE& ref = dict;
  ref["kn2"]; // This doesn't add kn2 key.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), ref.size());
  CPPUNIT_ASSERT(!ref.containsKey("kn2"));

  dict.removeKey("kn");
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(!dict.containsKey("kn"));
}

void BDETest::testDictIter()
{
  BDE dict = BDE::dict();
  dict["alpha2"] = std::string("alpha2");
  dict["charlie"] = std::string("charlie");
  dict["bravo"] = std::string("bravo");
  dict["alpha"] = std::string("alpha");

  BDE::Dict::iterator i = dict.dictBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*i++).first); 
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), (*i++).first);
  CPPUNIT_ASSERT(dict.dictEnd() == i);

  const BDE& ref = dict;
  BDE::Dict::const_iterator ci = ref.dictBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*ci++).first);
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.dictEnd() == ci);
}

void BDETest::testList()
{
  BDE list = BDE::list();
  CPPUNIT_ASSERT(list.empty());
  list << 7;
  list << std::string("aria2");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list.size());
  CPPUNIT_ASSERT_EQUAL(static_cast<BDE::Integer>(7), list[0].i());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), list[1].s());

  const BDE& ref = list;
  CPPUNIT_ASSERT_EQUAL(static_cast<BDE::Integer>(7), ref[0].i());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), ref[1].s());
}

void BDETest::testListIter()
{
  BDE list = BDE::list();
  list << std::string("alpha2");
  list << std::string("charlie");
  list << std::string("bravo");
  list << std::string("alpha");

  BDE::List::iterator i = list.listBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*i++).s());
  CPPUNIT_ASSERT(list.listEnd() == i);

  const BDE& ref = list;
  BDE::List::const_iterator ci = ref.listBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*ci++).s());
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.listEnd() == ci);
}

} // namespace aria2
