#include "ValueBase.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class ValueBaseTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ValueBaseTest);
  CPPUNIT_TEST(testString);
  CPPUNIT_TEST(testDict);
  CPPUNIT_TEST(testDictIter);
  CPPUNIT_TEST(testList);
  CPPUNIT_TEST(testListIter);
  CPPUNIT_TEST(testDowncast);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testString();
  void testDict();
  void testDictIter();
  void testList();
  void testListIter();
  void testDowncast();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ValueBaseTest);

void ValueBaseTest::testString()
{
  String s(std::string("aria2"));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());

  unsigned char dataWithNull[] = { 0xf0, '\0', 0x0f };
  String sWithNull(dataWithNull, sizeof(dataWithNull));
  CPPUNIT_ASSERT(memcmp(dataWithNull, sWithNull.s().c_str(),
                        sizeof(dataWithNull)) == 0);

  String zero("");
  CPPUNIT_ASSERT_EQUAL(std::string(""), zero.s());

  String z;
  CPPUNIT_ASSERT_EQUAL(std::string(""), z.s());

  const unsigned char uc[] = { 0x08, 0x19, 0x2a, 0x3b };
  String data(uc, sizeof(uc));
  CPPUNIT_ASSERT_EQUAL(util::toHex(uc, sizeof(uc)),
                       util::toHex(data.uc(), data.s().size()));
}

void ValueBaseTest::testDowncast()
{
  Integer integer(100);
  const Integer* x = downcast<Integer>(&integer);
  CPPUNIT_ASSERT(x);
  CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(100), x->i());
  CPPUNIT_ASSERT(!downcast<String>(&integer));
  SharedHandle<Integer> si(new Integer(101));
  const Integer* x2 = downcast<Integer>(si);
  CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(101), x2->i());

  String str("foo");
  const String* x3 = downcast<String>(&str);
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("foo"), x3->s());

  List list;
  const List* x4 = downcast<List>(&list);
  CPPUNIT_ASSERT(x4);

  Dict dict;
  const Dict* x5 = downcast<Dict>(&dict);
  CPPUNIT_ASSERT(x5);
}

void ValueBaseTest::testDict()
{
  Dict dict;
  CPPUNIT_ASSERT(dict.empty());

  dict["ki"] = Integer::g(7);
  dict["ks"] = String::g("abc");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("ki"));
  CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(7),
                       downcast<Integer>(dict["ki"])->i());
  CPPUNIT_ASSERT(dict.containsKey("ks"));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"),
                       downcast<String>(dict["ks"])->s());

  CPPUNIT_ASSERT(!dict["kn"]); // This adds kn key with default value.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("kn"));

  const Dict& ref = dict;
  ref["kn2"]; // This doesn't add kn2 key.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), ref.size());
  CPPUNIT_ASSERT(!ref.containsKey("kn2"));

  dict.removeKey("kn");
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(!dict.containsKey("kn"));
}

void ValueBaseTest::testDictIter()
{
  Dict dict;
  dict["alpha2"] = String::g("alpha2");
  dict["charlie"] = String::g("charlie");
  dict["bravo"] = String::g("bravo");
  dict["alpha"] = String::g("alpha");

  Dict::ValueType::iterator i = dict.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*i++).first); 
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), (*i++).first);
  CPPUNIT_ASSERT(dict.end() == i);

  const Dict& ref = dict;
  Dict::ValueType::const_iterator ci = ref.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*ci++).first);
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.end() == ci);
}

void ValueBaseTest::testList()
{
  List list;
  CPPUNIT_ASSERT(list.empty());
  list << Integer::g(7) << String::g("aria2");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list.size());
  CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(7),
                       downcast<Integer>(list[0])->i());
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("aria2"),
                       downcast<String>(list[1])->s());

  const List& ref = list;
  CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(7),
                       downcast<Integer>(ref[0])->i());
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("aria2"),
                       downcast<String>(ref[1])->s());
}

void ValueBaseTest::testListIter()
{
  List list;
  list << String::g("alpha2")
       << String::g("charlie")
       << String::g("bravo")
       << String::g("alpha");

  List::ValueType::iterator i = list.begin();
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("alpha2"),
                       downcast<String>(*i++)->s());
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("charlie"),
                       downcast<String>(*i++)->s());
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("bravo"),
                       downcast<String>(*i++)->s());
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("alpha"),
                       downcast<String>(*i++)->s());
  CPPUNIT_ASSERT(list.end() == i);

  const List& ref = list;
  List::ValueType::const_iterator ci = ref.begin();
  CPPUNIT_ASSERT_EQUAL(static_cast<String::ValueType>("alpha2"),
                       downcast<String>(*ci++)->s());
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.end() == ci);
}

} // namespace aria2
