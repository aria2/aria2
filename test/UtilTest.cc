#include "Util.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class UtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilTest);
  CPPUNIT_TEST(testTrim);
  CPPUNIT_TEST(testSplit);
  CPPUNIT_TEST(testSlice);
  CPPUNIT_TEST(testEndsWith);
  CPPUNIT_TEST(testReplace);
  CPPUNIT_TEST(testStartsWith);
  // may be moved to other helper class in the future.
  CPPUNIT_TEST(testGetContentDispositionFilename);
  CPPUNIT_TEST(testComputeFastSet);
  CPPUNIT_TEST(testRandomAlpha);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testTrim();
  void testSplit();
  void testSlice();
  void testEndsWith();
  void testReplace();
  void testStartsWith();
  void testComputeFastSet();
  // may be moved to other helper class in the future.
  void testGetContentDispositionFilename();
  void testRandomAlpha();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UtilTest );

void UtilTest::testTrim() {
  string str1 = "aria2";
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim("aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim(" aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim(" aria2 "));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim("  aria2  "));
  string str2 = "aria2 debut";
  CPPUNIT_ASSERT_EQUAL(str2, Util::trim("aria2 debut"));
  CPPUNIT_ASSERT_EQUAL(str2, Util::trim(" aria2 debut "));
  string str3 = "";
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim(""));
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim(" "));
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim("  "));
  string str4 = "A";
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim("A"));
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim(" A "));
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim("  A  "));
}

void UtilTest::testSplit() {
  pair<string, string> p1;
  Util::split(p1, "name=value", '=');
  CPPUNIT_ASSERT_EQUAL(string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(string("value"), p1.second);
  Util::split(p1, " name = value ", '=');
  CPPUNIT_ASSERT_EQUAL(string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(string("value"), p1.second);
  Util::split(p1, "=value", '=');
  CPPUNIT_ASSERT_EQUAL(string(""), p1.first);
  CPPUNIT_ASSERT_EQUAL(string("value"), p1.second);
  Util::split(p1, "name=", '=');
  CPPUNIT_ASSERT_EQUAL(string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(string(""), p1.second);
  Util::split(p1, "name", '=');
  CPPUNIT_ASSERT_EQUAL(string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(string(""), p1.second);
}

void UtilTest::testSlice() {
  Strings v1;
  Util::slice(v1, "name1=value1; name2=value2; name3=value3;", ';');
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  v1.clear();
  Util::slice(v1, "name1=value1; name2=value2; name3=value3", ';');
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  Strings::iterator itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string("name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string("name3=value3"), *itr++);
}

void UtilTest::testEndsWith() {
  string target = "abcdefg";
  string part = "fg";
  CPPUNIT_ASSERT(Util::endsWith(target, part));

  target = "abdefg";
  part = "g";
  CPPUNIT_ASSERT(Util::endsWith(target, part));

  target = "abdefg";
  part = "eg";
  CPPUNIT_ASSERT(!Util::endsWith(target, part));

  target = "g";
  part = "eg";
  CPPUNIT_ASSERT(!Util::endsWith(target, part));

  target = "g";
  part = "g";
  CPPUNIT_ASSERT(Util::endsWith(target, part));

  target = "g";
  part = "";
  CPPUNIT_ASSERT(Util::endsWith(target, part));

  target = "";
  part = "";
  CPPUNIT_ASSERT(Util::endsWith(target, part));

  target = "";
  part = "g";
  CPPUNIT_ASSERT(!Util::endsWith(target, part));
}

void UtilTest::testReplace() {
  CPPUNIT_ASSERT_EQUAL(string("abc\n"), Util::replace("abc\r\n", "\r", ""));
  CPPUNIT_ASSERT_EQUAL(string("abc"), Util::replace("abc\r\n", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(string(""), Util::replace("", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(string("abc"), Util::replace("abc", "", "a"));
  CPPUNIT_ASSERT_EQUAL(string("xbc"), Util::replace("abc", "a", "x"));
}

void UtilTest::testStartsWith() {
  string target;
  string part;

  target = "abcdefg";
  part = "abc";
  CPPUNIT_ASSERT(Util::startsWith(target, part));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!Util::startsWith(target, part));

  target = "abcdefg";
  part = "bcd";
  CPPUNIT_ASSERT(!Util::startsWith(target, part));

  target = "";
  part = "a";
  CPPUNIT_ASSERT(!Util::startsWith(target, part));

  target = "";
  part = "";
  CPPUNIT_ASSERT(Util::startsWith(target, part));
  
  target = "a";
  part = "";
  CPPUNIT_ASSERT(Util::startsWith(target, part));

  target = "a";
  part = "a";
  CPPUNIT_ASSERT(Util::startsWith(target, part));

}

void UtilTest::testGetContentDispositionFilename() {
  string h1 = "attachment; filename=\"aria2.tar.bz2\"";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), Util::getContentDispositionFilename(h1));

  string h2 = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL(string(""), Util::getContentDispositionFilename(h2));

  string h3 = "attachment; filename=\"";
  CPPUNIT_ASSERT_EQUAL(string(""), Util::getContentDispositionFilename(h3));

  string h4 = "attachment;";
  CPPUNIT_ASSERT_EQUAL(string(""), Util::getContentDispositionFilename(h4));
}

class Printer {
public:
  template<class T>
  void operator()(T t) {
    cerr << t << ", ";
  }
};

void UtilTest::testComputeFastSet() {
  string ipaddr = "192.168.0.1";
  unsigned char infoHash[20];
  memset(infoHash, 0, sizeof(infoHash));
  infoHash[0] = 0xff;
  
  int pieces = 1000;
  int fastSetSize = 10;

  Integers fastSet = Util::computeFastSet(ipaddr, infoHash, pieces, fastSetSize);
  //for_each(fastSet.begin(), fastSet.end(), Printer());
  //cerr << endl;
  int ans1[] = { 686, 459, 278, 200, 404, 834, 64, 203, 760, 950 };
  Integers ansSet1(&ans1[0], &ans1[10]);
  CPPUNIT_ASSERT(equal(fastSet.begin(), fastSet.end(), ansSet1.begin()));

  ipaddr = "10.0.0.1";
  fastSet = Util::computeFastSet(ipaddr, infoHash, pieces, fastSetSize);
  int ans2[] = { 568, 188, 466, 452, 550, 662, 109, 226, 398, 11 };
  Integers ansSet2(&ans2[0], &ans2[10]);
  CPPUNIT_ASSERT(equal(fastSet.begin(), fastSet.end(), ansSet2.begin()));
}

void UtilTest::testRandomAlpha() {
  CPPUNIT_ASSERT_EQUAL(string("rUopvKRn"), Util::randomAlpha(8));
}
