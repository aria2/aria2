#include "DefaultBtAnnounce.h"
#include "DefaultBtContext.h"
#include "Option.h"
#include "Util.h"
#include "Exception.h"
#include "MockBtContext.h"
#include "MockPieceStorage.h"
#include "MockPeerStorage.h"
#include "BtRuntime.h"
#include "AnnounceTier.h"
#include "FixedNumberRandomizer.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultBtAnnounceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtAnnounceTest);
  CPPUNIT_TEST(testGetAnnounceUrl);
  CPPUNIT_TEST(testNoMoreAnnounce);
  CPPUNIT_TEST(testIsAllAnnounceFailed);
  CPPUNIT_TEST(testURLOrderInStoppedEvent);
  CPPUNIT_TEST(testURLOrderInCompletedEvent);
  CPPUNIT_TEST(testProcessAnnounceResponse_malformed);
  CPPUNIT_TEST(testProcessAnnounceResponse_failureReason);
  CPPUNIT_TEST(testProcessAnnounceResponse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MockBtContext> _btContext;
  SharedHandle<MockPieceStorage> _pieceStorage;
  SharedHandle<MockPeerStorage> _peerStorage;
  SharedHandle<BtRuntime> _btRuntime;
  Option* _option;
public:
  DefaultBtAnnounceTest():_btContext(0),
			  _pieceStorage(0),
			  _peerStorage(0),
			  _btRuntime(0) {}

  void setUp() {
    _option = new Option();

    int64_t totalLength = 4*1024*1024;
    int32_t pieceLength = 256*1024;
    
    static const unsigned char infoHash[] = {  0x01, 0x23, 0x45, 0x67,
					       0x89, 0xab, 0xcd, 0xef,
					       0x01, 0x23, 0x45, 0x67,
					       0x89, 0xab, 0xcd, 0xef,
					       0x01, 0x23, 0x45, 0x67 };
    
    std::string peerId = "-aria2-ultrafastdltl";

    _btContext = new MockBtContext();
    _btContext->setInfoHash(infoHash);
    _btContext->setTotalLength(totalLength);
    _btContext->setPieceLength(pieceLength);
    _btContext->setPeerId((const unsigned char*)peerId.c_str());

    _pieceStorage = new MockPieceStorage();
    _pieceStorage->setTotalLength(totalLength);
    _pieceStorage->setCompletedLength(pieceLength*10);

    _peerStorage = new MockPeerStorage();
    TransferStat stat;
    stat.setSessionDownloadLength(pieceLength*5);
    stat.setSessionUploadLength(pieceLength*6);
    _peerStorage->setStat(stat);

    _btRuntime = new BtRuntime();
    _btRuntime->setListenPort(6989);
  }

  void tearDown()
  {
    delete _option;
  }

  void testGetAnnounceUrl();
  void testNoMoreAnnounce();
  void testIsAllAnnounceFailed();
  void testURLOrderInStoppedEvent();
  void testURLOrderInCompletedEvent();
  void testProcessAnnounceResponse_malformed();
  void testProcessAnnounceResponse_failureReason();
  void testProcessAnnounceResponse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtAnnounceTest);

void DefaultBtAnnounceTest::testNoMoreAnnounce()
{
  std::string trackerURI1 = "http://localhost/announce";
  std::deque<std::string> uris1;
  uris1.push_back(trackerURI1);
  SharedHandle<AnnounceTier> announceTier1 = new AnnounceTier(uris1);

  std::string trackerURI2 = "http://backup/announce";
  std::deque<std::string> uris2;
  uris2.push_back(trackerURI2);
  SharedHandle<AnnounceTier> announceTier2 = new AnnounceTier(uris2);


  _btContext->addAnnounceTier(announceTier1);
  _btContext->addAnnounceTier(announceTier2);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT(btAnnounce.noMoreAnnounce());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl()
{
  std::string trackerURI = "http://localhost/announce";
  std::deque<std::string> uris;
  uris.push_back(trackerURI);
  SharedHandle<AnnounceTier> announceTier = new AnnounceTier(uris);

  _btContext->addAnnounceTier(announceTier);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testIsAllAnnounceFailed()
{
  std::string trackerURI1 = "http://localhost/announce";
  std::deque<std::string> uris1;
  uris1.push_back(trackerURI1);
  SharedHandle<AnnounceTier> announceTier1 = new AnnounceTier(uris1);

  std::string trackerURI2 = "http://backup/announce";
  std::deque<std::string> uris2;
  uris2.push_back(trackerURI2);
  SharedHandle<AnnounceTier> announceTier2 = new AnnounceTier(uris2);


  _btContext->addAnnounceTier(announceTier1);
  _btContext->addAnnounceTier(announceTier2);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT(!btAnnounce.isAnnounceReady());
  CPPUNIT_ASSERT_EQUAL(std::string(""), btAnnounce.getAnnounceUrl());
  CPPUNIT_ASSERT(btAnnounce.isAllAnnounceFailed());
  
  btAnnounce.resetAnnounce();

  CPPUNIT_ASSERT(!btAnnounce.isAllAnnounceFailed());  
}

void DefaultBtAnnounceTest::testURLOrderInStoppedEvent()
{
  const char* urls[] = { "http://localhost1/announce",
			 "http://localhost2/announce" };
  SharedHandle<AnnounceTier> announceTier = new AnnounceTier(std::deque<std::string>(&urls[0], &urls[2]));

  _btContext->addAnnounceTier(announceTier);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testURLOrderInCompletedEvent()
{
  const char* urls[] = { "http://localhost1/announce",
			 "http://localhost2/announce" };
  SharedHandle<AnnounceTier> announceTier = new AnnounceTier(std::deque<std::string>(&urls[0], &urls[2]));

  _btContext->addAnnounceTier(announceTier);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_malformed()
{
  try {
    std::string res = "i123e";
    DefaultBtAnnounce(new MockBtContext(), _option).processAnnounceResponse(res.c_str(), res.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_failureReason()
{
  try {
    std::string res = "d14:failure reason11:hello worlde";
    DefaultBtAnnounce(new MockBtContext(), _option).processAnnounceResponse(res.c_str(), res.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse()
{
  std::string res = "d"
    "15:warning message11:hello world"
    "10:tracker id3:foo"
    "8:intervali3000e"
    "12:min intervali1800e"
    "8:completei100e"
    "10:incompletei200e"
    "e";
  
  DefaultBtAnnounce an(new MockBtContext(), _option);
  an.processAnnounceResponse(res.c_str(), res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), an.getTrackerID());
  CPPUNIT_ASSERT_EQUAL(3000, an.getInterval());
  CPPUNIT_ASSERT_EQUAL(1800, an.getMinInterval());
  CPPUNIT_ASSERT_EQUAL(100, an.getComplete());
  CPPUNIT_ASSERT_EQUAL(200, an.getIncomplete());
}

} // namespace aria2
