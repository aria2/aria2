#include "BencodeVisitor.h"
#include "Data.h"
#include "List.h"
#include "Dictionary.h"
#include <cppunit/extensions/HelperMacros.h>

class BencodeVisitorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BencodeVisitorTest);
  CPPUNIT_TEST(testVisit_data);
  CPPUNIT_TEST(testVisit_list);
  CPPUNIT_TEST(testVisit_dictionary);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testVisit_data();
  void testVisit_list();
  void testVisit_dictionary();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BencodeVisitorTest );

void BencodeVisitorTest::testVisit_data()
{
  {
    BencodeVisitor v;
    string str = "apple";
    MetaEntryHandle m = new Data(str.c_str(), str.size());
    m->accept(&v);
    CPPUNIT_ASSERT_EQUAL(string("5:apple"), v.getBencodedData());
  }
  {
    BencodeVisitor v;
    string str = "123";
    MetaEntryHandle m = new Data(str.c_str(), str.size(), true);
    m->accept(&v);
    CPPUNIT_ASSERT_EQUAL(string("i123e"), v.getBencodedData());
  }
}

void BencodeVisitorTest::testVisit_list()
{
  BencodeVisitor v;
  List l;
  string s1 = "alpha";
  l.add(new Data(s1.c_str(), s1.size()));
  string s2 = "bravo";
  l.add(new Data(s2.c_str(), s2.size()));
  string s3 = "123";
  l.add(new Data(s3.c_str(), s3.size(), true));
  l.accept(&v);
  CPPUNIT_ASSERT_EQUAL(string("l5:alpha5:bravoi123ee"), v.getBencodedData());
}

void BencodeVisitorTest::testVisit_dictionary()
{
  BencodeVisitor v;
  Dictionary d;
  string s1 = "alpha";
  d.put("team", new Data(s1.c_str(), s1.size()));
  string s2 = "123";
  d.put("score", new Data(s2.c_str(), s2.size(), true));
  d.accept(&v);
  CPPUNIT_ASSERT_EQUAL(string("d4:team5:alpha5:scorei123ee"), v.getBencodedData());
}
