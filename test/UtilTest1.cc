#include "util.h"

#include <cmath>
#include <cstring>
#include <string>
#include <cassert>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "FixedNumberRandomizer.h"
#include "DlAbortEx.h"
#include "BitfieldMan.h"
#include "ByteArrayDiskWriter.h"
#include "FileEntry.h"
#include "File.h"
#include "array_fun.h"
#include "BufferedFile.h"
#include "TestUtil.h"
#include "SocketCore.h"

namespace aria2 {

class UtilTest1 : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilTest1);
  CPPUNIT_TEST(testStrip);
  CPPUNIT_TEST(testStripIter);
  CPPUNIT_TEST(testLstripIter);
  CPPUNIT_TEST(testLstripIter_char);
  CPPUNIT_TEST(testDivide);
  CPPUNIT_TEST(testSplit);
  CPPUNIT_TEST(testSplitIter);
  CPPUNIT_TEST(testSplitIterM);
  CPPUNIT_TEST(testStreq);
  CPPUNIT_TEST(testStrieq);
  CPPUNIT_TEST(testStrifind);
  CPPUNIT_TEST(testEndsWith);
  CPPUNIT_TEST(testIendsWith);
  CPPUNIT_TEST(testReplace);
  CPPUNIT_TEST(testStartsWith);
  CPPUNIT_TEST(testIstartsWith);
  // may be moved to other helper class in the future.
  CPPUNIT_TEST(testGetContentDispositionFilename);
  CPPUNIT_TEST(testParseContentDisposition1);
  CPPUNIT_TEST(testParseContentDisposition2);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testStrip();
  void testStripIter();
  void testLstripIter();
  void testLstripIter_char();
  void testDivide();
  void testSplit();
  void testSplitIter();
  void testSplitIterM();
  void testStreq();
  void testStrieq();
  void testStrifind();
  void testEndsWith();
  void testIendsWith();
  void testReplace();
  void testStartsWith();
  void testIstartsWith();
  // may be moved to other helper class in the future.
  void testGetContentDispositionFilename();
  void testParseContentDisposition1();
  void testParseContentDisposition2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTest1);

void UtilTest1::testStrip()
{
  std::string str1 = "aria2";
  CPPUNIT_ASSERT_EQUAL(str1, util::strip("aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, util::strip(" aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, util::strip("aria2 "));
  CPPUNIT_ASSERT_EQUAL(str1, util::strip(" aria2 "));
  CPPUNIT_ASSERT_EQUAL(str1, util::strip("  aria2  "));
  std::string str2 = "aria2 debut";
  CPPUNIT_ASSERT_EQUAL(str2, util::strip("aria2 debut"));
  CPPUNIT_ASSERT_EQUAL(str2, util::strip(" aria2 debut "));
  std::string str3 = "";
  CPPUNIT_ASSERT_EQUAL(str3, util::strip(""));
  CPPUNIT_ASSERT_EQUAL(str3, util::strip(" "));
  CPPUNIT_ASSERT_EQUAL(str3, util::strip("  "));
  std::string str4 = "A";
  CPPUNIT_ASSERT_EQUAL(str4, util::strip("A"));
  CPPUNIT_ASSERT_EQUAL(str4, util::strip(" A "));
  CPPUNIT_ASSERT_EQUAL(str4, util::strip("  A  "));
}

void UtilTest1::testStripIter()
{
  Scip p;
  std::string str1 = "aria2";
  std::string s = "aria2";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str1, std::string(p.first, p.second));
  s = " aria2";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str1, std::string(p.first, p.second));
  s = "aria2 ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str1, std::string(p.first, p.second));
  s = " aria2 ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str1, std::string(p.first, p.second));
  s = "  aria2  ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str1, std::string(p.first, p.second));
  std::string str2 = "aria2 debut";
  s = "aria2 debut";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str2, std::string(p.first, p.second));
  s = " aria2 debut ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str2, std::string(p.first, p.second));
  std::string str3 = "";
  s = "";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str3, std::string(p.first, p.second));
  s = " ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str3, std::string(p.first, p.second));
  s = "  ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str3, std::string(p.first, p.second));
  std::string str4 = "A";
  s = "A";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str4, std::string(p.first, p.second));
  s = " A ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str4, std::string(p.first, p.second));
  s = "  A  ";
  p = util::stripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(str4, std::string(p.first, p.second));
}

void UtilTest1::testLstripIter()
{
  std::string::iterator r;
  std::string s = "foo";
  r = util::lstripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), std::string(r, s.end()));

  s = "  foo bar  ";
  r = util::lstripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("foo bar  "), std::string(r, s.end()));

  s = "f";
  r = util::lstripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("f"), std::string(r, s.end()));

  s = "foo  ";
  r = util::lstripIter(s.begin(), s.end());
  CPPUNIT_ASSERT_EQUAL(std::string("foo  "), std::string(r, s.end()));
}

void UtilTest1::testLstripIter_char()
{
  std::string::iterator r;
  std::string s = "foo";
  r = util::lstripIter(s.begin(), s.end(), '$');
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), std::string(r, s.end()));

  s = "$$foo$bar$$";
  r = util::lstripIter(s.begin(), s.end(), '$');
  CPPUNIT_ASSERT_EQUAL(std::string("foo$bar$$"), std::string(r, s.end()));

  s = "f";
  r = util::lstripIter(s.begin(), s.end(), '$');
  CPPUNIT_ASSERT_EQUAL(std::string("f"), std::string(r, s.end()));

  s = "foo$$";
  r = util::lstripIter(s.begin(), s.end(), '$');
  CPPUNIT_ASSERT_EQUAL(std::string("foo$$"), std::string(r, s.end()));
}

void UtilTest1::testDivide()
{
  std::string s = "name=value";
  auto p1 = util::divide(std::begin(s), std::end(s), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = " name = value ";
  p1 = util::divide(std::begin(s), std::end(s), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = "=value";
  p1 = util::divide(std::begin(s), std::end(s), '=');
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = "name=";
  p1 = util::divide(std::begin(s), std::end(s), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.second.first, p1.second.second));
  s = "name";
  p1 = util::divide(std::begin(s), std::end(s), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.second.first, p1.second.second));
}

void UtilTest1::testSplit()
{
  std::vector<std::string> v;
  std::string s = "k1; k2;; k3";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', true);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  std::vector<std::string>::iterator itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("k3"), *itr++);

  v.clear();

  s = "k1; k2; k3";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" k2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" k3"), *itr++);

  v.clear();

  s = "k=v";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("k=v"), *itr++);

  v.clear();

  s = ";;k1;;k2;";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)6, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);

  v.clear();

  s = ";;k1;;k2;";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), *itr++);

  v.clear();

  s = "k; ";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("k"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" "), *itr++);

  v.clear();

  s = " ";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', true, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), v[0]);

  v.clear();

  s = " ";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', true);
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = " ";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(" "), v[0]);

  v.clear();

  s = ";";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = ";";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  itr = v.begin();
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(""), *itr++);

  v.clear();

  s = "";
  util::split(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), v[0]);
}

void UtilTest1::testSplitIter()
{
  std::vector<Scip> v;
  std::string s = "k1; k2;; k3";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', true);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k3"), std::string(v[2].first, v[2].second));

  v.clear();

  s = "k1; k2; k3";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" k2"),
                       std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" k3"),
                       std::string(v[2].first, v[2].second));

  v.clear();

  s = "k=v";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k=v"),
                       std::string(v[0].first, v[0].second));

  v.clear();

  s = ";;k1;;k2;";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)6, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[2].first, v[2].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[3].first, v[3].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[4].first, v[4].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[5].first, v[5].second));

  v.clear();

  s = ";;k1;;k2;";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[1].first, v[1].second));

  v.clear();

  s = "k; ";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" "), std::string(v[1].first, v[1].second));

  v.clear();

  s = " ";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', true, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));

  v.clear();

  s = " ";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', true);
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = " ";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(" "), std::string(v[0].first, v[0].second));

  v.clear();

  s = ";";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';');
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = ";";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[1].first, v[1].second));

  v.clear();

  s = "";
  util::splitIter(s.begin(), s.end(), std::back_inserter(v), ';', false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
}

void UtilTest1::testSplitIterM()
{
  const char d[] = ";";
  const char md[] = "; ";
  std::vector<Scip> v;
  std::string s = "k1; k2;; k3";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, true);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k3"), std::string(v[2].first, v[2].second));

  v.clear();

  s = "k1; k2; k3";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" k2"),
                       std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" k3"),
                       std::string(v[2].first, v[2].second));

  v.clear();

  s = "k1; k2; k3";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), md);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k3"), std::string(v[2].first, v[2].second));

  v.clear();

  s = "k1; k2; k3;";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), md, false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)6, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[2].first, v[2].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[3].first, v[3].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k3"), std::string(v[4].first, v[4].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[5].first, v[5].second));

  v.clear();

  s = "k=v";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k=v"),
                       std::string(v[0].first, v[0].second));

  v.clear();

  s = ";;k1;;k2;";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)6, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[1].first, v[1].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[2].first, v[2].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[3].first, v[3].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[4].first, v[4].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[5].first, v[5].second));

  v.clear();

  s = ";;k1;;k2;";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d);
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k1"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string("k2"), std::string(v[1].first, v[1].second));

  v.clear();

  s = "k; ";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d);
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k"), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(" "), std::string(v[1].first, v[1].second));

  v.clear();

  s = " ";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, true, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));

  v.clear();

  s = " ";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, true);
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = " ";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(" "), std::string(v[0].first, v[0].second));

  v.clear();

  s = ";";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d);
  CPPUNIT_ASSERT_EQUAL((size_t)0, v.size());

  v.clear();

  s = ";";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)2, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[1].first, v[1].second));

  v.clear();

  s = "";
  util::splitIterM(s.begin(), s.end(), std::back_inserter(v), d, false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), std::string(v[0].first, v[0].second));
}

void UtilTest1::testEndsWith()
{
  std::string target = "abcdefg";
  std::string part = "fg";
  CPPUNIT_ASSERT(
      util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "abdefg";
  part = "g";
  CPPUNIT_ASSERT(
      util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "abdefg";
  part = "eg";
  CPPUNIT_ASSERT(
      !util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "g";
  part = "eg";
  CPPUNIT_ASSERT(
      !util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "g";
  part = "g";
  CPPUNIT_ASSERT(
      util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "g";
  part = "";
  CPPUNIT_ASSERT(
      util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "";
  part = "";
  CPPUNIT_ASSERT(
      util::endsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "";
  part = "g";
  CPPUNIT_ASSERT(
      !util::endsWith(target.begin(), target.end(), part.begin(), part.end()));
}

void UtilTest1::testIendsWith()
{
  std::string target = "abcdefg";
  std::string part = "Fg";
  CPPUNIT_ASSERT(
      util::iendsWith(target.begin(), target.end(), part.begin(), part.end()));

  target = "abdefg";
  part = "ef";
  CPPUNIT_ASSERT(
      !util::iendsWith(target.begin(), target.end(), part.begin(), part.end()));
}

void UtilTest1::testStreq()
{
  std::string s1, s2;
  s1 = "foo";
  s2 = "foo";
  CPPUNIT_ASSERT(util::streq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(util::streq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "fooo";
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "fo";
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "";
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::streq(s1.begin(), s1.end(), s2.c_str()));

  s1 = "";
  CPPUNIT_ASSERT(util::streq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(util::streq(s1.begin(), s1.end(), s2.c_str()));
}

void UtilTest1::testStrieq()
{
  std::string s1, s2;
  s1 = "foo";
  s2 = "foo";
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.c_str()));

  s1 = "FoO";
  s2 = "fOo";
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "fooo";
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "fo";
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.c_str()));

  s2 = "";
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(!util::strieq(s1.begin(), s1.end(), s2.c_str()));

  s1 = "";
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.begin(), s2.end()));
  CPPUNIT_ASSERT(util::strieq(s1.begin(), s1.end(), s2.c_str()));
}

void UtilTest1::testStrifind()
{
  std::string s1, s2;
  s1 = "yamagakani mukashi wo toheba hARU no tuki";
  s2 = "HaRu";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end()) !=
                 s1.end());
  s2 = "aki";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end()) ==
                 s1.end());
  s1 = "h";
  s2 = "HH";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end()) ==
                 s1.end());
}

void UtilTest1::testReplace()
{
  CPPUNIT_ASSERT_EQUAL(std::string("abc\n"),
                       util::replace("abc\r\n", "\r", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"),
                       util::replace("abc\r\n", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::replace("", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), util::replace("abc", "", "a"));
  CPPUNIT_ASSERT_EQUAL(std::string("xbc"), util::replace("abc", "a", "x"));
}

void UtilTest1::testStartsWith()
{
  std::string target;
  std::string part;

  target = "abcdefg";
  part = "abc";
  CPPUNIT_ASSERT(
      util::startsWith(target.begin(), target.end(), part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.begin(),
                                   part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "bcd";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.begin(),
                                   part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "";
  part = "a";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.begin(),
                                   part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "";
  part = "";
  CPPUNIT_ASSERT(
      util::startsWith(target.begin(), target.end(), part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "a";
  part = "";
  CPPUNIT_ASSERT(
      util::startsWith(target.begin(), target.end(), part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "a";
  part = "a";
  CPPUNIT_ASSERT(
      util::startsWith(target.begin(), target.end(), part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));
}

void UtilTest1::testIstartsWith()
{
  std::string target;
  std::string part;

  target = "abcdefg";
  part = "aBc";
  CPPUNIT_ASSERT(util::istartsWith(target.begin(), target.end(), part.begin(),
                                   part.end()));
  CPPUNIT_ASSERT(util::istartsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!util::istartsWith(target.begin(), target.end(), part.begin(),
                                    part.end()));
  CPPUNIT_ASSERT(
      !util::istartsWith(target.begin(), target.end(), part.c_str()));
}

void UtilTest1::testGetContentDispositionFilename()
{
  std::string val;

  val = "attachment; filename=\"aria2.tar.bz2\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename= \" aria2.tar.bz2 \"";
  CPPUNIT_ASSERT_EQUAL(std::string(" aria2.tar.bz2 "),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=dir/file";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=dir\\file";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"dir/file\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"dir\\\\file\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"/etc/passwd\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=\"..\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename=..";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, false));

  // Unescaping %2E%2E%2F produces "../". But since we won't unescape,
  // we just accept it as is.
  val = "attachment; filename=\"%2E%2E%2Ffoo.html\"";
  CPPUNIT_ASSERT_EQUAL(std::string("%2E%2E%2Ffoo.html"),
                       util::getContentDispositionFilename(val, false));

  // iso-8859-1 string will be converted to utf-8.
  val = "attachment; filename*=iso-8859-1''foo-%E4.html";
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::getContentDispositionFilename(val, false));

  val = "attachment; filename*= UTF-8''foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::getContentDispositionFilename(val, false));

  // iso-8859-1 string will be converted to utf-8.
  val = "attachment; filename=\"foo-%E4.html\"";
  val = util::percentDecode(val.begin(), val.end());
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::getContentDispositionFilename(val, false));

  // allow utf-8 in filename if default_utf8 is set.
  val = "attachment; filename=\"foo-ä.html\"";
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::getContentDispositionFilename(val, true));

  // return empty if default_utf8 is set but invalid utf8.
  val = "attachment; filename=\"foo-\xc2\x02.html\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(val, true));
}

void UtilTest1::testParseContentDisposition1()
{
  char dest[1_k];
  size_t destlen = sizeof(dest);
  const char* cs;
  size_t cslen;
  std::string val;

  // test cases from http://greenbytes.de/tech/tc2231/
  // inlonly
  val = "inline";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // inlonlyquoted
  val = "\"inline\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // inlwithasciifilename
  val = "inline; filename=\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // inlwithfnattach
  val = "inline; filename=\"Not an attachment!\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)18, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("Not an attachment!"),
                       std::string(&dest[0], &dest[18]));

  // inlwithasciifilenamepdf
  val = "inline; filename=\"foo.pdf\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)7, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.pdf"), std::string(&dest[0], &dest[7]));

  // attwithasciifilename25
  val = "attachment; filename=\"0000000000111111111122222\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)25, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("0000000000111111111122222"),
                       std::string(&dest[0], &dest[25]));

  // attwithasciifilename35
  val = "attachment; filename=\"00000000001111111111222222222233333\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)35, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("00000000001111111111222222222233333"),
                       std::string(&dest[0], &dest[35]));

  // attwithasciifnescapedchar
  val = "attachment; filename=\"f\\oo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithasciifnescapedquote
  val = "attachment; filename=\"\\\"quoting\\\" tested.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)21, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("\"quoting\" tested.html"),
                       std::string(&dest[0], &dest[21]));

  // attwithquotedsemicolon
  val = "attachment; filename=\"Here's a semicolon;.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)24, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("Here's a semicolon;.html"),
                       std::string(&dest[0], &dest[24]));

  // attwithfilenameandextparam
  val = "attachment; foo=\"bar\"; filename=\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithfilenameandextparamescaped
  val = "attachment; foo=\"\\\"\\\\\";filename=\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithasciifilenameucase
  val = "attachment; FILENAME=\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithasciifilenamenq
  val = "attachment; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithtokfncommanq
  val = "attachment; filename=foo,bar.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithasciifilenamenqs
  val = "attachment; filename=foo.html ;";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attemptyparam
  val = "attachment; ;filename=foo";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithasciifilenamenqws
  val = "attachment; filename=foo bar.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfntokensq
  val = "attachment; filename='foo.bar'";
  CPPUNIT_ASSERT_EQUAL((ssize_t)9, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("'foo.bar'"),
                       std::string(&dest[0], &dest[9]));

  // attwithisofnplain
  // attachment; filename="foo-ä.html"
  val = "attachment; filename=\"foo-%E4.html\"";
  val = util::percentDecode(val.begin(), val.end());
  CPPUNIT_ASSERT_EQUAL((ssize_t)10, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::iso8859p1ToUtf8(std::string(&dest[0], &dest[10])));

  // attwithutf8fnplain
  // attachment; filename="foo-Ã¤.html"
  val = "attachment; filename=\"foo-%C3%A4.html\"";
  val = util::percentDecode(val.begin(), val.end());
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-Ã¤.html"),
                       util::iso8859p1ToUtf8(std::string(&dest[0], &dest[11])));

  // attwithfnrawpctenca
  val = "attachment; filename=\"foo-%41.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)12, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-%41.html"),
                       std::string(&dest[0], &dest[12]));

  // attwithfnusingpct
  val = "attachment; filename=\"50%.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("50%.html"),
                       std::string(&dest[0], &dest[8]));

  // attwithfnrawpctencaq
  val = "attachment; filename=\"foo-%\\41.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)12, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-%41.html"),
                       std::string(&dest[0], &dest[12]));

  // attwithnamepct
  val = "attachment; name=\"foo-%41.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // attwithfilenamepctandiso
  // attachment; filename="ä-%41.html"
  val = "attachment; filename=\"%E4-%2541.html\"";
  val = util::percentDecode(val.begin(), val.end());
  CPPUNIT_ASSERT_EQUAL((ssize_t)10, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("ä-%41.html"),
                       util::iso8859p1ToUtf8(std::string(&dest[0], &dest[10])));

  // attwithfnrawpctenclong
  val = "attachment; filename=\"foo-%c3%a4-%e2%82%ac.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)25, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-%c3%a4-%e2%82%ac.html"),
                       std::string(&dest[0], &dest[25]));

  // attwithasciifilenamews1
  val = "attachment; filename =\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attwith2filenames
  val = "attachment; filename=\"foo.html\"; filename=\"bar.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attfnbrokentoken
  val = "attachment; filename=foo[1](2).html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attfnbrokentokeniso
  val = "attachment; filename=foo-%E4.html";
  val = util::percentDecode(val.begin(), val.end());
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attfnbrokentokenutf
  // attachment; filename=foo-Ã¤.html
  val = "attachment; filename=foo-ä.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdisposition
  val = "filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdisposition2
  val = "x=y; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdisposition3
  val = "\"foo; filename=bar;baz\"; filename=qux";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdisposition4
  val = "filename=foo.html, filename=bar.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // emptydisposition
  val = "; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // doublecolon
  val = ": inline; attachment; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attandinline
  val = "inline; attachment; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attandinline2
  val = "attachment; inline; filename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attbrokenquotedfn
  val = "attachment; filename=\"foo.html\".txt";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attbrokenquotedfn2
  val = "attachment; filename=\"bar";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attbrokenquotedfn3
  val = "attachment; filename=foo\"bar;baz\"qux";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmultinstances
  val = "attachment; filename=foo.html, attachment; filename=bar.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
}

void UtilTest1::testParseContentDisposition2()
{
  char dest[1_k];
  size_t destlen = sizeof(dest);
  const char* cs;
  size_t cslen;
  std::string val;

  // test cases from http://greenbytes.de/tech/tc2231/
  // attmissingdelim
  val = "attachment; foo=foo filename=bar";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdelim2
  val = "attachment; filename=bar foo=foo ";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attmissingdelim3
  val = "attachment filename=bar";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attreversed
  val = "filename=foo.html; attachment";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attconfusedparam
  val = "attachment; xfilename=foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // attabspath
  val = "attachment; filename=\"/foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)9, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("/foo.html"),
                       std::string(&dest[0], &dest[9]));

  // attabspathwin
  val = "attachment; filename=\"\\\\foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)9, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("\\foo.html"),
                       std::string(&dest[0], &dest[9]));

  // attcdate
  val = "attachment; creation-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // dispext
  val = "foobar";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // dispextbadfn
  val = "attachment; example=\"filename=example.txt\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // attwithisofn2231iso
  val = "attachment; filename*=iso-8859-1''foo-%E4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)10, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("iso-8859-1"), std::string(cs, cslen));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       util::iso8859p1ToUtf8(std::string(&dest[0], &dest[10])));

  // attwithfn2231utf8
  val = "attachment; filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)15, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("UTF-8"), std::string(cs, cslen));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä-€.html"),
                       std::string(&dest[0], &dest[15]));

  // attwithfn2231noc
  val = "attachment; filename*=''foo-%c3%a4-%e2%82%ac.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231utf8comp
  val = "attachment; filename*=UTF-8''foo-a%cc%88.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)12, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  val = "foo-a%cc%88.html";
  CPPUNIT_ASSERT_EQUAL(std::string(util::percentDecode(val.begin(), val.end())),
                       std::string(&dest[0], &dest[12]));

  // attwithfn2231utf8-bad
  val = "attachment; filename*=iso-8859-1''foo-%c3%a4-%e2%82%ac.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231iso-bad
  val = "attachment; filename*=utf-8''foo-%E4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231ws1
  val = "attachment; filename *=UTF-8''foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231ws2
  val = "attachment; filename*= UTF-8''foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       std::string(&dest[0], &dest[11]));

  // attwithfn2231ws3
  val = "attachment; filename* =UTF-8''foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       std::string(&dest[0], &dest[11]));

  // attwithfn2231quot
  val = "attachment; filename*=\"UTF-8''foo-%c3%a4.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231quot2
  val = "attachment; filename*=\"foo%20bar.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231singleqmissing
  val = "attachment; filename*=UTF-8'foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231nbadpct1
  val = "attachment; filename*=UTF-8''foo%";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231nbadpct2
  val = "attachment; filename*=UTF-8''f%oo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attwithfn2231dpct
  val = "attachment; filename*=UTF-8''A-%2541.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)10, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("A-%41.html"),
                       std::string(&dest[0], &dest[10]));

  // attwithfn2231abspathdisguised
  val = "attachment; filename*=UTF-8''%5cfoo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)9, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("\\foo.html"),
                       std::string(&dest[0], &dest[9]));

  // attfnboth
  val =
      "attachment; filename=\"foo-ae.html\"; filename*=UTF-8''foo-%c3%a4.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       std::string(&dest[0], &dest[11]));

  // attfnboth2
  val =
      "attachment; filename*=UTF-8''foo-%c3%a4.html; filename=\"foo-ae.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       std::string(&dest[0], &dest[11]));

  // attfnboth3
  val = "attachment; filename*0*=ISO-8859-15''euro-sign%3d%a4; "
        "filename*=ISO-8859-1''currency-sign%3d%a4";
  CPPUNIT_ASSERT_EQUAL((ssize_t)15, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("ISO-8859-1"), std::string(cs, cslen));
  CPPUNIT_ASSERT_EQUAL(std::string("currency-sign=¤"),
                       util::iso8859p1ToUtf8(std::string(&dest[0], &dest[15])));

  // attnewandfn
  val = "attachment; foobar=x; filename=\"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // attrfc2047token
  val = "attachment; filename==?ISO-8859-1?Q?foo-=E4.html?=";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // attrfc2047quoted
  val = "attachment; filename=\"=?ISO-8859-1?Q?foo-=E4.html?=\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)29, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("=?ISO-8859-1?Q?foo-=E4.html?="),
                       std::string(&dest[0], &dest[29]));

  // aria2 original testcases

  // zero-length filename. token cannot be empty, so this is invalid.
  val = "attachment; filename=";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // zero-length filename. quoted-string can be empty string, so this
  // is ok.
  val = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));

  // empty value is not allowed
  val = "attachment; filename=;";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // / is not valid char in token.
  val = "attachment; filename=dir/file";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));

  // value-chars is *(pct-encoded / attr-char), so empty string is
  // allowed.
  val = "attachment; filename*=UTF-8''";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("UTF-8"), std::string(cs, cslen));

  val = "attachment; filename*=UTF-8''; filename=foo";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("UTF-8"), std::string(cs, cslen));

  val = "attachment; filename*=UTF-8''  ; filename=foo";
  CPPUNIT_ASSERT_EQUAL((ssize_t)0, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("UTF-8"), std::string(cs, cslen));

  // with language
  val = "attachment; filename*=UTF-8'japanese'konnichiwa";
  CPPUNIT_ASSERT_EQUAL((ssize_t)10, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("konnichiwa"),
                       std::string(&dest[0], &dest[10]));

  // lws before and after "="
  val = "attachment; filename = foo.html";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // lws before and after "=" with quoted-string
  val = "attachment; filename = \"foo.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // lws after parm
  val = "attachment; filename=foo.html  ";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  val = "attachment; filename=foo.html ; hello=world";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  val = "attachment; filename=\"foo.html\"  ";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  val = "attachment; filename=\"foo.html\" ; hello=world";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  val = "attachment; filename*=UTF-8''foo.html  ; hello=world";
  CPPUNIT_ASSERT_EQUAL((ssize_t)8, util::parse_content_disposition(
                                       dest, destlen, &cs, &cslen, val.c_str(),
                                       val.size(), false));
  CPPUNIT_ASSERT_EQUAL(std::string("foo.html"),
                       std::string(&dest[0], &dest[8]));

  // allow utf8 if content-disposition-default-utf8 is set
  val = "attachment; filename=\"foo-ä.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)11, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), true));
  CPPUNIT_ASSERT_EQUAL(std::string("foo-ä.html"),
                       std::string(&dest[0], &dest[11]));

  // incomplete utf8 sequence must be rejected
  val = "attachment; filename=\"foo-\xc3.html\"";
  CPPUNIT_ASSERT_EQUAL((ssize_t)-1, util::parse_content_disposition(
                                        dest, destlen, &cs, &cslen, val.c_str(),
                                        val.size(), true));
}

} // namespace aria2
