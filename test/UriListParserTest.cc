#include "UriListParser.h"

#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "prefs.h"
#include "OptionHandler.h"
#include "a2io.h"

namespace aria2 {

class UriListParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UriListParserTest);
  CPPUNIT_TEST(testHasNext);
  CPPUNIT_TEST(testStdinPipeNonBlocking);
  CPPUNIT_TEST(testStdinPipeFullFormat);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string list2String(const std::vector<std::string>& src);

public:
  void setUp() {}

  void testHasNext();

  void testStdinPipeNonBlocking();

  void testStdinPipeFullFormat();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UriListParserTest);

std::string UriListParserTest::list2String(const std::vector<std::string>& src)
{
  std::ostringstream strm;
  std::copy(src.begin(), src.end(),
            std::ostream_iterator<std::string>(strm, " "));
  return util::strip(strm.str());
}

void UriListParserTest::testHasNext()
{
  std::string filename = A2_TEST_DIR "/filelist1.txt";

  UriListParser flp(filename);

  std::vector<std::string> uris;
  Option reqOp;

  CPPUNIT_ASSERT(flp.hasNext());

  flp.parseNext(uris, reqOp);
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://localhost/index.html http://localhost2/index.html"),
      list2String(uris));

  uris.clear();
  reqOp.clear();

  CPPUNIT_ASSERT(flp.hasNext());

  flp.parseNext(uris, reqOp);
  CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.tar.bz2"),
                       list2String(uris));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), reqOp.get(PREF_DIR));
  CPPUNIT_ASSERT_EQUAL(std::string("chunky_chocolate"), reqOp.get(PREF_OUT));

  uris.clear();
  reqOp.clear();

  CPPUNIT_ASSERT(!flp.hasNext());

  flp.parseNext(uris, reqOp);
  CPPUNIT_ASSERT_EQUAL(std::string(""), list2String(uris));

  CPPUNIT_ASSERT(!flp.hasNext());
}

void UriListParserTest::testStdinPipeNonBlocking()
{
  int fds[2];
  CPPUNIT_ASSERT(pipe(fds) == 0);

  // Save original stdin
  int savedStdin = dup(STDIN_FILENO);
  CPPUNIT_ASSERT(savedStdin >= 0);

  // Redirect stdin to the read end of the pipe
  CPPUNIT_ASSERT(dup2(fds[0], STDIN_FILENO) != -1);
  close(fds[0]);

  {
    UriListParser parser(DEV_STDIN);

    // No data yet — inputReady() should return false so the event loop
    // does not block on an empty pipe.
    CPPUNIT_ASSERT(!parser.inputReady());

    // Write a complete URI line into the pipe
    const char* line = "http://example.com/test\n\n";
    CPPUNIT_ASSERT(::write(fds[1], line, strlen(line)) > 0);

    // Now data is available — inputReady() should return true
    CPPUNIT_ASSERT(parser.inputReady());

    // Close write end so parseNext can reach EOF instead of blocking.
    close(fds[1]);
    fds[1] = -1;

    std::vector<std::string> uris;
    Option op;
    parser.parseNext(uris, op);
    CPPUNIT_ASSERT_EQUAL(std::string("http://example.com/test"), uris[0]);
  }

  // Restore stdin and clean up
  dup2(savedStdin, STDIN_FILENO);
  close(savedStdin);
  if (fds[1] != -1) {
    close(fds[1]);
  }
  // BufferedFile uses the global stdin FILE*; clear any EOF/error
  // flags left over from the previous pipe so the next test starts
  // with a clean stream.
  clearerr(stdin);
}

void UriListParserTest::testStdinPipeFullFormat()
{
  int fds[2];
  CPPUNIT_ASSERT(pipe(fds) == 0);

  int savedStdin = dup(STDIN_FILENO);
  CPPUNIT_ASSERT(savedStdin >= 0);

  CPPUNIT_ASSERT(dup2(fds[0], STDIN_FILENO) != -1);
  close(fds[0]);
  // Clear any stale EOF/error state on the global stdin FILE* before
  // constructing a new UriListParser that wraps it.
  clearerr(stdin);

  {
    UriListParser parser(DEV_STDIN);

    // Full format: multiple URLs, options, comments, blank-line separators
    const char* data =
        "# comment line\n"
        "http://localhost/index.html\thttp://localhost2/index.html\n"
        "\n"
        "ftp://localhost/aria2.tar.bz2\n"
        "  dir=/tmp\n"
        "# comment line\n"
        "\t out=chunky_chocolate\n"
        "\n";

    // Before writing anything, inputReady() should be false
    CPPUNIT_ASSERT(!parser.inputReady());

    // Write the full format data into the pipe
    CPPUNIT_ASSERT(::write(fds[1], data, strlen(data)) > 0);

    // Data is available
    CPPUNIT_ASSERT(parser.inputReady());

    // Close write end so parseNext can reach EOF instead of blocking
    // when it runs past the end of the input.
    close(fds[1]);
    fds[1] = -1;

    // Parse first entry: two tab-separated URLs, no options
    std::vector<std::string> uris;
    Option op;
    parser.parseNext(uris, op);
    CPPUNIT_ASSERT_EQUAL(
        std::string("http://localhost/index.html http://localhost2/index.html"),
        list2String(uris));

    CPPUNIT_ASSERT(parser.hasNext());

    // Parse second entry: one URL with options
    uris.clear();
    op.clear();
    parser.parseNext(uris, op);
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://localhost/aria2.tar.bz2"),
                         list2String(uris));
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), op.get(PREF_DIR));
    CPPUNIT_ASSERT_EQUAL(std::string("chunky_chocolate"), op.get(PREF_OUT));

    CPPUNIT_ASSERT(!parser.hasNext());
  }

  dup2(savedStdin, STDIN_FILENO);
  close(savedStdin);
  close(fds[1]);
}

} // namespace aria2
