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

namespace aria2 {

class UtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilTest);
  CPPUNIT_TEST(testTrim);
  CPPUNIT_TEST(testSplit);
  CPPUNIT_TEST(testSplit_many);
  CPPUNIT_TEST(testEndsWith);
  CPPUNIT_TEST(testReplace);
  CPPUNIT_TEST(testStartsWith);
  // may be moved to other helper class in the future.
  CPPUNIT_TEST(testGetContentDispositionFilename);
  CPPUNIT_TEST(testRandomAlpha);
  CPPUNIT_TEST(testToUpper);
  CPPUNIT_TEST(testToLower);
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
  CPPUNIT_TEST(testParseIntRange);
  CPPUNIT_TEST(testParseIntRange_invalidRange);
  CPPUNIT_TEST(testParseInt);
  CPPUNIT_TEST(testParseUInt);
  CPPUNIT_TEST(testParseLLInt);
  CPPUNIT_TEST(testParseULLInt);
  CPPUNIT_TEST(testToString_binaryStream);
  CPPUNIT_TEST(testItos);
  CPPUNIT_TEST(testUitos);
  CPPUNIT_TEST(testNtoh64);
  CPPUNIT_TEST(testPercentEncode);
  CPPUNIT_TEST(testHtmlEscape);
  CPPUNIT_TEST(testJoinPath);
  CPPUNIT_TEST(testParseIndexPath);
  CPPUNIT_TEST(testCreateIndexPathMap);
  CPPUNIT_TEST(testGenerateRandomData);
  CPPUNIT_TEST(testFromHex);
  CPPUNIT_TEST(testParsePrioritizePieceRange);
  CPPUNIT_TEST(testApplyDir);
  CPPUNIT_TEST(testFixTaintedBasename);
  CPPUNIT_TEST(testIsNumericHost);
  CPPUNIT_TEST(testDetectDirTraversal);
  CPPUNIT_TEST(testEscapePath);
  CPPUNIT_TEST(testGetCidrPrefix);
  CPPUNIT_TEST(testInSameCidrBlock);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testTrim();
  void testSplit();
  void testSplit_many();
  void testEndsWith();
  void testReplace();
  void testStartsWith();
  // may be moved to other helper class in the future.
  void testGetContentDispositionFilename();
  void testRandomAlpha();
  void testToUpper();
  void testToLower();
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
  void testParseIntRange();
  void testParseIntRange_invalidRange();
  void testParseInt();
  void testParseUInt();
  void testParseLLInt();
  void testParseULLInt();
  void testToString_binaryStream();
  void testItos();
  void testUitos();
  void testNtoh64();
  void testPercentEncode();
  void testHtmlEscape();
  void testJoinPath();
  void testParseIndexPath();
  void testCreateIndexPathMap();
  void testGenerateRandomData();
  void testFromHex();
  void testParsePrioritizePieceRange();
  void testApplyDir();
  void testFixTaintedBasename();
  void testIsNumericHost();
  void testDetectDirTraversal();
  void testEscapePath();
  void testGetCidrPrefix();
  void testInSameCidrBlock();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UtilTest );

void UtilTest::testTrim() {
  std::string str1 = "aria2";
  CPPUNIT_ASSERT_EQUAL(str1, util::trim("aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, util::trim(" aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, util::trim(" aria2 "));
  CPPUNIT_ASSERT_EQUAL(str1, util::trim("  aria2  "));
  std::string str2 = "aria2 debut";
  CPPUNIT_ASSERT_EQUAL(str2, util::trim("aria2 debut"));
  CPPUNIT_ASSERT_EQUAL(str2, util::trim(" aria2 debut "));
  std::string str3 = "";
  CPPUNIT_ASSERT_EQUAL(str3, util::trim(""));
  CPPUNIT_ASSERT_EQUAL(str3, util::trim(" "));
  CPPUNIT_ASSERT_EQUAL(str3, util::trim("  "));
  std::string str4 = "A";
  CPPUNIT_ASSERT_EQUAL(str4, util::trim("A"));
  CPPUNIT_ASSERT_EQUAL(str4, util::trim(" A "));
  CPPUNIT_ASSERT_EQUAL(str4, util::trim("  A  "));
}

void UtilTest::testSplit() {
  std::pair<std::string, std::string> p1;
  util::split(p1, "name=value", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  util::split(p1, " name = value ", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  util::split(p1, "=value", '=');
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  util::split(p1, "name=", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.second);
  util::split(p1, "name", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.second);
}

void UtilTest::testSplit_many() {
  std::vector<std::string> v1;
  util::split("name1=value1; name2=value2; name3=value3",std::back_inserter(v1),
              ";", true);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v1.size());
  std::vector<std::string>::iterator itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("name3=value3"), *itr++);

  v1.clear();

  util::split("name1=value1; name2=value2; name3=value3",std::back_inserter(v1),
              ";", false);
  CPPUNIT_ASSERT_EQUAL((size_t)3, v1.size());
  itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" name3=value3"), *itr++);

  v1.clear();

  util::split("k=v", std::back_inserter(v1), ";", false, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v1.size());
  CPPUNIT_ASSERT_EQUAL(std::string("k=v"), v1[0]);

  v1.clear();

  util::split(" ", std::back_inserter(v1), ";", true, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, v1.size());
  CPPUNIT_ASSERT_EQUAL(std::string(""), v1[0]);

  v1.clear();

  util::split(" ", std::back_inserter(v1), ";", true);
  CPPUNIT_ASSERT_EQUAL((size_t)0, v1.size());
}

void UtilTest::testEndsWith() {
  std::string target = "abcdefg";
  std::string part = "fg";
  CPPUNIT_ASSERT(util::endsWith(target, part));

  target = "abdefg";
  part = "g";
  CPPUNIT_ASSERT(util::endsWith(target, part));

  target = "abdefg";
  part = "eg";
  CPPUNIT_ASSERT(!util::endsWith(target, part));

  target = "g";
  part = "eg";
  CPPUNIT_ASSERT(!util::endsWith(target, part));

  target = "g";
  part = "g";
  CPPUNIT_ASSERT(util::endsWith(target, part));

  target = "g";
  part = "";
  CPPUNIT_ASSERT(util::endsWith(target, part));

  target = "";
  part = "";
  CPPUNIT_ASSERT(util::endsWith(target, part));

  target = "";
  part = "g";
  CPPUNIT_ASSERT(!util::endsWith(target, part));
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
  CPPUNIT_ASSERT(util::startsWith(target, part));

  target = "abcdefg";
  part = "abx";
  CPPUNIT_ASSERT(!util::startsWith(target, part));

  target = "abcdefg";
  part = "bcd";
  CPPUNIT_ASSERT(!util::startsWith(target, part));

  target = "";
  part = "a";
  CPPUNIT_ASSERT(!util::startsWith(target, part));

  target = "";
  part = "";
  CPPUNIT_ASSERT(util::startsWith(target, part));
  
  target = "a";
  part = "";
  CPPUNIT_ASSERT(util::startsWith(target, part));

  target = "a";
  part = "a";
  CPPUNIT_ASSERT(util::startsWith(target, part));

}

void UtilTest::testGetContentDispositionFilename() {
  std::string h1 = "attachment; filename=\"aria2.tar.bz2\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), util::getContentDispositionFilename(h1));

  std::string h2 = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h2));

  std::string h3 = "attachment; filename=\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h3));

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
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       util::getContentDispositionFilename(h9));

  std::string h10 = "attachment; filename=";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h10));

  std::string h11 = "attachment; filename=;";
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::getContentDispositionFilename(h11));

  std::string filenameWithDir = "attachment; filename=dir/file";
  CPPUNIT_ASSERT_EQUAL(std::string(""),
                       util::getContentDispositionFilename(filenameWithDir));
  CPPUNIT_ASSERT_EQUAL
    (std::string(""),
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

void UtilTest::testPercentDecode() {
  std::string src = "http://aria2.sourceforge.net/aria2%200.7.0%20docs.html";
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sourceforge.net/aria2 0.7.0 docs.html"),
                       util::percentDecode(src));

  std::string src2 = "aria2+aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2+aria2"), util::percentDecode(src2));

  std::string src3 = "%5t%20";
  CPPUNIT_ASSERT_EQUAL(std::string("%5t "), util::percentDecode(src3));

  std::string src4 = "%";
  CPPUNIT_ASSERT_EQUAL(std::string("%"), util::percentDecode(src4));
  
  std::string src5 = "%3";
  CPPUNIT_ASSERT_EQUAL(std::string("%3"), util::percentDecode(src5));

  std::string src6 = "%2f";
  CPPUNIT_ASSERT_EQUAL(std::string("/"), util::percentDecode(src6));
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
  util::toStream(entries.begin(), entries.end(), os);
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
                       os.str());
}

void UtilTest::testIsNumber()
{
  CPPUNIT_ASSERT_EQUAL(true, util::isNumber("000"));
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber("a"));
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber("0a"));
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(""));
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(" "));
}

void UtilTest::testIsLowercase()
{
  CPPUNIT_ASSERT_EQUAL(true, util::isLowercase("alpha"));
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase("Alpha"));
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase("1alpha"));
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(""));
  CPPUNIT_ASSERT_EQUAL(false, util::isLowercase(" "));
}

void UtilTest::testIsUppercase()
{
  CPPUNIT_ASSERT_EQUAL(true, util::isUppercase("ALPHA"));
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase("Alpha"));
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase("1ALPHA"));
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(""));
  CPPUNIT_ASSERT_EQUAL(false, util::isUppercase(" "));
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
  std::string dir = "/tmp/aria2-UtilTest-testMkdirs";
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  util::mkdirs(dir);
  CPPUNIT_ASSERT(d.isDir());

  std::string file = "./UtilTest.cc";
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

void UtilTest::testParseIntRange()
{
  IntSequence seq = util::parseIntRange("1,3-8,10");

  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)1, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)3, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)4, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)5, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)6, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)7, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)8, seq.next());
  CPPUNIT_ASSERT(seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)10, seq.next());
  CPPUNIT_ASSERT(!seq.hasNext());
  CPPUNIT_ASSERT_EQUAL((int32_t)0, seq.next()); 
}

void UtilTest::testParseIntRange_invalidRange()
{
  try {
    IntSequence seq = util::parseIntRange("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = util::parseIntRange("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = util::parseIntRange("2147483647-2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = util::parseIntRange("1-2x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = util::parseIntRange("3x-4");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseInt()
{
  CPPUNIT_ASSERT_EQUAL(-1, util::parseInt(" -1 "));
  CPPUNIT_ASSERT_EQUAL(2147483647, util::parseInt("2147483647"));
  try {
    util::parseInt("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseInt("-2147483649");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseUInt()
{
  CPPUNIT_ASSERT_EQUAL(4294967295U, util::parseUInt(" 4294967295 "));
  try {
    util::parseUInt("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseUInt("4294967296");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseLLInt()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)-1LL, util::parseLLInt(" -1 "));
  CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL,
                       util::parseLLInt("9223372036854775807"));
  try {
    util::parseLLInt("9223372036854775808");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseLLInt("-9223372036854775809");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseLLInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseLLInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseULLInt()
{
  CPPUNIT_ASSERT_EQUAL((uint64_t)18446744073709551615ULL,
                       util::parseULLInt("18446744073709551615"));
  try {
    util::parseUInt("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::parseLLInt("18446744073709551616");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
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
  std::map<size_t, std::string>::value_type p = util::parseIndexPath("1=foo");
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

void UtilTest::testCreateIndexPathMap()
{
  std::stringstream in
    ("1=/tmp/myfile\n"
     "100=/myhome/mypicture.png\n");
  std::map<size_t, std::string> m = util::createIndexPathMap(in);
  CPPUNIT_ASSERT_EQUAL((size_t)2, m.size());
  CPPUNIT_ASSERT(m.find(1) != m.end());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), m[1]);
  CPPUNIT_ASSERT(m.find(100) != m.end());
  CPPUNIT_ASSERT_EQUAL(std::string("/myhome/mypicture.png"), m[100]);
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
  dest = util::fromHex(src);
  CPPUNIT_ASSERT_EQUAL((size_t)3, dest.size());
  CPPUNIT_ASSERT_EQUAL((char)0x00, dest[0]);
  CPPUNIT_ASSERT_EQUAL((char)0x11, dest[1]);
  CPPUNIT_ASSERT_EQUAL((char)0xff, dest[2]);

  src = "0011f";
  CPPUNIT_ASSERT(util::fromHex(src).empty());

  src = "001g";
  CPPUNIT_ASSERT(util::fromHex(src).empty());
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
  CPPUNIT_ASSERT_EQUAL(std::string("a_b"), util::fixTaintedBasename("a/b"));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("a_b"), util::fixTaintedBasename("a\\b"));
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
  CPPUNIT_ASSERT_EQUAL(std::string("foo_bar__"),
                       util::escapePath(std::string("foo")+(char)0x00+
                                        std::string("bar")+(char)0x00+
                                        (char)0x01));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo_bar"), util::escapePath("foo\\bar"));
#else // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo\\bar"), util::escapePath("foo\\bar"));
#endif // !__MINGW32__
}

void UtilTest::testGetCidrPrefix()
{
  struct in_addr in;
  CPPUNIT_ASSERT(util::getCidrPrefix(in, "192.168.0.1", 16));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.0"), std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(util::getCidrPrefix(in, "192.168.255.255", 17));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.128.0"),std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(util::getCidrPrefix(in, "192.168.128.1", 16));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.0"), std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(util::getCidrPrefix(in, "192.168.0.1", 32));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(util::getCidrPrefix(in, "192.168.0.1", 0));
  CPPUNIT_ASSERT_EQUAL(std::string("0.0.0.0"), std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(util::getCidrPrefix(in, "10.10.1.44", 27));
  CPPUNIT_ASSERT_EQUAL(std::string("10.10.1.32"), std::string(inet_ntoa(in)));

  CPPUNIT_ASSERT(!util::getCidrPrefix(in, "::1", 32));
}

void UtilTest::testInSameCidrBlock()
{
  CPPUNIT_ASSERT(util::inSameCidrBlock("192.168.128.1", "192.168.0.1", 16));
  CPPUNIT_ASSERT(!util::inSameCidrBlock("192.168.128.1", "192.168.0.1", 17));
}

} // namespace aria2
