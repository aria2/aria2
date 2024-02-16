#include "ProtocolDetector.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class ProtocolDetectorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ProtocolDetectorTest);
  CPPUNIT_TEST(testIsStreamProtocol);
  CPPUNIT_TEST(testGuessTorrentFile);
  CPPUNIT_TEST(testGuessTorrentMagnet);
  CPPUNIT_TEST(testGuessMetalinkFile);
  CPPUNIT_TEST(testGuessAria2ControlFile);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testIsStreamProtocol();
  void testGuessTorrentFile();
  void testGuessTorrentMagnet();
  void testGuessMetalinkFile();
  void testGuessAria2ControlFile();
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
  CPPUNIT_ASSERT(detector.guessTorrentFile(A2_TEST_DIR "/test.torrent"));
  CPPUNIT_ASSERT(!detector.guessTorrentFile("http://localhost/test.torrent"));
  CPPUNIT_ASSERT(!detector.guessTorrentFile(A2_TEST_DIR "/test.xml"));
}

void ProtocolDetectorTest::testGuessTorrentMagnet()
{
  ProtocolDetector detector;
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT(detector.guessTorrentMagnet(
      "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c"));
  CPPUNIT_ASSERT(!detector.guessTorrentMagnet("magnet:?"));
#else  // !ENABLE_BITTORRENT
  CPPUNIT_ASSERT(!detector.guessTorrentMagnet(
      "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c"));
#endif // !ENABLE_BITTORRENT
}

void ProtocolDetectorTest::testGuessMetalinkFile()
{
  ProtocolDetector detector;
  CPPUNIT_ASSERT(detector.guessMetalinkFile(A2_TEST_DIR "/test.xml"));
  CPPUNIT_ASSERT(!detector.guessMetalinkFile("http://localhost/test.xml"));
  CPPUNIT_ASSERT(!detector.guessMetalinkFile(A2_TEST_DIR "/test.torrent"));
}

void ProtocolDetectorTest::testGuessAria2ControlFile()
{
  const ProtocolDetector detector;
  CPPUNIT_ASSERT(detector.guessAria2ControlFile(A2_TEST_DIR "/control_file.aria2"));
  CPPUNIT_ASSERT(!detector.guessAria2ControlFile(A2_TEST_DIR));
  CPPUNIT_ASSERT(detector.guessAria2ControlFile(A2_TEST_DIR "/control_file.aria"));
}

} // namespace aria2
