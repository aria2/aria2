#include "Util.h"
#include "FixedNumberRandomizer.h"
#include "DlAbortEx.h"
#include "BitfieldMan.h"
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
  CPPUNIT_TEST(testRandomAlpha);
  CPPUNIT_TEST(testToUpper);
  CPPUNIT_TEST(testToLower);
  CPPUNIT_TEST(testUrldecode);
  CPPUNIT_TEST(testCountBit);
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
  CPPUNIT_TEST(testParseLLInt);
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
  void testCountBit();
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
  void testParseLLInt();
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
  Util::slice(v1, "name1=value1; name2=value2; name3=value3;", ';', true);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  v1.clear();
  Util::slice(v1, "name1=value1; name2=value2; name3=value3", ';', true);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  Strings::iterator itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string("name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string("name3=value3"), *itr++);

  v1.clear();

  Util::slice(v1, "name1=value1; name2=value2; name3=value3", ';', false);
  CPPUNIT_ASSERT_EQUAL(3, (int)v1.size());
  itr = v1.begin();
  CPPUNIT_ASSERT_EQUAL(string("name1=value1"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string(" name2=value2"), *itr++);
  CPPUNIT_ASSERT_EQUAL(string(" name3=value3"), *itr++);
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

  string h5 = "attachment; filename=aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), Util::getContentDispositionFilename(h5));

  string h6 = "attachment; filename='aria2.tar.bz2'";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), Util::getContentDispositionFilename(h6));

  string h7 = "attachment; filename='aria2.tar.bz2";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), Util::getContentDispositionFilename(h7));

  string h8 = "attachment; filename=aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2"), Util::getContentDispositionFilename(h8));

  string h9 = "attachment; filename=\"aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT\"";
  CPPUNIT_ASSERT_EQUAL(string("aria2.tar.bz2; creation-date=20 Jun 2007 00:00:00 GMT"), Util::getContentDispositionFilename(h9));

  string h10 = "attachment; filename=";
  CPPUNIT_ASSERT_EQUAL(string(""), Util::getContentDispositionFilename(h10));

  string h11 = "attachment; filename=;";
  CPPUNIT_ASSERT_EQUAL(string(""), Util::getContentDispositionFilename(h11));

}

class Printer {
public:
  template<class T>
  void operator()(T t) {
    cerr << t << ", ";
  }
};

void UtilTest::testRandomAlpha() {
  string s = Util::randomAlpha(8, new FixedNumberRandomizer());
  CPPUNIT_ASSERT_EQUAL(string("AAAAAAAA"), s);
}

void UtilTest::testToUpper() {
  string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  string upp = "608CABC0F2FA18C260CAFD974516865C772363D5";

  CPPUNIT_ASSERT_EQUAL(upp, Util::toUpper(src));
}

void UtilTest::testToLower() {
  string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  string upp = "608cabc0f2fa18c260cafd974516865c772363d5";

  CPPUNIT_ASSERT_EQUAL(upp, Util::toLower(src));
}

#include "SharedHandle.h"

void UtilTest::testUrldecode() {
  string src = "http://aria2.sourceforge.net/aria2%200.7.0%20docs.html";
  CPPUNIT_ASSERT_EQUAL(string("http://aria2.sourceforge.net/aria2 0.7.0 docs.html"),
		       Util::urldecode(src));

  string src2 = "aria2+aria2";
  CPPUNIT_ASSERT_EQUAL(string("aria2+aria2"), Util::urldecode(src2));

  string src3 = "%5t%20";
  CPPUNIT_ASSERT_EQUAL(string("%5t "), Util::urldecode(src3));

  string src4 = "%";
  CPPUNIT_ASSERT_EQUAL(string("%"), Util::urldecode(src4));
  
  string src5 = "%3";
  CPPUNIT_ASSERT_EQUAL(string("%3"), Util::urldecode(src5));

  string src6 = "%2f";
  CPPUNIT_ASSERT_EQUAL(string("/"), Util::urldecode(src6));
}

void UtilTest::testCountBit() {
  CPPUNIT_ASSERT_EQUAL((int32_t)32, Util::countBit(UINT32_MAX));
  CPPUNIT_ASSERT_EQUAL((int32_t)8, Util::countBit(255));
}

void UtilTest::testGetRealSize()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, Util::getRealSize("4096M"));
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, Util::getRealSize("1K"));
  try {
    Util::getRealSize("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::getRealSize("foo");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::getRealSize("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::getRealSize("9223372036854775807K");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::getRealSize("9223372036854775807M");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
}

void UtilTest::testAbbrevSize()
{
  CPPUNIT_ASSERT_EQUAL(string("4,096.0Mi"), Util::abbrevSize(4294967296LL));
  CPPUNIT_ASSERT_EQUAL(string("1.0Ki"), Util::abbrevSize(1024));
  CPPUNIT_ASSERT_EQUAL(string("1,023"), Util::abbrevSize(1023));
  CPPUNIT_ASSERT_EQUAL(string("0"), Util::abbrevSize(0));
  CPPUNIT_ASSERT_EQUAL(string("1.1Ki"), Util::abbrevSize(1127));
  CPPUNIT_ASSERT_EQUAL(string("1.5Mi"), Util::abbrevSize(1572864));

}

void UtilTest::testToStream()
{
  ostringstream os;
  FileEntryHandle f1 = new FileEntry("aria2.tar.bz2", 12300, 0);
  FileEntryHandle f2 = new FileEntry("aria2.txt", 556, 0);
  FileEntries entries;
  entries.push_back(f1);
  entries.push_back(f2);
  Util::toStream(os, entries);
  CPPUNIT_ASSERT_EQUAL(
		       string("Files:\n"
			      "idx|path/length\n"
			      "===+===========================================================================\n"
			      "  1|aria2.tar.bz2\n"
			      "   |12.0KiB\n"
			      "---+---------------------------------------------------------------------------\n"
			      "  2|aria2.txt\n"
			      "   |556B\n"
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
  CPPUNIT_ASSERT_EQUAL((int32_t)0, Util::alphaToNum("a"));
  CPPUNIT_ASSERT_EQUAL((int32_t)0, Util::alphaToNum("aa"));
  CPPUNIT_ASSERT_EQUAL((int32_t)1, Util::alphaToNum("b"));
  CPPUNIT_ASSERT_EQUAL((int32_t)675, Util::alphaToNum("zz")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL((int32_t)675, Util::alphaToNum("ZZ")); // 25*26+25
  CPPUNIT_ASSERT_EQUAL((int32_t)0, Util::alphaToNum(""));
}

void UtilTest::testMkdirs()
{
  string dir = "/tmp/aria2-UtilTest-testMkdirs";
  File d(dir);
  if(d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  Util::mkdirs(dir);
  CPPUNIT_ASSERT(d.isDir());

  string file = "./UtilTest.cc";
  File f(file);
  CPPUNIT_ASSERT(f.isFile());
  try {
    Util::mkdirs(file);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx* ex) {
    cerr << ex->getMsg() << endl;
    delete ex;
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
  
  CPPUNIT_ASSERT_EQUAL(string("9fffffffffffffffffffffffffffffff80"),
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
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    IntSequence seq = Util::parseIntRange("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    IntSequence seq = Util::parseIntRange("2147483647-2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    IntSequence seq = Util::parseIntRange("1-2x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    IntSequence seq = Util::parseIntRange("3x-4");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
}

void UtilTest::testParseInt()
{
  CPPUNIT_ASSERT_EQUAL((int32_t)-1, Util::parseInt("-1"));
  CPPUNIT_ASSERT_EQUAL((int32_t)2147483647, Util::parseInt("2147483647"));
  try {
    Util::parseInt("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseInt("-2147483649");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
}

void UtilTest::testParseLLInt()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)-1, Util::parseLLInt("-1"));
  CPPUNIT_ASSERT_EQUAL((int64_t)9223372036854775807LL,
		       Util::parseLLInt("9223372036854775807"));
  try {
    Util::parseLLInt("9223372036854775808");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseLLInt("-9223372036854775809");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseLLInt("12x");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
  try {
    Util::parseLLInt("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    cerr << *e;
    delete e;
  }
}
