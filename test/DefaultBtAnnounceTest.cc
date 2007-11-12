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
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DefaultBtAnnounceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtAnnounceTest);
  CPPUNIT_TEST(testGetAnnounceUrl);
  CPPUNIT_TEST(testNoMoreAnnounce);
  CPPUNIT_TEST(testIsAllAnnounceFailed);
  CPPUNIT_TEST_SUITE_END();
private:
  MockBtContextHandle _btContext;
  MockPieceStorageHandle _pieceStorage;
  MockPeerStorageHandle _peerStorage;
  BtRuntimeHandle _btRuntime;
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
    
    string peerId = "-aria2-ultrafastdltl";

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
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtAnnounceTest);

void DefaultBtAnnounceTest::testNoMoreAnnounce()
{
  string trackerURI1 = "http://localhost/announce";
  Strings uris1;
  uris1.push_back(trackerURI1);
  AnnounceTierHandle announceTier1 = new AnnounceTier(uris1);

  string trackerURI2 = "http://backup/announce";
  Strings uris2;
  uris2.push_back(trackerURI2);
  AnnounceTierHandle announceTier2 = new AnnounceTier(uris2);


  _btContext->addAnnounceTier(announceTier1);
  _btContext->addAnnounceTier(announceTier2);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT(btAnnounce.noMoreAnnounce());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl()
{
  string trackerURI = "http://localhost/announce";
  Strings uris;
  uris.push_back(trackerURI);
  AnnounceTierHandle announceTier = new AnnounceTier(uris);

  _btContext->addAnnounceTier(announceTier);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped"), btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testIsAllAnnounceFailed()
{
  string trackerURI1 = "http://localhost/announce";
  Strings uris1;
  uris1.push_back(trackerURI1);
  AnnounceTierHandle announceTier1 = new AnnounceTier(uris1);

  string trackerURI2 = "http://backup/announce";
  Strings uris2;
  uris2.push_back(trackerURI2);
  AnnounceTierHandle announceTier2 = new AnnounceTier(uris2);


  _btContext->addAnnounceTier(announceTier1);
  _btContext->addAnnounceTier(announceTier2);

  DefaultBtAnnounce btAnnounce(_btContext, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(new FixedNumberRandomizer());
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(string("http://localhost/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(string("http://backup/announce?info_hash=%01%23Eg%89%ab%cd%ef%01%23Eg%89%ab%cd%ef%01%23Eg&peer_id=%2daria2%2dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT(!btAnnounce.isAnnounceReady());
  CPPUNIT_ASSERT_EQUAL(string(""), btAnnounce.getAnnounceUrl());
  CPPUNIT_ASSERT(btAnnounce.isAllAnnounceFailed());
  
  btAnnounce.resetAnnounce();

  CPPUNIT_ASSERT(!btAnnounce.isAllAnnounceFailed());  
}
