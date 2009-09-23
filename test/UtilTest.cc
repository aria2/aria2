#include "Util.h"

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
  CPPUNIT_TEST(testSlice);
  CPPUNIT_TEST(testEndsWith);
  CPPUNIT_TEST(testReplace);
  CPPUNIT_TEST(testStartsWith);
  // may be moved to other helper class in the future.
  CPPUNIT_TEST(testGetContentDispositionFilename);
  CPPUNIT_TEST(testRandomAlpha);
  CPPUNIT_TEST(testToUpper);
  CPPUNIT_TEST(testToLower);
  CPPUNIT_TEST(testUrldecode);
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
  CPPUNIT_TEST(testUrlencode);
  CPPUNIT_TEST(testHtmlEscape);
  CPPUNIT_TEST(testJoinPath);
  CPPUNIT_TEST(testParseIndexPath);
  CPPUNIT_TEST(testCreateIndexPathMap);
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
  // may be moved to other helper class in the future.
  void testGetContentDispositionFilename();
  void testRandomAlpha();
  void testToUpper();
  void testToLower();
  void testUrldecode();
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
  void testUrlencode();
  void testHtmlEscape();
  void testJoinPath();
  void testParseIndexPath();
  void testCreateIndexPathMap();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UtilTest );

void UtilTest::testTrim() {
  std::string str1 = "aria2";
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim("aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim(" aria2"));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim(" aria2 "));
  CPPUNIT_ASSERT_EQUAL(str1, Util::trim("  aria2  "));
  std::string str2 = "aria2 debut";
  CPPUNIT_ASSERT_EQUAL(str2, Util::trim("aria2 debut"));
  CPPUNIT_ASSERT_EQUAL(str2, Util::trim(" aria2 debut "));
  std::string str3 = "";
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim(""));
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim(" "));
  CPPUNIT_ASSERT_EQUAL(str3, Util::trim("  "));
  std::string str4 = "A";
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim("A"));
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim(" A "));
  CPPUNIT_ASSERT_EQUAL(str4, Util::trim("  A  "));
}

void UtilTest::testSplit() {
  std::pair<std::string, std::string> p1;
  Util::split(p1, "name=value", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  Util::split(p1, " name = value ", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  Util::split(p1, "=value", '=');
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string("value"), p1.second);
  Util::split(p1, "name=", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.second);
  Util::split(p1, "name", '=');
  CPPUNIT_ASSERT_EQUAL(std::string("name"), p1.first);
  CPPUNIT_ASSERT_EQUAL(std::string(""), p1.second);
}

void UtilTest::testSlice() {
  std::deque<std::string> v1;
  Util::slice(v1, "name1=value1; name2=value2; name3=value3;", ';', true);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  v1.clear();
  Util::slice(v1, "name1=value1; name2=value2; name3=value3", ';', true);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  std::deque<std::string>::iterator itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string("name3=value3"), *itr++);

  v1.clear();

  Util::slice(v1, "name1=value1; name2=value2; name3=value3", ';', false);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(std::string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(std::string(" name3=value3"), *itr++);
}

void UtilTest::testEndsWith() {
  std::string target = "abcdefg";
  std::string part = "fg";
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
  CPPUNIT_ASSERT_EQUAL(std::string("abc\n"), Util::replace("abc\r\n", "\r", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), Util::replace("abc\r\n", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::replace("", "\r\n", ""));
  CPPUNIT_ASSERT_EQUAL(std::string("abc"), Util::replace("abc", "", "a"));
  CPPUNIT_ASSERT_EQUAL(std::string("xbc"), Util::replace("abc", "a", "x"));
}

void UtilTest::testStartsWith() {
  std::string target;
  std::string part;

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
  std::string h1 = "attachment; filename=\"aria2.tar.bz2\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), Util::getContentDispositionFilename(h1));

  std::string h2 = "attachment; filename=\"\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::getContentDispositionFilename(h2));

  std::string h3 = "attachment; filename=\"";
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::getContentDispositionFilename(h3));

  std::string h4 = "attachment;";
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::getContentDispositionFilename(h4));

  std::string h5 = "attachment; filename=aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), Util::getContentDispositionFilename(h5));

  std::string h6 = "attachment; filename='aria2.tar.bz2'";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), Util::getContentDispositionFilename(h6));

  std::string h7 = "attachment; filename='aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), Util::getContentDispositionFilename(h7));

  std::string h8 = "attachment; filename=aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"), Util::getContentDispositionFilename(h8));

  std::string h9 = "attachment; filename=\"aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT\"";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT"), Util::getContentDispositionFilename(h9));

  std::string h10 = "attachment; filename=";
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::getContentDispositionFilename(h10));

  std::string h11 = "attachment; filename=;";
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::getContentDispositionFilename(h11));

  std::string filenameWithDir = "attachment; filename=dir/file";
  CPPUNIT_ASSERT_EQUAL(std::string("file"),
		       Util::getContentDispositionFilename(filenameWithDir));

  std::string parentDir = "attachment; filename=..";
  CPPUNIT_ASSERT_EQUAL(std::string(),
		       Util::getContentDispositionFilename(parentDir));

  std::string currentDir = "attachment; filename=.";
  CPPUNIT_ASSERT_EQUAL(std::string(),
		       Util::getContentDispositionFilename(currentDir));
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
  std::string s = Util::randomAlpha(8, rand);
  CPPUNIT_ASSERT_EQUAL(std::string("AAAAAAAA"), s);
}

void UtilTest::testToUpper() {
  std::string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  std::string upp = "608CABC0F2FA18C260CAFD974516865C772363D5";

  CPPUNIT_ASSERT_EQUAL(upp, Util::toUpper(src));
}

void UtilTest::testToLower() {
  std::string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  std::string upp = "608cabc0f2fa18c260cafd974516865c772363d5";

  CPPUNIT_ASSERT_EQUAL(upp, Util::toLower(src));
}

void UtilTest::testUrldecode() {
  std::string src = "http://aria2.sourceforge.net/aria2%200.7.0%20docs.html";
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sourceforge.net/aria2 0.7.0 docs.html"),
		       Util::urldecode(src));

  std::string src2 = "aria2+aria2";
  CPPUNIT_ASSERT_EQUAL(std::string("aria2+aria2"), Util::urldecode(src2));

  std::string src3 = "%5t%20";
  CPPUNIT_ASSERT_EQUAL(std::string("%5t "), Util::urldecode(src3));

  std::string src4 = "%";
  CPPUNIT_ASSERT_EQUAL(std::string("%"), Util::urldecode(src4));
  
  std::string src5 = "%3";
  CPPUNIT_ASSERT_EQUAL(std::string("%3"), Util::urldecode(src5));

  std::string src6 = "%2f";
  CPPUNIT_ASSERT_EQUAL(std::string("/"), Util::urldecode(src6));
}

void UtilTest::testGetRealSize()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, Util::getRealSize("4096M"));
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, Util::getRealSize("1K"));
  try {
    Util::getRealSize("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::getRealSize("foo");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::getRealSize("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::getRealSize("9223372036854775807K");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::getRealSize("9223372036854775807M");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testAbbrevSize()
{
  CPPUNIT_ASSERT_EQUAL(std::string("4,096.0Mi"), Util::abbrevSize(4294967296LL));
  CPPUNIT_ASSERT_EQUAL(std::string("1.0Ki"), Util::abbrevSize(1024));
  CPPUNIT_ASSERT_EQUAL(std::string("1,023"), Util::abbrevSize(1023));
  CPPUNIT_ASSERT_EQUAL(std::string("0"), Util::abbrevSize(0));
  CPPUNIT_ASSERT_EQUAL(std::string("1.1Ki"), Util::abbrevSize(1127));
  CPPUNIT_ASSERT_EQUAL(std::string("1.5Mi"), Util::abbrevSize(1572864));

}

void UtilTest::testToStream()
{
  std::ostringstream os;
  SharedHandle<FileEntry> f1(new FileEntry("aria2.tar.bz2", 12300, 0));
  SharedHandle<FileEntry> f2(new FileEntry("aria2.txt", 556, 0));
  std::deque<SharedHandle<FileEntry> > entries;
  entries.push_back(f1);
  entries.push_back(f2);
  Util::toStream(entries.begin(), entries.end(), os);
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
  CPPUNIT_ASSERT_EQUAL(true, Util::isNumber("000"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isNumber("a"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isNumber("0a"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isNumber(""));
  CPPUNIT_ASSERT_EQUAL(false, Util::isNumber(" "));
}

void UtilTest::testIsLowercase()
{
  CPPUNIT_ASSERT_EQUAL(true, Util::isLowercase("alpha"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isLowercase("Alpha"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isLowercase("1alpha"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isLowercase(""));
  CPPUNIT_ASSERT_EQUAL(false, Util::isLowercase(" "));
}

void UtilTest::testIsUppercase()
{
  CPPUNIT_ASSERT_EQUAL(true, Util::isUppercase("ALPHA"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isUppercase("Alpha"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isUppercase("1ALPHA"));
  CPPUNIT_ASSERT_EQUAL(false, Util::isUppercase(""));
  CPPUNIT_ASSERT_EQUAL(false, Util::isUppercase(" "));
}

void UtilTest::testAlphaToNum()
{
  CPPUNIT_ASSERT_EQUAL(0U, Util::alphaToNum("a"));
  CPPUNIT_ASSERT_EQUAL(0U, Util::alphaToNum("aa"));
  CPPUNIT_ASSERT_EQUAL(1U, Util::alphaToNum("b"));
  CPPUNIT_ASSERT_EQUAL(675U, Util::alphaToNum("zz")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(675U, Util::alphaToNum("ZZ")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(0U, Util::alphaToNum(""));
  CPPUNIT_ASSERT_EQUAL(4294967295U, Util::alphaToNum("NXMRLXV"));
  CPPUNIT_ASSERT_EQUAL(0U, Util::alphaToNum("NXMRLXW")); // uint32_t overflow
}

void UtilTest::testMkdirs()
{
  std::string dir = "/tmp/aria2-UtilTest-testMkdirs";
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  Util::mkdirs(dir);
  CPPUNIT_ASSERT(d.isDir());

  std::string file = "./UtilTest.cc";
  File f(file);
  CPPUNIT_ASSERT(f.isFile());
  try {
    Util::mkdirs(file);
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
  Util::convertBitfield(&destBitfield, &srcBitfield);
  
  CPPUNIT_ASSERT_EQUAL(std::string("9fffffffffffffffffffffffffffffff80"),
		       Util::toHex(destBitfield.getBitfield(),
				   destBitfield.getBitfieldLength()));
}

void UtilTest::testParseIntRange()
{
  IntSequence seq = Util::parseIntRange("1,3-8,10");

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
    IntSequence seq = Util::parseIntRange("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = Util::parseIntRange("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = Util::parseIntRange("2147483647-2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = Util::parseIntRange("1-2x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    IntSequence seq = Util::parseIntRange("3x-4");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseInt()
{
  CPPUNIT_ASSERT_EQUAL(-1, Util::parseInt(" -1 "));
  CPPUNIT_ASSERT_EQUAL(2147483647, Util::parseInt("2147483647"));
  try {
    Util::parseInt("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseInt("-2147483649");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseUInt()
{
  CPPUNIT_ASSERT_EQUAL(4294967295U, Util::parseUInt(" 4294967295 "));
  try {
    Util::parseUInt("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseUInt("4294967296");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseLLInt()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)-1LL, Util::parseLLInt(" -1 "));
  CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL,
		       Util::parseLLInt("9223372036854775807"));
  try {
    Util::parseLLInt("9223372036854775808");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseLLInt("-9223372036854775809");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseLLInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseLLInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest::testParseULLInt()
{
  CPPUNIT_ASSERT_EQUAL((uint64_t)18446744073709551615ULL,
		       Util::parseULLInt("18446744073709551615"));
  try {
    Util::parseUInt("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    Util::parseLLInt("18446744073709551616");
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

  std::string readData = Util::toString(dw);

  CPPUNIT_ASSERT_EQUAL(data, readData);
}

void UtilTest::testItos()
{
  {
    int i = 0;
    CPPUNIT_ASSERT_EQUAL(std::string("0"), Util::itos(i));
  }
  {
    int i = 100;
    CPPUNIT_ASSERT_EQUAL(std::string("100"), Util::itos(i, true));
  }
  {
    int i = 100;
    CPPUNIT_ASSERT_EQUAL(std::string("100"), Util::itos(i));
  }
  {
    int i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12,345"), Util::itos(i, true));
  }
  {
    int i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12345"), Util::itos(i));
  }
  {
    int i = -12345;
    CPPUNIT_ASSERT_EQUAL(std::string("-12,345"), Util::itos(i, true));
  }
  {
    int64_t i = INT64_MAX;
    CPPUNIT_ASSERT_EQUAL(std::string("9,223,372,036,854,775,807"), Util::itos(i, true));
  }  
}

void UtilTest::testUitos()
{
  {
    uint16_t i = 12345;
    CPPUNIT_ASSERT_EQUAL(std::string("12345"), Util::uitos(i));
  }
  {
    int16_t i = -12345;
    CPPUNIT_ASSERT_EQUAL(std::string("/.-,+"), Util::uitos(i));
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

void UtilTest::testUrlencode()
{
  CPPUNIT_ASSERT_EQUAL
    (std::string("%3A%2F%3F%23%5B%5D%40%21%25%26%27%28%29%2A%2B%2C%3B%3D"),
     Util::urlencode(":/?#[]@!%&'()*+,;="));

  std::string unreserved =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-._~";
  CPPUNIT_ASSERT_EQUAL(unreserved, Util::urlencode(unreserved));

  CPPUNIT_ASSERT_EQUAL(std::string("1%5EA%20"), Util::urlencode("1^A "));
}

void UtilTest::testHtmlEscape()
{
  CPPUNIT_ASSERT_EQUAL(std::string("aria2&lt;&gt;&quot;&#39;util"),
		       Util::htmlEscape("aria2<>\"'util"));
}

void UtilTest::testJoinPath()
{
  const std::string dir1dir2file[] = { "dir1", "dir2", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("dir1/dir2/file"),
     Util::joinPath(&dir1dir2file[0],
		    &dir1dir2file[arrayLength(dir1dir2file)]));

  const std::string dirparentfile[] = { "dir", "..", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("file"),
     Util::joinPath(&dirparentfile[0],
		    &dirparentfile[arrayLength(dirparentfile)]));

  const std::string dirparentparentfile[] = { "dir", "..", "..", "file" };
  CPPUNIT_ASSERT_EQUAL
    (std::string("file"),
     Util::joinPath(&dirparentparentfile[0],
		    &dirparentparentfile[arrayLength(dirparentparentfile)]));

  const std::string dirdotfile[] = { "dir", ".", "file" };
  CPPUNIT_ASSERT_EQUAL(std::string("dir/file"),
		       Util::joinPath(&dirdotfile[0],
				      &dirdotfile[arrayLength(dirdotfile)]));

  const std::string empty[] = {};
  CPPUNIT_ASSERT_EQUAL(std::string(""), Util::joinPath(&empty[0], &empty[0]));

  const std::string parentdot[] = { "..", "." };
  CPPUNIT_ASSERT_EQUAL(std::string(""),
		       Util::joinPath(&parentdot[0],
				      &parentdot[arrayLength(parentdot)]));
}

void UtilTest::testParseIndexPath()
{
  std::map<size_t, std::string>::value_type p = Util::parseIndexPath("1=foo");
  CPPUNIT_ASSERT_EQUAL((size_t)1, p.first);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), p.second);
  try {
    Util::parseIndexPath("1X=foo");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    // success
  }
  try {
    Util::parseIndexPath("1=");
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
  std::map<size_t, std::string> m = Util::createIndexPathMap(in);
  CPPUNIT_ASSERT_EQUAL((size_t)2, m.size());
  CPPUNIT_ASSERT(m.find(1) != m.end());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), m[1]);
  CPPUNIT_ASSERT(m.find(100) != m.end());
  CPPUNIT_ASSERT_EQUAL(std::string("/myhome/mypicture.png"), m[100]);
}

} // namespace aria2
