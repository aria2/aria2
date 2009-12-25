#include "bencode.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"

namespace aria2 {

class BencodeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BencodeTest);
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

CPPUNIT_TEST_SUITE_REGISTRATION( BencodeTest );

void BencodeTest::testDecode()
{
  {
    // string, integer and list in dict
    BDE dict =
      bencode::decode("d4:name5:aria24:sizei12345678900e5:filesl3:bin3:docee");
    CPPUNIT_ASSERT(dict.isDict());
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), dict["name"].s());
    CPPUNIT_ASSERT_EQUAL(static_cast<BDE::Integer>(12345678900LL),
			 dict["size"].i());
    BDE list = dict["files"];
    CPPUNIT_ASSERT(list.isList());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list.size());
    CPPUNIT_ASSERT_EQUAL(std::string("bin"), list[0].s());
    CPPUNIT_ASSERT_EQUAL(std::string("doc"), list[1].s());
  }
  {
    // dict in list
    BDE list = bencode::decode("ld1:ki123eee");
    CPPUNIT_ASSERT(list.isList());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), list.size());
    BDE dict = list[0];
    CPPUNIT_ASSERT(dict.isDict());
    CPPUNIT_ASSERT_EQUAL(static_cast<BDE::Integer>(123),
			 dict["k"].i());
  }
  {
    // empty key is allowed
    BDE s = bencode::decode("d0:1:ve");
  }
  {
    // empty string
    BDE s = bencode::decode("0:");
    CPPUNIT_ASSERT_EQUAL(std::string(""), s.s());
  }
  {
    // empty dict
    BDE d = bencode::decode("de");
    CPPUNIT_ASSERT(d.empty());
  }
  {
    // empty list
    BDE l = bencode::decode("le");
    CPPUNIT_ASSERT(l.empty());
  }
  {
    // integer, without ending 'e'
    try {
      bencode::decode("i3");
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
      bencode::decode("d");
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
      bencode::decode("l");
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
      bencode::decode("3:ab");
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
      bencode::decode("x:abc");
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
      bencode::decode("-1:a");
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
    CPPUNIT_ASSERT(bencode::decode("").isNone());
  }
  {
    // ignore trailing garbage at the end of the input.
    BDE s = bencode::decode("5:aria2trail");
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());
  }
  {
    // Get trailing garbage position
    size_t end;
    BDE s = bencode::decode("5:aria2trail", end);
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());
    CPPUNIT_ASSERT_EQUAL((size_t)7, end);
  }
}

void BencodeTest::testDecode_overflow()
{
  std::string s;
  size_t depth = bencode::MAX_STRUCTURE_DEPTH+1;
  for(size_t i = 0; i < depth; ++i) {
    s += "l";
  }
  for(size_t i = 0; i < depth; ++i) {
    s += "e";
  }
  try {
    bencode::decode(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

void BencodeTest::testEncode()
{
  {
    BDE dict = BDE::dict();
    dict["name"] = std::string("aria2");
    dict["loc"] = 80000;
    dict["files"] = BDE::list();
    dict["files"] << std::string("aria2c");
    dict["attrs"] = BDE::dict();
    dict["attrs"]["license"] = std::string("GPL");

    CPPUNIT_ASSERT_EQUAL(std::string("d"
				     "5:attrsd7:license3:GPLe"
				     "5:filesl6:aria2ce"
				     "3:loci80000e"
				     "4:name5:aria2"
				     "e"),
			 bencode::encode(dict));
  }
}

} // namespace aria2
