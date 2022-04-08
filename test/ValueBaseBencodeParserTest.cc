#include "ValueBaseBencodeParser.h"

#include <cppunit/extensions/HelperMacros.h>

#include "ValueBase.h"
#include "BencodeParser.h"

namespace aria2 {

class ValueBaseBencodeParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ValueBaseBencodeParserTest);
  CPPUNIT_TEST(testParseUpdate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testParseUpdate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValueBaseBencodeParserTest);

namespace {
void checkDecodeError(const std::string& src)
{
  bittorrent::ValueBaseBencodeParser parser;
  ssize_t error;
  std::shared_ptr<ValueBase> r =
      parser.parseFinal(src.c_str(), src.size(), error);
  CPPUNIT_ASSERT(!r);
  CPPUNIT_ASSERT(error < 0);
}
} // namespace

void ValueBaseBencodeParserTest::testParseUpdate()
{
  bittorrent::ValueBaseBencodeParser parser;
  ssize_t error;
  {
    // empty string
    std::string src = "0:";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL(std::string(""), downcast<String>(s)->s());
  }
  {
    // integer 0
    std::string src = "i0e";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, downcast<Integer>(s)->i());
  }
  {
    // empty dict
    std::string src = "de";
    std::shared_ptr<ValueBase> d =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT(downcast<Dict>(d)->empty());
  }
  {
    // empty list
    std::string src = "le";
    std::shared_ptr<ValueBase> l =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT(downcast<List>(l)->empty());
  }
  {
    // string
    std::string src = "3:foo";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), downcast<String>(s)->s());
  }
  {
    // integer
    std::string src = "i9223372036854775807e";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL,
                         downcast<Integer>(s)->i());
  }
  {
    // float number, ignored and always 0.
    std::string src = "i+343243.342E-1333e";
    auto s = parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, downcast<Integer>(s)->i());
  }
  {
    // dict, size 1
    std::string src = "d3:fooi123ee";
    std::shared_ptr<ValueBase> d =
        parser.parseFinal(src.c_str(), src.size(), error);
    Dict* dict = downcast<Dict>(d);
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT(dict->get("foo"));
    CPPUNIT_ASSERT_EQUAL((int64_t)123,
                         downcast<Integer>(dict->get("foo"))->i());
  }
  {
    // dict, size 2
    std::string src = "d3:fooi123e3:bar1:ee";
    std::shared_ptr<ValueBase> d =
        parser.parseFinal(src.c_str(), src.size(), error);
    Dict* dict = downcast<Dict>(d);
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT_EQUAL((size_t)2, dict->size());
    CPPUNIT_ASSERT(dict->get("foo"));
    CPPUNIT_ASSERT_EQUAL((int64_t)123,
                         downcast<Integer>(dict->get("foo"))->i());
    CPPUNIT_ASSERT(dict->get("bar"));
    CPPUNIT_ASSERT_EQUAL(std::string("e"),
                         downcast<String>(dict->get("bar"))->s());
  }
  {
    // list, size 1
    std::string src = "l3:fooe";
    std::shared_ptr<ValueBase> l =
        parser.parseFinal(src.c_str(), src.size(), error);
    List* list = downcast<List>(l);
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL((size_t)1, list->size());
    CPPUNIT_ASSERT_EQUAL(std::string("foo"),
                         downcast<String>(list->get(0))->s());
  }
  {
    // list, size 2
    std::string src = "l3:fooi123ee";
    std::shared_ptr<ValueBase> l =
        parser.parseFinal(src.c_str(), src.size(), error);
    List* list = downcast<List>(l);
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL((size_t)2, list->size());
    CPPUNIT_ASSERT_EQUAL(std::string("foo"),
                         downcast<String>(list->get(0))->s());
    CPPUNIT_ASSERT_EQUAL((int64_t)123, downcast<Integer>(list->get(1))->i());
  }
  {
    // string, integer and list in dict
    std::string src = "d4:name5:aria24:sizei12345678900e5:filesl3:bin3:docee";
    std::shared_ptr<ValueBase> r =
        parser.parseFinal(src.c_str(), src.size(), error);
    const Dict* dict = downcast<Dict>(r);
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
                         downcast<String>(dict->get("name"))->s());
    CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(12345678900LL),
                         downcast<Integer>(dict->get("size"))->i());
    const List* list = downcast<List>(dict->get("files"));
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list->size());
    CPPUNIT_ASSERT_EQUAL(std::string("bin"),
                         downcast<String>(list->get(0))->s());
    CPPUNIT_ASSERT_EQUAL(std::string("doc"),
                         downcast<String>(list->get(1))->s());
  }
  {
    // dict in list
    std::string src = "ld1:ki123eee";
    std::shared_ptr<ValueBase> r =
        parser.parseFinal(src.c_str(), src.size(), error);
    const List* list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), list->size());
    const Dict* dict = downcast<Dict>(list->get(0));
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(123),
                         downcast<Integer>(dict->get("k"))->i());
  }
  {
    // empty key is allowed
    std::string src = "d0:1:ve";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
  }
  {
    // empty encoded data
    std::string src = "";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT(!s);
  }
  // integer, without ending 'e'
  checkDecodeError("i3");
  // dict, without ending 'e'
  checkDecodeError("d");
  // list, without ending 'e'
  checkDecodeError("l");
  // string, less than the specified length.
  checkDecodeError("3:ab");
  // string, but length is invalid
  checkDecodeError("x:abc");
  // string with minus length
  checkDecodeError("-1:a");
  // too deep structure
  checkDecodeError(std::string(51, 'l') + std::string(51, 'e'));
  checkDecodeError(std::string(50, 'l') + "d3:fooi100ee" +
                   std::string(50, 'e'));
  // float number, but including bad characters
  checkDecodeError("i-1.134a+33e");
  checkDecodeError("ixe");
  // empty number
  checkDecodeError("ie");
  {
    // ignore trailing garbage at the end of the input.
    std::string src = "5:aria2trail";
    std::shared_ptr<ValueBase> s =
        parser.parseFinal(src.c_str(), src.size(), error);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), downcast<String>(s)->s());
    // Get trailing garbage position
    CPPUNIT_ASSERT_EQUAL((ssize_t)7, error);
  }
  {
    // dict, empty member name
    std::string src = "d0:i123ee";
    std::shared_ptr<ValueBase> d =
        parser.parseFinal(src.c_str(), src.size(), error);
    Dict* dict = downcast<Dict>(d);
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT(dict->get(""));
    CPPUNIT_ASSERT_EQUAL((int64_t)123, downcast<Integer>(dict->get(""))->i());
  }
}

} // namespace aria2
