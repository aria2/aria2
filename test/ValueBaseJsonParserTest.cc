#include "ValueBaseJsonParser.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "array_fun.h"
#include "ValueBase.h"

namespace aria2 {

class ValueBaseJsonParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ValueBaseJsonParserTest);
  CPPUNIT_TEST(testParseUpdate);
  CPPUNIT_TEST(testParseUpdate_error);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testParseUpdate();
  void testParseUpdate_error();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ValueBaseJsonParserTest);

void ValueBaseJsonParserTest::testParseUpdate()
{
  json::ValueBaseJsonParser parser;
  ssize_t error;
  {
    // empty object
    std::string src = "{}";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto dict = downcast<Dict>(r);
    CPPUNIT_ASSERT(dict);
  }
  {
    // empty object
    std::string src = "{  }";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto dict = downcast<Dict>(r);
    CPPUNIT_ASSERT(dict);
  }
  {
    // empty array
    std::string src = "[]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
  }
  {
    // empty array
    std::string src = "[ ]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
  }
  {
    // empty string
    std::string src = "[\"\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string(), s->s());
  }
  {
    // string
    std::string src = "[\"foobar\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("foobar"), s->s());
  }
  {
    // string with escape
    std::string src = "[\"\\\\foo\\\"\\\"bar\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("\\foo\"\"bar"), s->s());
  }
  {
    // string with escape
    std::string src = "[\"foo\\\"\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("foo\""), s->s());
  }
  {
    // string: utf-8 1 to 3 bytes.
    std::string src = "[\"\\u0024\\u00A2\\u20AC\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("$¢€"), s->s());
  }
  {
    // string: utf-8 4 bytes
    std::string src = "[\"\\uD852\\uDF62\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    const unsigned char arr[] = {0xF0u, 0xA4u, 0xADu, 0xA2u};
    CPPUNIT_ASSERT_EQUAL(std::string(std::begin(arr), std::end(arr)), s->s());
  }
  {
    // null
    std::string src = "[null]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    const Null* s = downcast<Null>(list->get(0));
    CPPUNIT_ASSERT(s);
  }
  {
    // true, false
    std::string src = "[true, false]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    const Bool* trueValue = downcast<Bool>(list->get(0));
    CPPUNIT_ASSERT(trueValue);
    CPPUNIT_ASSERT(trueValue->val());
    const Bool* falseValue = downcast<Bool>(list->get(1));
    CPPUNIT_ASSERT(falseValue);
    CPPUNIT_ASSERT(!falseValue->val());
  }
  {
    // object: 1 member
    std::string src = "{\"foo\":[\"bar\"]}";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    const Dict* dict = downcast<Dict>(r);
    CPPUNIT_ASSERT(dict);
    auto list = downcast<List>(dict->get("foo"));
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), s->s());
  }
  {
    // object: 2 members
    // TODO ValueBaseJsonParser does not allow empty dict key
    std::string src = "{\"foo\":[\"bar\"], \"alpha\" : \"bravo\"}";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    const Dict* dict = downcast<Dict>(r);
    CPPUNIT_ASSERT(dict);
    auto list = downcast<List>(dict->get("foo"));
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("bar"), s->s());
    auto str = downcast<String>(dict->get("alpha"));
    CPPUNIT_ASSERT_EQUAL(std::string("bravo"), str->s());
  }
  {
    // array: 2 values
    std::string src = "[\"foo\", {}]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), s->s());
    const Dict* dict = downcast<Dict>(list->get(1));
    CPPUNIT_ASSERT(dict);
  }
  {
    // Number: currently we ignore frac and exp
    std::string src = "[0,-1,1.2,-1.2e-10,-1e10]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    const Integer* i = downcast<Integer>(list->get(0));
    CPPUNIT_ASSERT_EQUAL((Integer::ValueType)0, i->i());
    const Integer* i1 = downcast<Integer>(list->get(1));
    CPPUNIT_ASSERT_EQUAL((Integer::ValueType)-1, i1->i());
    const Integer* i2 = downcast<Integer>(list->get(2));
    CPPUNIT_ASSERT_EQUAL((Integer::ValueType)1, i2->i());
    const Integer* i3 = downcast<Integer>(list->get(3));
    CPPUNIT_ASSERT_EQUAL((Integer::ValueType)-1, i3->i());
    const Integer* i4 = downcast<Integer>(list->get(4));
    CPPUNIT_ASSERT_EQUAL((Integer::ValueType)-1, i4->i());
  }
  {
    // escape chars: ", \, /, \b, \f, \n, \r, \t
    std::string src = "[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("\"\\/\b\f\n\r\t"), s->s());
  }
  {
    // string: literal + escaped chars.
    std::string src = "[\"foo\\u0024b\\u00A2\\u20ACbaz\"]";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    auto s = downcast<String>(list->get(0));
    CPPUNIT_ASSERT_EQUAL(std::string("foo$b¢€baz"), s->s());
  }
  {
    // ignore garbage at the end of the input.
    std::string src = "[]trail";
    auto r = parser.parseFinal(src.c_str(), src.size(), error);
    auto list = downcast<List>(r);
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL((ssize_t)2, error);
  }
}

namespace {
void checkDecodeError(const std::string& src)
{
  json::ValueBaseJsonParser parser;
  ssize_t error;
  auto r = parser.parseFinal(src.c_str(), src.size(), error);
  CPPUNIT_ASSERT(!r);
  CPPUNIT_ASSERT(error < 0);
}
} // namespace

void ValueBaseJsonParserTest::testParseUpdate_error()
{
  // object
  checkDecodeError("{");
  // object
  checkDecodeError("}");
  // object
  checkDecodeError("{\"\":");
  // object
  checkDecodeError("{\"\":\"\",");
  // array
  checkDecodeError("[");
  // array
  checkDecodeError("]");
  // array
  checkDecodeError("[\"\"");
  // array
  checkDecodeError("[\"\",");
  // string
  checkDecodeError("[\"foo]");
  // string
  checkDecodeError("[\"\\u\"]");
  // string
  checkDecodeError("[\"\\u");
  // string
  checkDecodeError("[\"\\u000\"]");
  // string
  checkDecodeError("[\"\\u000");
  // string
  checkDecodeError("[\"\\uD852foo\"]");
  // string
  checkDecodeError("[\"\\uD852");
  // string
  checkDecodeError("[\"\\uD852\\u\"]");
  // string
  checkDecodeError("[\"\\uD852\\u");
  // string
  checkDecodeError("[\"\\uD852\\u0000\"]");
  // string
  checkDecodeError("[\"\\uD852\\uDF62");
  // object
  checkDecodeError("{:\"\"}");
  // object
  checkDecodeError("{\"foo\":}");
  // number
  // TODO ValueBaseJsonParser allows leading zeros
  // checkDecodeError("[00]");
  // number
  checkDecodeError("[1.]");
  // number
  checkDecodeError("[1.1e]");
  // bool
  checkDecodeError("[t");
  // too deep structure
  checkDecodeError(std::string(51, '[') + std::string(51, ']'));
  checkDecodeError(std::string(50, '[') + "{\"foo\":100}" +
                   std::string(50, ']'));
}

} // namespace aria2
