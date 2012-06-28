#include "util.h"

#include <cstring>
#include <string>
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

class UtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilTest);
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
  CPPUNIT_TEST(testRandomAlpha);
  CPPUNIT_TEST(testToUpper);
  CPPUNIT_TEST(testToLower);
  CPPUNIT_TEST(testUppercase);
  CPPUNIT_TEST(testLowercase);
  CPPUNIT_TEST(testPercentDecode);
  CPPUNIT_TEST(testGetRealSize);
  CPPUNIT_TEST(testAbbrevSize);
  CPPUNIT_TEST(testToStream);
  CPPUNIT_TEST(testIsNumber);
  CPPUNIT_TEST(testIsLowercase);
  CPPUNIT_TEST(testIsUppercase);
  CPPUNIT_TEST(testAlphaToNum);
  CPPUNIT_TEST(testMkdirs);
  CPPUNIT_TEST(testConvertBitfield);
  CPPUNIT_TEST(testParseIntSegments);
  CPPUNIT_TEST(testParseIntSegments_invalidRange);
  CPPUNIT_TEST(testParseInt);
  CPPUNIT_TEST(testParseUInt);
  CPPUNIT_TEST(testParseLLInt);
  CPPUNIT_TEST(testParseIntNoThrow);
  CPPUNIT_TEST(testParseUIntNoThrow);
  CPPUNIT_TEST(testParseLLIntNoThrow);
  CPPUNIT_TEST(testToString_binaryStream);
  CPPUNIT_TEST(testItos);
  CPPUNIT_TEST(testUitos);
  CPPUNIT_TEST(testNtoh64);
  CPPUNIT_TEST(testPercentEncode);
  CPPUNIT_TEST(testHtmlEscape);
  CPPUNIT_TEST(testJoinPath);
  CPPUNIT_TEST(testParseIndexPath);
  CPPUNIT_TEST(testCreateIndexPaths);
  CPPUNIT_TEST(testGenerateRandomData);
  CPPUNIT_TEST(testFromHex);
  CPPUNIT_TEST(testParsePrioritizePieceRange);
  CPPUNIT_TEST(testApplyDir);
  CPPUNIT_TEST(testFixTaintedBasename);
  CPPUNIT_TEST(testIsNumericHost);
  CPPUNIT_TEST(testDetectDirTraversal);
  CPPUNIT_TEST(testEscapePath);
  CPPUNIT_TEST(testInSameCidrBlock);
  CPPUNIT_TEST(testIsUtf8String);
  CPPUNIT_TEST(testNextParam);
  CPPUNIT_TEST(testNoProxyDomainMatch);
  CPPUNIT_TEST(testInPrivateAddress);
  CPPUNIT_TEST(testSecfmt);
  CPPUNIT_TEST(testTlsHostnameMatch);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

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
  void testRandomAlpha();
  void testToUpper();
  void testToLower();
  void testUppercase();
  void testLowercase();
  void testPercentDecode();
  void testGetRealSize();
  void testAbbrevSize();
  void testToStream();
  void testIsNumber();
  void testIsLowercase();
  void testIsUppercase();
  void testAlphaToNum();
  void testMkdirs();
  void testConvertBitfield();
  void testParseIntSegments();
  void testParseIntSegments_invalidRange();
  void testParseInt();
  void testParseUInt();
  void testParseLLInt();
  void testParseIntNoThrow();
  void testParseUIntNoThrow();
  void testParseLLIntNoThrow();
  void testToString_binaryStream();
  void testItos();
  void testUitos();
  void testNtoh64();
  void testPercentEncode();
  void testHtmlEscape();
  void testJoinPath();
  void testParseIndexPath();
  void testCreateIndexPaths();
  void testGenerateRandomData();
  void testFromHex();
  void testParsePrioritizePieceRange();
  void testApplyDir();
  void testFixTaintedBasename();
  void testIsNumericHost();
  void testDetectDirTraversal();
  void testEscapePath();
  void testInSameCidrBlock();
  void testIsUtf8String();
  void testNextParam();
  void testNoProxyDomainMatch();
  void testInPrivateAddress();
  void testSecfmt();
  void testTlsHostnameMatch();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UtilTest );

void UtilTest::testStrip()
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

void UtilTest::testStripIter()
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

void UtilTest::testLstripIter()
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

void UtilTest::testLstripIter_char()
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

void UtilTest::testDivide() {
  std::pair<Sip, Sip> p1;
  std::string s = "name=value";
  util::divide(p1, s.begin(), s.end(), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = " name = value ";
  util::divide(p1, s.begin(), s.end(), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = "=value";
  util::divide(p1, s.begin(), s.end(), '=');
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string("value"),
                       std::string(p1.second.first, p1.second.second));
  s = "name=";
  util::divide(p1, s.begin(), s.end(), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.second.first, p1.second.second));
  s = "name";
  util::divide(p1, s.begin(), s.end(), '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"),
                       std::string(p1.first.first, p1.first.second));
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       std::string(p1.second.first, p1.second.second));
}

void UtilTest::testSplit() {
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

void UtilTest::testSplitIter() {
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

void UtilTest::testSplitIterM() {
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

void UtilTest::testEndsWith() {
  std::string target = "abcdefg";
  std::string part = "fg";
  CPPUNIT_ASSERT(util::endsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "abdefg";
  part = "g";
  CPPUNIT_ASSERT(util::endsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "abdefg";
  part = "eg";
  CPPUNIT_ASSERT(!util::endsWith(target.begin(), target.end(),
                                 part.begin(), part.end()));

  target = "g";
  part = "eg";
  CPPUNIT_ASSERT(!util::endsWith(target.begin(), target.end(),
                                 part.begin(), part.end()));

  target = "g";
  part = "g";
  CPPUNIT_ASSERT(util::endsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "g";
  part = "";
  CPPUNIT_ASSERT(util::endsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "";
  part = "";
  CPPUNIT_ASSERT(util::endsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "";
  part = "g";
  CPPUNIT_ASSERT(!util::endsWith(target.begin(), target.end(),
                                 part.begin(), part.end()));
}

void UtilTest::testIendsWith() {
  std::string target = "abcdefg";
  std::string part = "Fg";
  CPPUNIT_ASSERT(util::iendsWith(target.begin(), target.end(),
                                part.begin(), part.end()));

  target = "abdefg";
  part = "ef";
  CPPUNIT_ASSERT(!util::iendsWith(target.begin(), target.end(),
                                  part.begin(), part.end()));
}

void UtilTest::testStreq()
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

void UtilTest::testStrieq()
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

void UtilTest::testStrifind()
{
  std::string s1, s2;
  s1 = "yamagakani mukashi wo toheba hARU no tuki";
  s2 = "HaRu";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end())
                 != s1.end());
  s2 = "aki";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end())
                 == s1.end());
  s1 = "h";
  s2 = "HH";
  CPPUNIT_ASSERT(util::strifind(s1.begin(), s1.end(), s2.begin(), s2.end())
                 == s1.end());
}

void UtilTest::testReplace() {
  CPPUNIT_ASSERT_EQUAL(std::string("abc\n"), util::replace("abc\r\n", "\r", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), util::replace("abc\r\n", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::replace("", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), util::replace("abc", "", "a"));
  CPPUNIT_ASSERT_EQUAL(std::string("xbc"), util::replace("abc", "a", "x"));
}

void UtilTest::testStartsWith() {
  std::string target;
  std::string part;

  target = "abcdefg";
  part = "abc";
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(),
                                  part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(),
                                   part.begin(), part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "bcd";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(),
                                   part.begin(), part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "";
  part = "a";
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(),
                                   part.begin(), part.end()));
  CPPUNIT_ASSERT(!util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "";
  part = "";
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(),
                                  part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));
  
  target = "a";
  part = "";
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(),
                                  part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));

  target = "a";
  part = "a";
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(),
                                  part.begin(), part.end()));
  CPPUNIT_ASSERT(util::startsWith(target.begin(), target.end(), part.c_str()));
}

void UtilTest::testIstartsWith() {
  std::string target;
  std::string part;

  target = "abcdefg";
  part = "aBc";
  CPPUNIT_ASSERT(util::istartsWith(target.begin(), target.end(),
                                   part.begin(), part.end()));
  CPPUNIT_ASSERT(util::istartsWith(target.begin(), target.end(), part.c_str()));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!util::istartsWith(target.begin(), target.end(),
                                    part.begin(), part.end()));
  CPPUNIT_ASSERT(!util::istartsWith(target.begin(), target.end(), part.c_str()));
}

void UtilTest::testGetContentDispositionFilename() {
  std::string h1 = "attachment; filename=\"aria2.tar.bz2\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h1));

  std::string h2 = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h2));

  std::string h3 = "attachment; filename=\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h3));

  std::string h3_2 = "attachment; filename= \" aria2.tar.bz2 \"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       util::getContentDispositionFilename(h3_2));

  std::string h4 = "attachment;";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h4));

  std::string h5 = "attachment; filename=aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h5));

  std::string h6 = "attachment; filename='aria2.tar.bz2'";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h6));

  std::string h7 = "attachment; filename='aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h7));

  std::string h8 = "attachment; filename=aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h8));

  std::string h9 = "attachment; filename=\"aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT"),
                       util::getContentDispositionFilename(h9));

  std::string h10 = "attachment; filename=";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h10));

  std::string h11 = "attachment; filename=;";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h11));

  std::string filenameWithDir = "attachment; filename=dir/file";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(filenameWithDir));

  std::string semicolonInside = "attachment; filename=\"foo;bar\"";
  CPPUNIT_ASSERT_EQUAL(std::string("foo;bar"),
                       util::getContentDispositionFilename(semicolonInside));

  // Unescaping %2E%2E%2F produces "../". But since we won't unescape,
  // we just accept it as is.
  CPPUNIT_ASSERT_EQUAL
    (std::string("%2E%2E%2Ffoo.html"),
     util::getContentDispositionFilename("filename=\"%2E%2E%2Ffoo.html\""));

  // RFC2231 Section4
  std::string extparam2 = "filename*=''aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
                       util::getContentDispositionFilename(extparam2));
  std::string extparam3 = "filename*='''";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(extparam3));
  std::string extparam4 = "filename*='aria2";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(extparam4));
  std::string extparam5 = "filename*='''aria2";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(extparam5));
  std::string extparam6 = "filename*";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(extparam6));
  std::string extparam7 = "filename*=UTF-8''aria2;filename=hello%20world";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"),
                       util::getContentDispositionFilename(extparam7));
  std::string extparam8 = "filename=aria2;filename*=UTF-8''hello%20world";
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"),
                       util::getContentDispositionFilename(extparam8));
  std::string extparam9 = "filename*=ISO-8859-1''%A3";
  std::string extparam9ans;
  extparam9ans += 0xc2;
  extparam9ans += 0xa3;
  CPPUNIT_ASSERT_EQUAL(extparam9ans,
                       util::getContentDispositionFilename(extparam9));
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
     util::getContentDispositionFilename("filename*=UTF-8''foo%2F.html"));
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo.html"),
     util::getContentDispositionFilename("filename*=UTF-8'';filename=\"foo.html\""));
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
     util::getContentDispositionFilename("filename*=UTF-8''%2E%2E%2Ffoo.html"));

  // Tests from http://greenbytes.de/tech/tc2231/
  // attwithasciifnescapedchar
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo.html"),
     util::getContentDispositionFilename("filename=\"f\\oo.html\""));
  // attwithasciifilenameucase
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo.html"),
     util::getContentDispositionFilename("FILENAME=\"foo.html\""));
  // attwithisofn2231iso
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo-ä.html"),
     util::getContentDispositionFilename("filename*=iso-8859-1''foo-%E4.html"));
  // attwithfn2231utf8
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo-ä-€.html"),
     util::getContentDispositionFilename
     ("filename*=UTF-8''foo-%c3%a4-%e2%82%ac.html"));
  // attwithfn2231utf8-bad
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
     util::getContentDispositionFilename
     ("filename*=iso-8859-1''foo-%c3%a4-%e2%82%ac.html"));
  // attwithfn2231ws1
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
     util::getContentDispositionFilename("filename *=UTF-8''foo-%c3%a4.html"));
  // attwithfn2231ws2
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo-ä.html"),
     util::getContentDispositionFilename("filename*= UTF-8''foo-%c3%a4.html"));
  // attwithfn2231ws3
  CPPUNIT_ASSERT_EQUAL
    (std::string("foo-ä.html"),
     util::getContentDispositionFilename("filename* =UTF-8''foo-%c3%a4.html"));
  // attwithfn2231quot
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
     util::getContentDispositionFilename
     ("filename*=\"UTF-8''foo-%c3%a4.html\""));
}

class Printer {
public:
  template<class T>
  void operator()(T t) {
    std::cerr << t << ", ";
  }
};

void UtilTest::testRandomAlpha() {
  SharedHandle<Randomizer> rand(new FixedNumberRandomizer());
  std::string s = util::randomAlpha(8, rand);
  CPPUNIT_ASSERT_EQUAL(std::string("AAAAAAAA"), s);
}

void UtilTest::testToUpper() {
  std::string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  std::string upp = "608CABC0F2FA18C260CAFD974516865C772363D5";

  CPPUNIT_ASSERT_EQUAL(upp, util::toUpper(src));
}

void UtilTest::testToLower() {
  std::string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  std::string upp = "608cabc0f2fa18c260cafd974516865c772363d5";

  CPPUNIT_ASSERT_EQUAL(upp, util::toLower(src));
}

void UtilTest::testUppercase() {
  std::string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  std::string ans = "608CABC0F2FA18C260CAFD974516865C772363D5";
  util::uppercase(src);
  CPPUNIT_ASSERT_EQUAL(ans, src);
}

void UtilTest::testLowercase() {
  std::string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  std::string ans = "608cabc0f2fa18c260cafd974516865c772363d5";
  util::lowercase(src);
  CPPUNIT_ASSERT_EQUAL(ans, src);
}

void UtilTest::testPercentDecode() {
  std::string src = "http://aria2.sourceforge.net/aria2%200.7.0%20docs.html";
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sourceforge.net/aria2 0.7.0 docs.html"),
                       util::percentDecode(src.begin(), src.end()));

  std::string src2 = "aria2+aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2+aria2"),
                       util::percentDecode(src2.begin(), src2.end()));

  std::string src3 = "%5t%20";
  CPPUNIT_ASSERT_EQUAL(std::string("%5t "),
                       util::percentDecode(src3.begin(), src3.end()));

  std::string src4 = "%";
  CPPUNIT_ASSERT_EQUAL(std::string("%"),
                       util::percentDecode(src4.begin(), src4.end()));
  
  std::string src5 = "%3";
  CPPUNIT_ASSERT_EQUAL(std::string("%3"),
                       util::percentDecode(src5.begin(), src5.end()));

  std::string src6 = "%2f";
  CPPUNIT_ASSERT_EQUAL(std::string("/"),
                       util::percentDecode(src6.begin(), src6.end()));
}

void UtilTest::testGetRealSize()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, util::getRealSize("4096M"));
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, util::getRealSize("1K"));
  try {
    util::getRealSize("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("foo");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("9223372036854775807K");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("9223372036854775807M");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testAbbrevSize()
{
  CPPUNIT_ASSERT_EQUAL(std::string("4,096.0Mi"), util::abbrevSize(4294967296LL));
  CPPUNIT_ASSERT_EQUAL(std::string("1.0Ki"), util::abbrevSize(1024));
  CPPUNIT_ASSERT_EQUAL(std::string("1,023"), util::abbrevSize(1023));
  CPPUNIT_ASSERT_EQUAL(std::string("0"), util::abbrevSize(0));
  CPPUNIT_ASSERT_EQUAL(std::string("1.1Ki"), util::abbrevSize(1127));
  CPPUNIT_ASSERT_EQUAL(std::string("1.5Mi"), util::abbrevSize(1572864));

}

void UtilTest::testToStream()
{
  std::ostringstream os;
  SharedHandle<FileEntry> f1(new FileEntry("aria2.tar.bz2", 12300, 0));
  SharedHandle<FileEntry> f2(new FileEntry("aria2.txt", 556, 0));
  std::deque<SharedHandle<FileEntry> > entries;
  entries.push_back(f1);
  entries.push_back(f2);
  std::string filename = A2_TEST_OUT_DIR"/aria2_UtilTest_testToStream";
  BufferedFile fp(filename, BufferedFile::WRITE);
  util::toStream(entries.begin(), entries.end(), fp);
  fp.close();
  CPPUNIT_ASSERT_EQUAL(
                       std::string("Files:\n"
                                   "idx|path/length\n"
                                   "===+===========================================================================\n"
                                   "  1|aria2.tar.bz2\n"
                                   "   |12.0KiB (12,300)\n"
                                   "---+---------------------------------------------------------------------------\n"
                                   "  2|aria2.txt\n"
                                   "   |556B (556)\n"
                                   "---+---------------------------------------------------------------------------\n"),
                       readFile(filename));
}

void UtilTest::testIsNumber()
{
  std::string s = "000";
  CPPUNIT_ASSERT_EQUAL(true, util::isNumber(s.begin(),s.end()));
  s = "a";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = "0a";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = "";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = " ";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
}

void UtilTest::testIsLowercase()
{
  std::string s = "alpha";
  CPPUNIT_ASSERT_EQUAL(true, util::isLowercase(s.begin(), s.end()));
  s = "Alpha";
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(s.begin(), s.end()));
  s = "1alpha";
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(s.begin(), s.end()));
  s = "";
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(s.begin(), s.end()));
  s = " ";
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(s.begin(), s.end()));
}

void UtilTest::testIsUppercase()
{
  std::string s = "ALPHA";
  CPPUNIT_ASSERT_EQUAL(true, util::isUppercase(s.begin(), s.end()));
  s = "Alpha";
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(s.begin(), s.end()));
  s = "1ALPHA";
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(s.begin(), s.end()));
  s = "";
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(s.begin(), s.end()));
  s = " ";
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(s.begin(), s.end()));
}

void UtilTest::testAlphaToNum()
{
  CPPUNIT_ASSERT_EQUAL(0U, util::alphaToNum("a"));
  CPPUNIT_ASSERT_EQUAL(0U, util::alphaToNum("aa"));
  CPPUNIT_ASSERT_EQUAL(1U, util::alphaToNum("b"));
  CPPUNIT_ASSERT_EQUAL(675U, util::alphaToNum("zz")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(675U, util::alphaToNum("ZZ")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(0U, util::alphaToNum(""));
  CPPUNIT_ASSERT_EQUAL(4294967295U, util::alphaToNum("NXMRLXV"));
  CPPUNIT_ASSERT_EQUAL(0U, util::alphaToNum("NXMRLXW")); // uint32_t overflow
}

void UtilTest::testMkdirs()
{
  std::string dir = A2_TEST_OUT_DIR"/aria2-UtilTest-testMkdirs";
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  util::mkdirs(dir);
  CPPUNIT_ASSERT(d.isDir());

  std::string file = A2_TEST_DIR"/UtilTest.cc";
  File f(file);
  CPPUNIT_ASSERT(f.isFile());
  try {
    util::mkdirs(file);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& ex) {
    std::cerr << ex.stackTrace() << std::endl;
  }
}

void UtilTest::testConvertBitfield()
{
  BitfieldMan srcBitfield(384*1024, 256*1024*256+1);
  BitfieldMan destBitfield(512*1024, srcBitfield.getTotalLength());
  srcBitfield.setAllBit();
  srcBitfield.unsetBit(2);// <- range [768, 1152)
  // which corresponds to the index [1,2] in destBitfield
  util::convertBitfield(&destBitfield, &srcBitfield);
  
  CPPUNIT_ASSERT_EQUAL(std::string("9fffffffffffffffffffffffffffffff80"),
                       util::toHex(destBitfield.getBitfield(),
                                   destBitfield.getBitfieldLength()));
}

void UtilTest::testParseIntSegments()
{
  SegList<int> sgl;
  util::parseIntSegments(sgl, "1,3-8,10");

  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(1, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(3, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(4, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(5, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(6, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(7, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(8, sgl.next());
  CPPUNIT_ASSERT(sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(10, sgl.next());
  CPPUNIT_ASSERT(!sgl.hasNext());
  CPPUNIT_ASSERT_EQUAL(0, sgl.next());

  sgl.clear();
  util::parseIntSegments(sgl, ",,,1,,,3,,,");
  CPPUNIT_ASSERT_EQUAL(1, sgl.next());
  CPPUNIT_ASSERT_EQUAL(3, sgl.next());
  CPPUNIT_ASSERT(!sgl.hasNext());
}

void UtilTest::testParseIntSegments_invalidRange()
{
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "1-");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "2147483647-2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "1-2x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    SegList<int> sgl;
    util::parseIntSegments(sgl, "3x-4");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
}

void UtilTest::testParseInt()
{
  std::string s;
  s = " -1 ";
  CPPUNIT_ASSERT_EQUAL(-1, util::parseInt(s));
  s = "2147483647";
  CPPUNIT_ASSERT_EQUAL(2147483647, util::parseInt(s));
  try {
    s = "2147483648";
    util::parseInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    s = "-2147483649";
    util::parseInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    s = "12x";
    util::parseInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    s = "";
    util::parseInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
}

void UtilTest::testParseUInt()
{
  std::string s;
  s = " 2147483647 ";
  CPPUNIT_ASSERT_EQUAL(2147483647U, util::parseUInt(s));
  try {
    s = "-1";
    util::parseUInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
  try {
    s = "2147483648";
    util::parseUInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
  }
}

void UtilTest::testParseLLInt()
{
  std::string s;
  {
    s = " -1 ";
    CPPUNIT_ASSERT_EQUAL((int64_t)-1LL, util::parseLLInt(s));
  }
  {
    s = "9223372036854775807";
    CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL,
                         util::parseLLInt(s));
  }
  try {
    s = "9223372036854775808";
    util::parseLLInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    s = "-9223372036854775809";
    util::parseLLInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    s = "12x";
    util::parseLLInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    s = "";
    util::parseLLInt(s);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseIntNoThrow()
{
  std::string s;
  int32_t n;
  s = " -1 ";
  CPPUNIT_ASSERT(util::parseIntNoThrow(n, s));
  CPPUNIT_ASSERT_EQUAL((int32_t)-1, n);

  s = "2147483647";
  CPPUNIT_ASSERT(util::parseIntNoThrow(n, s));
  CPPUNIT_ASSERT_EQUAL((int32_t)2147483647, n);

  s = "2147483648";
  CPPUNIT_ASSERT(!util::parseIntNoThrow(n, s));
  s = "-2147483649";
  CPPUNIT_ASSERT(!util::parseIntNoThrow(n, s));

  s = "12x";
  CPPUNIT_ASSERT(!util::parseIntNoThrow(n, s));
  s = "";
  CPPUNIT_ASSERT(!util::parseIntNoThrow(n, s));
}

void UtilTest::testParseUIntNoThrow()
{
  std::string s;
  uint32_t n;
  s = " 2147483647 ";
  CPPUNIT_ASSERT(util::parseUIntNoThrow(n, s));
  CPPUNIT_ASSERT_EQUAL((uint32_t)INT32_MAX, n);
  s = "2147483648";
  CPPUNIT_ASSERT(!util::parseUIntNoThrow(n, s));
  s = "-1";
  CPPUNIT_ASSERT(!util::parseUIntNoThrow(n, s));
}

void UtilTest::testParseLLIntNoThrow()
{
  std::string s;
  int64_t n;
  s = " 9223372036854775807 ";
  CPPUNIT_ASSERT(util::parseLLIntNoThrow(n, s));
  CPPUNIT_ASSERT_EQUAL((int64_t)INT64_MAX, n);
  s = "9223372036854775808";
  CPPUNIT_ASSERT(!util::parseLLIntNoThrow(n, s));
  s = "-9223372036854775808";
  CPPUNIT_ASSERT(util::parseLLIntNoThrow(n, s));
  CPPUNIT_ASSERT_EQUAL((int64_t)INT64_MIN, n);
  s = "-9223372036854775809";
  CPPUNIT_ASSERT(!util::parseLLIntNoThrow(n, s));
}

void UtilTest::testToString_binaryStream()
{
  SharedHandle<DiskWriter> dw(new ByteArrayDiskWriter());
  std::string data(16*1024+256, 'a');
  dw->initAndOpenFile();
  dw->writeData((const unsigned char*)data.c_str(), data.size(), 0);

  std::string readData = util::toString(dw);

  CPPUNIT_ASSERT_EQUAL(data, readData);
}

void UtilTest::testItos()
{
  {
    int i = 0;
    CPPUNIT_ASSERT_EQUAL(std::string("0"), util::itos(i));
  }
  {
    int i = 100;
    CPPUNIT_ASSERT_EQUAL(std::string("100"), util::itos(i, true));
  }
  {
    int i = 100;
    CPPUNIT_ASSERT_EQUAL(std::string("100"), util::itos(i));
  }
  {
    int i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12,345"), util::itos(i, true));
  }
  {
    int i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12345"), util::itos(i));
  }
  {
    int i = -12345;
    CPPUNIT_ASSERT_EQUAL(std::string("-12,345"), util::itos(i, true));
  }
  {
    int64_t i = INT64_MAX;
    CPPUNIT_ASSERT_EQUAL(std::string("9,223,372,036,854,775,807"),
                         util::itos(i, true));
  }
  {
    int64_t i = INT64_MIN;
    CPPUNIT_ASSERT_EQUAL(std::string("-9,223,372,036,854,775,808"),
                         util::itos(i, true));
  }
}

void UtilTest::testUitos()
{
  {
    uint16_t i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12345"), util::uitos(i));
  }
  {
    int16_t i = -12345;
    CPPUNIT_ASSERT_EQUAL(std::string("/.-,+"), util::uitos(i));
  }
}

void UtilTest::testNtoh64()
{
  uint64_t x = 0xff00ff00ee00ee00LL;
#ifdef WORDS_BIGENDIAN
  CPPUNIT_ASSERT_EQUAL(x, ntoh64(x));
  CPPUNIT_ASSERT_EQUAL(x, hton64(x));
#else // !WORDS_BIGENDIAN
  uint64_t y = 0x00ee00ee00ff00ffLL;
  CPPUNIT_ASSERT_EQUAL(y, ntoh64(x));
  CPPUNIT_ASSERT_EQUAL(x, hton64(y));
#endif // !WORDS_BIGENDIAN
}

void UtilTest::testPercentEncode()
{
  CPPUNIT_ASSERT_EQUAL
    (std::string("%3A%2F%3F%23%5B%5D%40%21%25%26%27%28%29%2A%2B%2C%3B%3D"),
     util::percentEncode(":/?#[]@!%&'()*+,;="));

  std::string unreserved =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-._~";
  CPPUNIT_ASSERT_EQUAL(unreserved, util::percentEncode(unreserved));

  CPPUNIT_ASSERT_EQUAL(std::string("1%5EA%20"), util::percentEncode("1^A "));
}

void UtilTest::testHtmlEscape()
{
  CPPUNIT_ASSERT_EQUAL(std::string("aria2&lt;&gt;&quot;&#39;util"),
                       util::htmlEscape("aria2<>\"'util"));
}

void UtilTest::testJoinPath()
{
  const std::string dir1dir2file[] = { "dir1", "dir2", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("dir1/dir2/file"),
     util::joinPath(vbegin(dir1dir2file), vend(dir1dir2file)));

  const std::string dirparentfile[] = { "dir", "..", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("file"),
     util::joinPath(vbegin(dirparentfile), vend(dirparentfile)));

  const std::string dirparentparentfile[] = { "dir", "..", "..", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("file"),
     util::joinPath(vbegin(dirparentparentfile), vend(dirparentparentfile)));

  const std::string dirdotfile[] = { "dir", ".", "file" };
  CPPUNIT_ASSERT_EQUAL(std::string("dir/file"),
                       util::joinPath(vbegin(dirdotfile), vend(dirdotfile)));

  const std::string empty[] = {};
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::joinPath(&empty[0], &empty[0]));

  const std::string parentdot[] = { "..", "." };
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::joinPath(vbegin(parentdot), vend(parentdot)));
}

void UtilTest::testParseIndexPath()
{
  std::pair<size_t, std::string> p = util::parseIndexPath("1=foo");
  CPPUNIT_ASSERT_EQUAL((size_t)1, p.first);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), p.second);
  try {
    util::parseIndexPath("1X=foo");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    // success
  }
  try {
    util::parseIndexPath("1=");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    // success
  }
}

void UtilTest::testCreateIndexPaths()
{
  std::stringstream in
    ("1=/tmp/myfile\n"
     "100=/myhome/mypicture.png\n");
  std::vector<std::pair<size_t, std::string> > m = util::createIndexPaths(in);
  CPPUNIT_ASSERT_EQUAL((size_t)2, m.size());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m[0].first);
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), m[0].second);
  CPPUNIT_ASSERT_EQUAL((size_t)100, m[1].first);
  CPPUNIT_ASSERT_EQUAL(std::string("/myhome/mypicture.png"), m[1].second);
}

void UtilTest::testGenerateRandomData()
{
  unsigned char data1[20];
  util::generateRandomData(data1, sizeof(data1));
  unsigned char data2[20];
  util::generateRandomData(data2, sizeof(data2));
  CPPUNIT_ASSERT(memcmp(data1, data2, sizeof(data1)) != 0);
}

void UtilTest::testFromHex()
{
  std::string src;
  std::string dest;

  src = "0011fF";
  dest = util::fromHex(src.begin(), src.end());
  CPPUNIT_ASSERT_EQUAL((size_t)3, dest.size());
  CPPUNIT_ASSERT_EQUAL((char)0x00, dest[0]);
  CPPUNIT_ASSERT_EQUAL((char)0x11, dest[1]);
  CPPUNIT_ASSERT_EQUAL((char)0xff, dest[2]);

  src = "0011f";
  CPPUNIT_ASSERT(util::fromHex(src.begin(), src.end()).empty());

  src = "001g";
  CPPUNIT_ASSERT(util::fromHex(src.begin(), src.end()).empty());
}

void UtilTest::testParsePrioritizePieceRange()
{
  // piece index
  // 0     1     2     3     4     5     6     7
  // |     |              |                    |
  // file1 |              |                    |
  //       |              |                    |
  //       file2          |                    |
  //                    file3                  |
  //                      |                    |
  //                      file4                |
  size_t pieceLength = 1024;
  std::vector<SharedHandle<FileEntry> > entries(4, SharedHandle<FileEntry>());
  entries[0].reset(new FileEntry("file1", 1024, 0));
  entries[1].reset(new FileEntry("file2",2560,entries[0]->getLastOffset()));
  entries[2].reset(new FileEntry("file3",0,entries[1]->getLastOffset()));
  entries[3].reset(new FileEntry("file4",3584,entries[2]->getLastOffset()));

  std::vector<size_t> result;
  util::parsePrioritizePieceRange(result, "head=1", entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  result.clear();
  util::parsePrioritizePieceRange(result, "tail=1", entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[2]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head=1K", entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head", entries, pieceLength, 1024);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "tail=1K", entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)2, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "tail", entries, pieceLength, 1024);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)2, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head=1,tail=1", entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head=300M,tail=300M",
                                  entries, pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)7, result.size());
  for(size_t i = 0; i < 7; ++i) {
    CPPUNIT_ASSERT_EQUAL(i, result[i]);
  }
  result.clear();
  util::parsePrioritizePieceRange(result, "", entries, pieceLength);
  CPPUNIT_ASSERT(result.empty());
}

void UtilTest::testApplyDir()
{
  CPPUNIT_ASSERT_EQUAL(std::string("./pred"), util::applyDir("", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("/pred"), util::applyDir("/", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("./pred"), util::applyDir(".", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("/dl/pred"), util::applyDir("/dl", "pred"));
}

void UtilTest::testFixTaintedBasename()
{
  CPPUNIT_ASSERT_EQUAL(std::string("a%2Fb"), util::fixTaintedBasename("a/b"));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("a%5Cb"), util::fixTaintedBasename("a\\b"));
#else // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("a\\b"), util::fixTaintedBasename("a\\b"));
#endif // !__MINGW32__
}

void UtilTest::testIsNumericHost()
{
  CPPUNIT_ASSERT(util::isNumericHost("192.168.0.1"));
  CPPUNIT_ASSERT(!util::isNumericHost("aria2.sf.net"));
  CPPUNIT_ASSERT(util::isNumericHost("::1"));
}

void UtilTest::testDetectDirTraversal()
{
  CPPUNIT_ASSERT(util::detectDirTraversal("/foo"));
  CPPUNIT_ASSERT(util::detectDirTraversal("./foo"));
  CPPUNIT_ASSERT(util::detectDirTraversal("../foo"));
  CPPUNIT_ASSERT(util::detectDirTraversal("foo/../bar"));
  CPPUNIT_ASSERT(util::detectDirTraversal("foo/./bar"));
  CPPUNIT_ASSERT(util::detectDirTraversal("foo/."));
  CPPUNIT_ASSERT(util::detectDirTraversal("foo/.."));
  CPPUNIT_ASSERT(util::detectDirTraversal("."));
  CPPUNIT_ASSERT(util::detectDirTraversal(".."));
  CPPUNIT_ASSERT(util::detectDirTraversal("/"));
  CPPUNIT_ASSERT(util::detectDirTraversal("foo/"));
  CPPUNIT_ASSERT(util::detectDirTraversal("\t"));
  CPPUNIT_ASSERT(!util::detectDirTraversal("foo/bar"));
  CPPUNIT_ASSERT(!util::detectDirTraversal("foo"));
}

void UtilTest::testEscapePath()
{
  CPPUNIT_ASSERT_EQUAL(std::string("foo%00bar%00%01"),
                       util::escapePath(std::string("foo")+(char)0x00+
                                        std::string("bar")+(char)0x00+
                                        (char)0x01));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo%5Cbar"), util::escapePath("foo\\bar"));
#else // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo\\bar"), util::escapePath("foo\\bar"));
#endif // !__MINGW32__
}

void UtilTest::testInSameCidrBlock()
{
  CPPUNIT_ASSERT(util::inSameCidrBlock("192.168.128.1", "192.168.0.1", 16));
  CPPUNIT_ASSERT(!util::inSameCidrBlock("192.168.128.1", "192.168.0.1", 17));

  CPPUNIT_ASSERT(util::inSameCidrBlock("192.168.0.1", "192.168.0.1", 32));
  CPPUNIT_ASSERT(!util::inSameCidrBlock("192.168.0.1", "192.168.0.0", 32));

  CPPUNIT_ASSERT(util::inSameCidrBlock("192.168.0.1", "10.0.0.1", 0));

  CPPUNIT_ASSERT(util::inSameCidrBlock("2001:db8::2:1", "2001:db0::2:2", 28));
  CPPUNIT_ASSERT(!util::inSameCidrBlock("2001:db8::2:1", "2001:db0::2:2", 29));

  CPPUNIT_ASSERT(!util::inSameCidrBlock("2001:db8::2:1", "192.168.0.1", 8));
}

void UtilTest::testIsUtf8String()
{
  CPPUNIT_ASSERT(util::isUtf8("ascii"));
  // "Hello World" in Japanese UTF-8
  CPPUNIT_ASSERT(util::isUtf8
                 (fromHex("e38193e38293e381abe381a1e381afe4b896e7958c")));
  // "World" in Shift_JIS
  CPPUNIT_ASSERT(!util::isUtf8(fromHex("90a28a")+"E"));
  // UTF8-2
  CPPUNIT_ASSERT(util::isUtf8(fromHex("c280")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("dfbf")));
  // UTF8-3
  CPPUNIT_ASSERT(util::isUtf8(fromHex("e0a080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("e0bf80")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("e18080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("ec8080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("ed8080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("ed9f80")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("ee8080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("ef8080")));
  // UTF8-4
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f0908080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f0bf8080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f1808080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f3808080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f4808080")));
  CPPUNIT_ASSERT(util::isUtf8(fromHex("f48f8080")));

  CPPUNIT_ASSERT(util::isUtf8(""));
  CPPUNIT_ASSERT(!util::isUtf8(fromHex("00")));
}

void UtilTest::testNextParam()
{
  std::string s1 = "    :a  :  b=c :d=b::::g::";
  std::pair<std::string::iterator, bool> r;
  std::string name, value;
  r = util::nextParam(name, value, s1.begin(), s1.end(), ':');
  CPPUNIT_ASSERT(r.second);
  CPPUNIT_ASSERT_EQUAL(std::string("a"), name);
  CPPUNIT_ASSERT_EQUAL(std::string(), value);

  r = util::nextParam(name, value, r.first, s1.end(), ':');
  CPPUNIT_ASSERT(r.second);
  CPPUNIT_ASSERT_EQUAL(std::string("b"), name);
  CPPUNIT_ASSERT_EQUAL(std::string("c"), value);

  r = util::nextParam(name, value, r.first, s1.end(), ':');
  CPPUNIT_ASSERT(r.second);
  CPPUNIT_ASSERT_EQUAL(std::string("d"), name);
  CPPUNIT_ASSERT_EQUAL(std::string("b"), value);

  r = util::nextParam(name, value, r.first, s1.end(), ':');
  CPPUNIT_ASSERT(r.second);
  CPPUNIT_ASSERT_EQUAL(std::string("g"), name);
  CPPUNIT_ASSERT_EQUAL(std::string(), value);

  std::string s2 = "";
  r = util::nextParam(name, value, s2.begin(), s2.end(), ':');
  CPPUNIT_ASSERT(!r.second);

  std::string s3 = "   ";
  r = util::nextParam(name, value, s3.begin(), s3.end(), ':');
  CPPUNIT_ASSERT(!r.second);

  std::string s4 = ":::";
  r = util::nextParam(name, value, s4.begin(), s4.end(), ':');
  CPPUNIT_ASSERT(!r.second);
}

void UtilTest::testNoProxyDomainMatch()
{
  CPPUNIT_ASSERT(util::noProxyDomainMatch("localhost", "localhost"));
  CPPUNIT_ASSERT(util::noProxyDomainMatch("192.168.0.1", "192.168.0.1"));
  CPPUNIT_ASSERT(util::noProxyDomainMatch("www.example.org", ".example.org"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("www.example.org", "example.org"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("192.168.0.1", "0.1"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("example.org", "example.com"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("example.org", "www.example.org"));
}

void UtilTest::testInPrivateAddress()
{
  CPPUNIT_ASSERT(!util::inPrivateAddress("localhost"));
  CPPUNIT_ASSERT(util::inPrivateAddress("192.168.0.1"));
  // Only checks prefix..
  CPPUNIT_ASSERT(util::inPrivateAddress("10."));
  CPPUNIT_ASSERT(!util::inPrivateAddress("172."));
  CPPUNIT_ASSERT(!util::inPrivateAddress("172.15.0.0"));
  CPPUNIT_ASSERT(util::inPrivateAddress("172.16.0.0"));
  CPPUNIT_ASSERT(util::inPrivateAddress("172.31.0.0"));
  CPPUNIT_ASSERT(!util::inPrivateAddress("172.32.0.0"));
}

void UtilTest::testSecfmt()
{
  CPPUNIT_ASSERT_EQUAL(std::string("0s"), util::secfmt(0));
  CPPUNIT_ASSERT_EQUAL(std::string("1s"), util::secfmt(1));
  CPPUNIT_ASSERT_EQUAL(std::string("9s"), util::secfmt(9));
  CPPUNIT_ASSERT_EQUAL(std::string("10s"), util::secfmt(10));
  CPPUNIT_ASSERT_EQUAL(std::string("1m"), util::secfmt(60));
  CPPUNIT_ASSERT_EQUAL(std::string("1m59s"), util::secfmt(119));
  CPPUNIT_ASSERT_EQUAL(std::string("2m"), util::secfmt(120));
  CPPUNIT_ASSERT_EQUAL(std::string("59m59s"), util::secfmt(3599));
  CPPUNIT_ASSERT_EQUAL(std::string("1h"), util::secfmt(3600));
}

void UtilTest::testTlsHostnameMatch()
{
  CPPUNIT_ASSERT(util::tlsHostnameMatch("Foo.com", "foo.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("*.a.com", "foo.a.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("*.a.com", "bar.foo.a.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("f*.com", "foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("*.com", "bar.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("com", "com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("foo.*", "foo.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("a.foo.com", "A.foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("a.foo.com", "b.foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("*a.foo.com", "a.foo.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("*a.foo.com", "ba.foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("a*.foo.com", "a.foo.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("a*.foo.com", "ab.foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("foo.b*z.foo.com", "foo.baz.foo.com"));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("B*z.foo.com", "bAZ.Foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("b*z.foo.com", "bz.foo.com"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("*", "foo"));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("*", ""));
  CPPUNIT_ASSERT(util::tlsHostnameMatch("", ""));
  CPPUNIT_ASSERT(!util::tlsHostnameMatch("xn--*.a.b", "xn--c.a.b"));
}

} // namespace aria2
