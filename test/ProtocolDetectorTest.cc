#include "ProtocolDetector.h"
#include "Exception.h"
#include "Util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ProtocolDetectorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ProtocolDetectorTest);
  CPPUNIT_TEST(testIsStreamProtocol);
  CPPUNIT_TEST(testGuessTorrentFile);
  CPPUNIT_TEST(testGuessMetalinkFile);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testIsStreamProtocol();
  void testGuessTorrentFile();
  void testGuessMetalinkFile();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ProtocolDetectorTest);

void ProtocolDetectorTest::testIsStreamProtocol()
{
  ProtocolDetector detector;
  CPPUNIT_ASSERT(detector.isStreamProtocol("http://localhost/index.html"));
  CPPUNIT_ASSERT(detector.isStreamProtocol("ftp://localhost/index.html"));
  CPPUNIT_ASSERT(!detector.isStreamProtocol("/home/web/localhost/index.html"));
}

void ProtocolDetectorTest::testGuessTorrentFile()
{
  ProtocolDetector detector;
  CPPUNIT_ASSERT(detector.guessTorrentFile("test.torrent"));
  CPPUNIT_ASSERT(!detector.guessTorrentFile("http://localhost/test.torrent"));
  CPPUNIT_ASSERT(!detector.guessTorrentFile("test.xml"));
}

void ProtocolDetectorTest::testGuessMetalinkFile()
{
  ProtocolDetector detector;
  CPPUNIT_ASSERT(detector.guessMetalinkFile("test.xml"));
  CPPUNIT_ASSERT(!detector.guessMetalinkFile("http://localhost/test.xml"));
  CPPUNIT_ASSERT(!detector.guessMetalinkFile("test.torrent"));
}

} // namespace aria2
