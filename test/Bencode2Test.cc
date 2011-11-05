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
    std::string src = "d4:name5:aria24:sizei12345678900e5:filesl3:bin3:docee";
    SharedHandle<ValueBase> r = bencode2::decode(src.begin(), src.end());
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
    SharedHandle<ValueBase> r = bencode2::decode(src.begin(), src.end());
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
    SharedHandle<ValueBase> s = bencode2::decode(src.begin(), src.end());
  }
  {
    // empty string
    std::string src = "0:";
    SharedHandle<ValueBase> s = bencode2::decode(src.begin(), src.end());
    CPPUNIT_ASSERT_EQUAL(std::string(""), downcast<String>(s)->s());
  }
  {
    // empty dict
    std::string src = "de";
    SharedHandle<ValueBase> d = bencode2::decode(src.begin(), src.end());
    CPPUNIT_ASSERT(downcast<Dict>(d)->empty());
  }
  {
    // empty list
    std::string src = "le";
    SharedHandle<ValueBase> l = bencode2::decode(src.begin(), src.end());
    CPPUNIT_ASSERT(downcast<List>(l)->empty());
  }
  {
    // integer, without ending 'e'
    std::string src = "i3";
    try {
      bencode2::decode(src.begin(), src.end());
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Bencode decoding failed:"
                                       " Integer expected but none found"),
                           std::string(e.what()));
    }    
  }
  {
    // dict, without ending 'e'
    std::string src = "d";
    try {
      bencode2::decode(src.begin(), src.end());
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
      std::string src = "l";
      bencode2::decode(src.begin(), src.end());
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
      std::string src = "3:ab";
      bencode2::decode(src.begin(), src.end());
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
      std::string src = "x:abc";
      bencode2::decode(src.begin(), src.end());
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
    std::string src = "-1:a";
    try {
      bencode2::decode(src.begin(), src.end());
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
    std::string src = "";
    CPPUNIT_ASSERT(!bencode2::decode(src.begin(), src.end()));
  }
  {
    // ignore trailing garbage at the end of the input.
    std::string src = "5:aria2trail";
    SharedHandle<ValueBase> s = bencode2::decode(src.begin(), src.end());
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), downcast<String>(s)->s());
  }
  {
    // Get trailing garbage position
    std::string src = "5:aria2trail";
    size_t end;
    SharedHandle<ValueBase> s = bencode2::decode(src.begin(), src.end(), end);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), downcast<String>(s)->s());
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
    bencode2::decode(s.begin(), s.end());
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
