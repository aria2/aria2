#include "bencode.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Util.h"
#include "RecoverableException.h"

namespace aria2 {

class BencodeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BencodeTest);
  CPPUNIT_TEST(testString);
  CPPUNIT_TEST(testInteger);
  CPPUNIT_TEST(testDict);
  CPPUNIT_TEST(testDictIter);
  CPPUNIT_TEST(testList);
  CPPUNIT_TEST(testListIter);
  CPPUNIT_TEST(testDecode);
  CPPUNIT_TEST(testEncode);
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


CPPUNIT_TEST_SUITE_REGISTRATION( BencodeTest );

void BencodeTest::testString()
{
  bencode::BDE s(std::string("aria2"));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());

  unsigned char dataWithNull[] = { 0xf0, '\0', 0x0f };
  bencode::BDE sWithNull(dataWithNull, sizeof(dataWithNull));
  CPPUNIT_ASSERT(memcmp(dataWithNull, sWithNull.s().c_str(),
			sizeof(dataWithNull)) == 0);

  bencode::BDE zero("");
  CPPUNIT_ASSERT_EQUAL(std::string(""), zero.s());

  const unsigned char uc[] = { 0x08, 0x19, 0x2a, 0x3b };
  bencode::BDE data(uc, sizeof(uc));
  CPPUNIT_ASSERT_EQUAL(Util::toHex(uc, sizeof(uc)),
		       Util::toHex(data.uc(), data.s().size()));
}

void BencodeTest::testInteger()
{
  bencode::BDE integer(INT64_MAX);
  CPPUNIT_ASSERT_EQUAL(INT64_MAX, integer.i());
}

void BencodeTest::testDict()
{
  bencode::BDE dict = bencode::BDE::dict();
  CPPUNIT_ASSERT(dict.empty());

  dict["ki"] = 7;
  dict["ks"] = std::string("abc");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("ki"));
  CPPUNIT_ASSERT_EQUAL(static_cast<bencode::BDE::Integer>(7), dict["ki"].i());
  CPPUNIT_ASSERT(dict.containsKey("ks"));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), dict["ks"].s());

  CPPUNIT_ASSERT(dict["kn"].isNone()); // This adds kn key with default value.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), dict.size());
  CPPUNIT_ASSERT(dict.containsKey("kn"));

  const bencode::BDE& ref = dict;
  ref["kn2"]; // This doesn't add kn2 key.
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), ref.size());
  CPPUNIT_ASSERT(!ref.containsKey("kn2"));

  dict.removeKey("kn");
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), dict.size());
  CPPUNIT_ASSERT(!dict.containsKey("kn"));
}

void BencodeTest::testDictIter()
{
  bencode::BDE dict = bencode::BDE::dict();
  dict["alpha2"] = std::string("alpha2");
  dict["charlie"] = std::string("charlie");
  dict["bravo"] = std::string("bravo");
  dict["alpha"] = std::string("alpha");

  bencode::BDE::Dict::iterator i = dict.dictBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*i++).first); 
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), (*i++).first);
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), (*i++).first);
  CPPUNIT_ASSERT(dict.dictEnd() == i);

  const bencode::BDE& ref = dict;
  bencode::BDE::Dict::const_iterator ci = ref.dictBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*ci++).first);
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.dictEnd() == ci);
}

void BencodeTest::testList()
{
  bencode::BDE list = bencode::BDE::list();
  CPPUNIT_ASSERT(list.empty());
  list << 7;
  list << std::string("aria2");

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list.size());
  CPPUNIT_ASSERT_EQUAL(static_cast<bencode::BDE::Integer>(7), list[0].i());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), list[1].s());

  const bencode::BDE& ref = list;
  CPPUNIT_ASSERT_EQUAL(static_cast<bencode::BDE::Integer>(7), ref[0].i());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), ref[1].s());
}

void BencodeTest::testListIter()
{
  bencode::BDE list = bencode::BDE::list();
  list << std::string("alpha2");
  list << std::string("charlie");
  list << std::string("bravo");
  list << std::string("alpha");

  bencode::BDE::List::iterator i = list.listBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("charlie"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), (*i++).s());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), (*i++).s());
  CPPUNIT_ASSERT(list.listEnd() == i);

  const bencode::BDE& ref = list;
  bencode::BDE::List::const_iterator ci = ref.listBegin();
  CPPUNIT_ASSERT_EQUAL(std::string("alpha2"), (*ci++).s());
  std::advance(ci, 3);
  CPPUNIT_ASSERT(ref.listEnd() == ci);
}

void BencodeTest::testDecode()
{
  {
    // string, integer and list in dict
    bencode::BDE dict =
      bencode::decode("d4:name5:aria24:sizei12345678900e5:filesl3:bin3:docee");
    CPPUNIT_ASSERT(dict.isDict());
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), dict["name"].s());
    CPPUNIT_ASSERT_EQUAL(static_cast<bencode::BDE::Integer>(12345678900LL),
			 dict["size"].i());
    bencode::BDE list = dict["files"];
    CPPUNIT_ASSERT(list.isList());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), list.size());
    CPPUNIT_ASSERT_EQUAL(std::string("bin"), list[0].s());
    CPPUNIT_ASSERT_EQUAL(std::string("doc"), list[1].s());
  }
  {
    // dict in list
    bencode::BDE list = bencode::decode("ld1:ki123eee");
    CPPUNIT_ASSERT(list.isList());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), list.size());
    bencode::BDE dict = list[0];
    CPPUNIT_ASSERT(dict.isDict());
    CPPUNIT_ASSERT_EQUAL(static_cast<bencode::BDE::Integer>(123),
			 dict["k"].i());
  }
  {
    // empty key is allowed
    bencode::BDE s = bencode::decode("d0:1:ve");
  }
  {
    // empty string
    bencode::BDE s = bencode::decode("0:");
    CPPUNIT_ASSERT_EQUAL(std::string(""), s.s());
  }
  {
    // empty dict
    bencode::BDE d = bencode::decode("de");
    CPPUNIT_ASSERT(d.empty());
  }
  {
    // empty list
    bencode::BDE l = bencode::decode("le");
    CPPUNIT_ASSERT(l.empty());
  }
  {
    // integer, without ending 'e'
    try {
      bencode::decode("i3");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Delimiter 'e' not found."),
			   std::string(e.what()));
    }    
  }
  {
    // dict, without ending 'e'
    try {
      bencode::decode("d");
      CPPUNIT_FAIL("exception must be thrown.");
    } catch(RecoverableException& e) {
      CPPUNIT_ASSERT_EQUAL(std::string("Unexpected EOF in dict context."
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
      CPPUNIT_ASSERT_EQUAL(std::string("Unexpected EOF in list context."
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
      CPPUNIT_ASSERT_EQUAL(std::string("Expected 3 bytes of data,"
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
      CPPUNIT_ASSERT_EQUAL(std::string("A positive integer expected"
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
      CPPUNIT_ASSERT_EQUAL(std::string("A positive integer expected"
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
    bencode::BDE s = bencode::decode("5:aria2trail");
    CPPUNIT_ASSERT_EQUAL(std::string("aria2"), s.s());
  }
}

void BencodeTest::testEncode()
{
  {
    bencode::BDE dict = bencode::BDE::dict();
    dict["name"] = std::string("aria2");
    dict["loc"] = 80000;
    dict["files"] = bencode::BDE::list();
    dict["files"] << std::string("aria2c");
    dict["attrs"] = bencode::BDE::dict();
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
