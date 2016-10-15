#include "util.h"

#include <cmath>
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

class UtilTest2 : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UtilTest2);
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
  CPPUNIT_TEST(testMkdirs);
  CPPUNIT_TEST(testConvertBitfield);
  CPPUNIT_TEST(testParseIntSegments);
  CPPUNIT_TEST(testParseIntSegments_invalidRange);
  CPPUNIT_TEST(testParseIntNoThrow);
  CPPUNIT_TEST(testParseUIntNoThrow);
  CPPUNIT_TEST(testParseLLIntNoThrow);
  CPPUNIT_TEST(testToString_binaryStream);
  CPPUNIT_TEST(testItos);
  CPPUNIT_TEST(testUitos);
  CPPUNIT_TEST(testNtoh64);
  CPPUNIT_TEST(testPercentEncode);
  CPPUNIT_TEST(testPercentEncodeMini);
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
  CPPUNIT_TEST(testParseDoubleNoThrow);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

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
  void testMkdirs();
  void testConvertBitfield();
  void testParseIntSegments();
  void testParseIntSegments_invalidRange();
  void testParseIntNoThrow();
  void testParseUIntNoThrow();
  void testParseLLIntNoThrow();
  void testToString_binaryStream();
  void testItos();
  void testUitos();
  void testNtoh64();
  void testPercentEncode();
  void testPercentEncodeMini();
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
  void testParseDoubleNoThrow();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTest2);

class Printer {
public:
  template <class T> void operator()(T t) { std::cerr << t << ", "; }
};

void UtilTest2::testToUpper()
{
  std::string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  std::string upp = "608CABC0F2FA18C260CAFD974516865C772363D5";

  CPPUNIT_ASSERT_EQUAL(upp, util::toUpper(src));
}

void UtilTest2::testToLower()
{
  std::string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  std::string upp = "608cabc0f2fa18c260cafd974516865c772363d5";

  CPPUNIT_ASSERT_EQUAL(upp, util::toLower(src));
}

void UtilTest2::testUppercase()
{
  std::string src = "608cabc0f2fa18c260cafd974516865c772363d5";
  std::string ans = "608CABC0F2FA18C260CAFD974516865C772363D5";
  util::uppercase(src);
  CPPUNIT_ASSERT_EQUAL(ans, src);
}

void UtilTest2::testLowercase()
{
  std::string src = "608CABC0F2FA18C260CAFD974516865C772363D5";
  std::string ans = "608cabc0f2fa18c260cafd974516865c772363d5";
  util::lowercase(src);
  CPPUNIT_ASSERT_EQUAL(ans, src);
}

void UtilTest2::testPercentDecode()
{
  std::string src = "http://aria2.sourceforge.net/aria2%200.7.0%20docs.html";
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://aria2.sourceforge.net/aria2 0.7.0 docs.html"),
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

void UtilTest2::testGetRealSize()
{
  CPPUNIT_ASSERT_EQUAL((int64_t)4_g, util::getRealSize("4096M"));
  CPPUNIT_ASSERT_EQUAL((int64_t)1_k, util::getRealSize("1K"));
  CPPUNIT_ASSERT_EQUAL((int64_t)4_g, util::getRealSize("4096m"));
  CPPUNIT_ASSERT_EQUAL((int64_t)1_k, util::getRealSize("1k"));
  try {
    util::getRealSize("");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("foo");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("9223372036854775807K");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace();
  }
  try {
    util::getRealSize("9223372036854775807M");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace();
  }
}

void UtilTest2::testAbbrevSize()
{
  CPPUNIT_ASSERT_EQUAL(std::string("8,589,934,591Gi"),
                       util::abbrevSize(9223372036854775807LL));
  CPPUNIT_ASSERT_EQUAL(std::string("4.0Gi"), util::abbrevSize(4_g));
  CPPUNIT_ASSERT_EQUAL(std::string("1.0Ki"), util::abbrevSize(1_k));
  CPPUNIT_ASSERT_EQUAL(std::string("0.9Ki"), util::abbrevSize(1023));
  CPPUNIT_ASSERT_EQUAL(std::string("511"), util::abbrevSize(511));
  CPPUNIT_ASSERT_EQUAL(std::string("0"), util::abbrevSize(0));
  CPPUNIT_ASSERT_EQUAL(std::string("1.1Ki"), util::abbrevSize(1127));
  CPPUNIT_ASSERT_EQUAL(std::string("1.5Mi"), util::abbrevSize(1572864));
}

void UtilTest2::testToStream()
{
  std::ostringstream os;
  std::shared_ptr<FileEntry> f1(new FileEntry("aria2.tar.bz2", 12300, 0));
  std::shared_ptr<FileEntry> f2(new FileEntry("aria2.txt", 556, 0));
  std::deque<std::shared_ptr<FileEntry>> entries;
  entries.push_back(f1);
  entries.push_back(f2);
  const char* filename = A2_TEST_OUT_DIR "/aria2_UtilTest2_testToStream";
  BufferedFile fp(filename, BufferedFile::WRITE);
  util::toStream(entries.begin(), entries.end(), fp);
  fp.close();
  CPPUNIT_ASSERT_EQUAL(std::string("Files:\n"
                                   "idx|path/length\n"
                                   "===+======================================="
                                   "====================================\n"
                                   "  1|aria2.tar.bz2\n"
                                   "   |12KiB (12,300)\n"
                                   "---+---------------------------------------"
                                   "------------------------------------\n"
                                   "  2|aria2.txt\n"
                                   "   |556B (556)\n"
                                   "---+---------------------------------------"
                                   "------------------------------------\n"),
                       readFile(filename));
}

void UtilTest2::testIsNumber()
{
  std::string s = "000";
  CPPUNIT_ASSERT_EQUAL(true, util::isNumber(s.begin(), s.end()));
  s = "a";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = "0a";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = "";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
  s = " ";
  CPPUNIT_ASSERT_EQUAL(false, util::isNumber(s.begin(), s.end()));
}

void UtilTest2::testIsLowercase()
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

void UtilTest2::testIsUppercase()
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

void UtilTest2::testMkdirs()
{
  std::string dir = A2_TEST_OUT_DIR "/aria2-UtilTest2-testMkdirs";
  File d(dir);
  if (d.exists()) {
    CPPUNIT_ASSERT(d.remove());
  }
  CPPUNIT_ASSERT(!d.exists());
  util::mkdirs(dir);
  CPPUNIT_ASSERT(d.isDir());

  std::string file = A2_TEST_DIR "/UtilTest2.cc";
  File f(file);
  CPPUNIT_ASSERT(f.isFile());
  try {
    util::mkdirs(file);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlAbortEx& ex) {
    std::cerr << ex.stackTrace() << std::endl;
  }
}

void UtilTest2::testConvertBitfield()
{
  BitfieldMan srcBitfield(384_k, 256_k * 256 + 1);
  BitfieldMan destBitfield(512_k, srcBitfield.getTotalLength());
  srcBitfield.setAllBit();
  srcBitfield.unsetBit(2); // <- range [768, 1152)
  // which corresponds to the index [1,2] in destBitfield
  util::convertBitfield(&destBitfield, &srcBitfield);

  CPPUNIT_ASSERT_EQUAL(std::string("9fffffffffffffffffffffffffffffff80"),
                       util::toHex(destBitfield.getBitfield(),
                                   destBitfield.getBitfieldLength()));
}

void UtilTest2::testParseIntSegments()
{
  {
    auto sgl = util::parseIntSegments("1,3-8,10");

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
  }
  {
    auto sgl = util::parseIntSegments(",,,1,,,3,,,");
    CPPUNIT_ASSERT_EQUAL(1, sgl.next());
    CPPUNIT_ASSERT_EQUAL(3, sgl.next());
    CPPUNIT_ASSERT(!sgl.hasNext());
  }
}

void UtilTest2::testParseIntSegments_invalidRange()
{
  try {
    auto sgl = util::parseIntSegments("-1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    auto sgl = util::parseIntSegments("1-");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    auto sgl = util::parseIntSegments("2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    auto sgl = util::parseIntSegments("2147483647-2147483648");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    auto sgl = util::parseIntSegments("1-2x");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    auto sgl = util::parseIntSegments("3x-4");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
}

void UtilTest2::testParseIntNoThrow()
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

void UtilTest2::testParseUIntNoThrow()
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

void UtilTest2::testParseLLIntNoThrow()
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

void UtilTest2::testToString_binaryStream()
{
  std::shared_ptr<DiskWriter> dw(new ByteArrayDiskWriter());
  std::string data(16_k + 256, 'a');
  dw->initAndOpenFile();
  dw->writeData((const unsigned char*)data.c_str(), data.size(), 0);

  std::string readData = util::toString(dw);

  CPPUNIT_ASSERT_EQUAL(data, readData);
}

void UtilTest2::testItos()
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

void UtilTest2::testUitos()
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

void UtilTest2::testNtoh64()
{
  uint64_t x = 0xff00ff00ee00ee00LL;
#ifdef WORDS_BIGENDIAN
  CPPUNIT_ASSERT_EQUAL(x, ntoh64(x));
  CPPUNIT_ASSERT_EQUAL(x, hton64(x));
#else  // !WORDS_BIGENDIAN
  uint64_t y = 0x00ee00ee00ff00ffLL;
  CPPUNIT_ASSERT_EQUAL(y, ntoh64(x));
  CPPUNIT_ASSERT_EQUAL(x, hton64(y));
#endif // !WORDS_BIGENDIAN
}

void UtilTest2::testPercentEncode()
{
  CPPUNIT_ASSERT_EQUAL(
      std::string("%3A%2F%3F%23%5B%5D%40%21%25%26%27%28%29%2A%2B%2C%3B%3D"),
      util::percentEncode(":/?#[]@!%&'()*+,;="));

  std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789"
                           "-._~";
  CPPUNIT_ASSERT_EQUAL(unreserved, util::percentEncode(unreserved));

  CPPUNIT_ASSERT_EQUAL(std::string("1%5EA%20"), util::percentEncode("1^A "));
}

void UtilTest2::testPercentEncodeMini()
{
  CPPUNIT_ASSERT_EQUAL(std::string("%80"),
                       util::percentEncodeMini({(char)0x80}));
}

void UtilTest2::testHtmlEscape()
{
  CPPUNIT_ASSERT_EQUAL(std::string("aria2&lt;&gt;&quot;&#39;util"),
                       util::htmlEscape("aria2<>\"'util"));
}

void UtilTest2::testJoinPath()
{
  const std::string dir1dir2file[] = {"dir1", "dir2", "file"};
  CPPUNIT_ASSERT_EQUAL(
      std::string("dir1/dir2/file"),
      util::joinPath(std::begin(dir1dir2file), std::end(dir1dir2file)));

  const std::string dirparentfile[] = {"dir", "..", "file"};
  CPPUNIT_ASSERT_EQUAL(
      std::string("file"),
      util::joinPath(std::begin(dirparentfile), std::end(dirparentfile)));

  const std::string dirparentparentfile[] = {"dir", "..", "..", "file"};
  CPPUNIT_ASSERT_EQUAL(std::string("file"),
                       util::joinPath(std::begin(dirparentparentfile),
                                      std::end(dirparentparentfile)));

  const std::string dirdotfile[] = {"dir", ".", "file"};
  CPPUNIT_ASSERT_EQUAL(
      std::string("dir/file"),
      util::joinPath(std::begin(dirdotfile), std::end(dirdotfile)));

  const std::string empty[] = {};
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::joinPath(&empty[0], &empty[0]));

  const std::string parentdot[] = {"..", "."};
  CPPUNIT_ASSERT_EQUAL(std::string(""), util::joinPath(std::begin(parentdot),
                                                       std::end(parentdot)));
}

void UtilTest2::testParseIndexPath()
{
  std::pair<size_t, std::string> p = util::parseIndexPath("1=foo");
  CPPUNIT_ASSERT_EQUAL((size_t)1, p.first);
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), p.second);
  try {
    util::parseIndexPath("1X=foo");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    // success
  }
  try {
    util::parseIndexPath("1=");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    // success
  }
}

void UtilTest2::testCreateIndexPaths()
{
  std::stringstream in("1=/tmp/myfile\n"
                       "100=/myhome/mypicture.png\n");
  std::vector<std::pair<size_t, std::string>> m = util::createIndexPaths(in);
  CPPUNIT_ASSERT_EQUAL((size_t)2, m.size());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m[0].first);
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/myfile"), m[0].second);
  CPPUNIT_ASSERT_EQUAL((size_t)100, m[1].first);
  CPPUNIT_ASSERT_EQUAL(std::string("/myhome/mypicture.png"), m[1].second);
}

void UtilTest2::testGenerateRandomData()
{
  using namespace std;

  // Simple sanity check
  unsigned char data1[25];
  memset(data1, 0, sizeof(data1));
  util::generateRandomData(data1, sizeof(data1));

  unsigned char data2[25];
  memset(data2, 0, sizeof(data2));
  util::generateRandomData(data2, sizeof(data2));

  CPPUNIT_ASSERT(memcmp(data1, data2, sizeof(data1)) != 0);

  // Simple stddev/mean tests
  map<uint8_t, size_t> counts;
  uint8_t bytes[1 << 20];
  for (auto i = 0; i < 10; ++i) {
    util::generateRandomData(bytes, sizeof(bytes));
    for (auto b : bytes) {
      counts[b]++;
    }
  }
  CPPUNIT_ASSERT_MESSAGE("Should see all kinds of bytes", counts.size() == 256);
  double sum =
      accumulate(counts.begin(), counts.end(), 0.0,
                 [](double total, const decltype(counts)::value_type& elem) {
                   return total + elem.second;
                 });
  double mean = sum / counts.size();
  vector<double> diff(counts.size());
  transform(counts.begin(), counts.end(), diff.begin(),
            [&](const decltype(counts)::value_type& elem) -> double {
              return (double)elem.second - mean;
            });
  double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  double stddev = sqrt(sq_sum / counts.size());
  cout << "stddev: " << fixed << stddev << endl;
  CPPUNIT_ASSERT_MESSAGE("stddev makes sense (lower)", stddev <= 320);
  CPPUNIT_ASSERT_MESSAGE("stddev makes sense (upper)", stddev >= 100);
}

void UtilTest2::testFromHex()
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

void UtilTest2::testParsePrioritizePieceRange()
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
  constexpr size_t pieceLength = 1_k;
  std::vector<std::shared_ptr<FileEntry>> entries(4,
                                                  std::shared_ptr<FileEntry>());
  entries[0].reset(new FileEntry("file1", 1024, 0));
  entries[1].reset(new FileEntry("file2", 2560, entries[0]->getLastOffset()));
  entries[2].reset(new FileEntry("file3", 0, entries[1]->getLastOffset()));
  entries[3].reset(new FileEntry("file4", 3584, entries[2]->getLastOffset()));

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
  util::parsePrioritizePieceRange(result, "head", entries, pieceLength, 1_k);
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
  util::parsePrioritizePieceRange(result, "tail", entries, pieceLength, 1_k);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)2, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head=1,tail=1", entries,
                                  pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)4, result.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, result[0]);
  CPPUNIT_ASSERT_EQUAL((size_t)1, result[1]);
  CPPUNIT_ASSERT_EQUAL((size_t)3, result[2]);
  CPPUNIT_ASSERT_EQUAL((size_t)6, result[3]);
  result.clear();
  util::parsePrioritizePieceRange(result, "head=300M,tail=300M", entries,
                                  pieceLength);
  CPPUNIT_ASSERT_EQUAL((size_t)7, result.size());
  for (size_t i = 0; i < 7; ++i) {
    CPPUNIT_ASSERT_EQUAL(i, result[i]);
  }
  result.clear();
  util::parsePrioritizePieceRange(result, "", entries, pieceLength);
  CPPUNIT_ASSERT(result.empty());
}

void UtilTest2::testApplyDir()
{
  CPPUNIT_ASSERT_EQUAL(std::string("./pred"), util::applyDir("", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("/pred"), util::applyDir("/", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("./pred"), util::applyDir(".", "pred"));
  CPPUNIT_ASSERT_EQUAL(std::string("/dl/pred"), util::applyDir("/dl", "pred"));
}

void UtilTest2::testFixTaintedBasename()
{
  CPPUNIT_ASSERT_EQUAL(std::string("a%2Fb"), util::fixTaintedBasename("a/b"));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("a%5Cb"), util::fixTaintedBasename("a\\b"));
#else  // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("a\\b"), util::fixTaintedBasename("a\\b"));
#endif // !__MINGW32__
}

void UtilTest2::testIsNumericHost()
{
  CPPUNIT_ASSERT(util::isNumericHost("192.168.0.1"));
  CPPUNIT_ASSERT(!util::isNumericHost("aria2.sf.net"));
  CPPUNIT_ASSERT(util::isNumericHost("::1"));
}

void UtilTest2::testDetectDirTraversal()
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

void UtilTest2::testEscapePath()
{
  CPPUNIT_ASSERT_EQUAL(std::string("foo%00bar%00%01"),
                       util::escapePath(std::string("foo") + (char)0x00 +
                                        std::string("bar") + (char)0x00 +
                                        (char)0x01));
#ifdef __MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo%5Cbar"), util::escapePath("foo\\bar"));
#else  // !__MINGW32__
  CPPUNIT_ASSERT_EQUAL(std::string("foo\\bar"), util::escapePath("foo\\bar"));
#endif // !__MINGW32__
}

void UtilTest2::testInSameCidrBlock()
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

void UtilTest2::testIsUtf8String()
{
  CPPUNIT_ASSERT(util::isUtf8("ascii"));
  // "Hello World" in Japanese UTF-8
  CPPUNIT_ASSERT(
      util::isUtf8(fromHex("e38193e38293e381abe381a1e381afe4b896e7958c")));
  // "World" in Shift_JIS
  CPPUNIT_ASSERT(!util::isUtf8(fromHex("90a28a") + "E"));
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

void UtilTest2::testNextParam()
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

void UtilTest2::testNoProxyDomainMatch()
{
  CPPUNIT_ASSERT(util::noProxyDomainMatch("localhost", "localhost"));
  CPPUNIT_ASSERT(util::noProxyDomainMatch("192.168.0.1", "192.168.0.1"));
  CPPUNIT_ASSERT(util::noProxyDomainMatch("www.example.org", ".example.org"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("www.example.org", "example.org"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("192.168.0.1", "0.1"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("example.org", "example.com"));
  CPPUNIT_ASSERT(!util::noProxyDomainMatch("example.org", "www.example.org"));
}

void UtilTest2::testInPrivateAddress()
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

void UtilTest2::testSecfmt()
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

void UtilTest2::testTlsHostnameMatch()
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

void UtilTest2::testParseDoubleNoThrow()
{
  double n;

  CPPUNIT_ASSERT(util::parseDoubleNoThrow(n, " 123 "));
  CPPUNIT_ASSERT_EQUAL(123., n);

  CPPUNIT_ASSERT(util::parseDoubleNoThrow(n, "3.14"));
  CPPUNIT_ASSERT_EQUAL(3.14, n);

  CPPUNIT_ASSERT(util::parseDoubleNoThrow(n, "-3.14"));
  CPPUNIT_ASSERT_EQUAL(-3.14, n);

  CPPUNIT_ASSERT(!util::parseDoubleNoThrow(n, ""));
  CPPUNIT_ASSERT(!util::parseDoubleNoThrow(n, "123x"));
}

} // namespace aria2
