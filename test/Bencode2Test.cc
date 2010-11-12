#include "bencode2.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"

namespace aria2 {

class Bencode2Test:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Bencode2Test);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST(testDecode_overflow);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testDecode();
  void testDecode_overflow();
  void testEncode();
};

CPPUNIT_TEST_SUITE_REGISTRATION( Bencode2Test );

void Bencode2Test::testDecode()
{
  {
    // string, integer and list in dict
    SharedHandle<ValueBase> r =
      bencode2::decode("d4:name5:aria24:sizei12345678900e5:filesl3:bin3:docee");
    const Dict* dict = asDict(r);
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
                         asString(dict->get("name"))->s());
    CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(12345678900LL),
                         asInteger(dict->get("size"))->i());
    const List* list = asList(dict->get("files"));
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list->size());
    CPPUNIT_ASSERT_EQUAL(std::string("bin"),
                         asString(list->get(0))->s());
    CPPUNIT_ASSERT_EQUAL(std::string("doc"),
                         asString(list->get(1))->s());
  }
  {
    // dict in list
    SharedHandle<ValueBase> r = bencode2::decode("ld1:ki123eee");
    const List* list = asList(r);
    CPPUNIT_ASSERT(list);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), list->size());
    const Dict* dict = asDict(list->get(0));
    CPPUNIT_ASSERT(dict);
    CPPUNIT_ASSERT_EQUAL(static_cast<Integer::ValueType>(123),
                         asInteger(dict->get("k"))->i());
  }
  {
    // empty key is allowed
    SharedHandle<ValueBase> s = bencode2::decode("d0:1:ve");
  }
  {
    // empty string
    SharedHandle<ValueBase> s = bencode2::decode("0:");
    CPPUNIT_ASSERT_EQUAL(std::string(""), asString(s)->s());
  }
  {
    // empty dict
    SharedHandle<ValueBase> d = bencode2::decode("de");
    CPPUNIT_ASSERT(asDict(d)->empty());
  }
  {
    // empty list
    SharedHandle<ValueBase> l = bencode2::decode("le");
    CPPUNIT_ASSERT(asList(l)->empty());
  }
  {
    // integer, without ending 'e'
    try {
      bencode2::decode("i3");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " Delimiter 'e' not found."),
                           std::string(e.what()));
    }    
  }
  {
    // dict, without ending 'e'
    try {
      bencode2::decode("d");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " Unexpected EOF in dict context."
                                       " 'e' expected."),
                           std::string(e.what()));
    }          
  }
  {
    // list, without ending 'e'
    try {
      bencode2::decode("l");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " Unexpected EOF in list context."
                                       " 'e' expected."),
                           std::string(e.what()));
    }          
  }
  {
    // string, less than the specified length.
    try {
      bencode2::decode("3:ab");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " Expected 3 bytes of data,"
                                       " but only 2 read."),
                           std::string(e.what()));
    }
  }
  {
    // string, but length is invalid
    try {
      bencode2::decode("x:abc");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " A positive integer expected"
                                       " but none found."),
                           std::string(e.what()));
    }
  }
  {
    // string with minus length
    try {
      bencode2::decode("-1:a");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " A positive integer expected"
                                       " but none found."),
                           std::string(e.what()));
    }
  }
  {
    // empty encoded data
    CPPUNIT_ASSERT(!bencode2::decode(""));
  }
  {
    // ignore trailing garbage at the end of the input.
    SharedHandle<ValueBase> s = bencode2::decode("5:aria2trail");
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), asString(s)->s());
  }
  {
    // Get trailing garbage position
    size_t end;
    SharedHandle<ValueBase> s = bencode2::decode("5:aria2trail", end);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), asString(s)->s());
    CPPUNIT_ASSERT_EQUAL((size_t)7, end);
  }
}

void Bencode2Test::testDecode_overflow()
{
  std::string s;
  size_t depth = bencode2::MAX_STRUCTURE_DEPTH+1;
  for(size_t i = 0; i < depth; ++i) {
    s += "l";
  }
  for(size_t i = 0; i < depth; ++i) {
    s += "e";
  }
  try {
    bencode2::decode(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

void Bencode2Test::testEncode()
{
  {
    Dict dict;
    dict["name"] = String::g("aria2");
    dict["loc"] = Integer::g(80000);
    SharedHandle<List> files = List::g();
    files->append(String::g("aria2c"));
    dict["files"] = files;
    SharedHandle<Dict> attrs = Dict::g();
    attrs->put("license", String::g("GPL"));
    dict["attrs"] = attrs;

    CPPUNIT_ASSERT_EQUAL(std::string("d"
                                     "5:attrsd7:license3:GPLe"
                                     "5:filesl6:aria2ce"
                                     "3:loci80000e"
                                     "4:name5:aria2"
                                     "e"),
                         bencode2::encode(&dict));
  }
}

} // namespace aria2
