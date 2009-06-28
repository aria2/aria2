#include "DefaultBtAnnounce.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "Util.h"
#include "Exception.h"
#include "MockPieceStorage.h"
#include "MockPeerStorage.h"
#include "BtRuntime.h"
#include "AnnounceTier.h"
#include "FixedNumberRandomizer.h"
#include "FileEntry.h"
#include "prefs.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "array_fun.h"

namespace aria2 {

class DefaultBtAnnounceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtAnnounceTest);
  CPPUNIT_TEST(testGetAnnounceUrl);
  CPPUNIT_TEST(testGetAnnounceUrl_withQuery);
  CPPUNIT_TEST(testGetAnnounceUrl_externalIP);
  CPPUNIT_TEST(testNoMoreAnnounce);
  CPPUNIT_TEST(testIsAllAnnounceFailed);
  CPPUNIT_TEST(testURLOrderInStoppedEvent);
  CPPUNIT_TEST(testURLOrderInCompletedEvent);
  CPPUNIT_TEST(testProcessAnnounceResponse_malformed);
  CPPUNIT_TEST(testProcessAnnounceResponse_failureReason);
  CPPUNIT_TEST(testProcessAnnounceResponse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadContext> _dctx;
  SharedHandle<MockPieceStorage> _pieceStorage;
  SharedHandle<MockPeerStorage> _peerStorage;
  SharedHandle<BtRuntime> _btRuntime;
  Option* _option;
public:
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

    _dctx.reset(new DownloadContext(pieceLength, totalLength));
    BDE torrentAttrs = BDE::dict();
    torrentAttrs[bittorrent::INFO_HASH] =
      std::string(&infoHash[0], &infoHash[arrayLength(infoHash)]);
    _dctx->setAttribute(bittorrent::BITTORRENT, torrentAttrs);
    bittorrent::setStaticPeerId(peerId);

    _pieceStorage.reset(new MockPieceStorage());
    _pieceStorage->setTotalLength(totalLength);
    _pieceStorage->setCompletedLength(pieceLength*10);

    _peerStorage.reset(new MockPeerStorage());
    TransferStat stat;
    stat.setSessionDownloadLength(pieceLength*5);
    stat.setSessionUploadLength(pieceLength*6);
    _peerStorage->setStat(stat);

    _btRuntime.reset(new BtRuntime());
    _btRuntime->setListenPort(6989);
  }

  void tearDown()
  {
    delete _option;
  }

  void testGetAnnounceUrl();
  void testGetAnnounceUrl_withQuery();
  void testGetAnnounceUrl_externalIP();
  void testNoMoreAnnounce();
  void testIsAllAnnounceFailed();
  void testURLOrderInStoppedEvent();
  void testURLOrderInCompletedEvent();
  void testProcessAnnounceResponse_malformed();
  void testProcessAnnounceResponse_failureReason();
  void testProcessAnnounceResponse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtAnnounceTest);

template<typename InputIterator>
static BDE createAnnounceTier(InputIterator first, InputIterator last)
{
  BDE announceTier = BDE::list();
  for(; first != last; ++first) {
    announceTier << BDE(*first);
  }
  return announceTier;
}

static BDE createAnnounceTier(const std::string& uri)
{
  BDE announceTier = BDE::list();
  announceTier << uri;
  return announceTier;
}

static void setAnnounceList(const SharedHandle<DownloadContext>& dctx,
			    const BDE& announceList)
{
  dctx->getAttribute(bittorrent::BITTORRENT)[bittorrent::ANNOUNCE_LIST] =
    announceList;
}

void DefaultBtAnnounceTest::testNoMoreAnnounce()
{
  BDE announceList = BDE::list();
  announceList << createAnnounceTier("http://localhost/announce");
  announceList << createAnnounceTier("http://backup/announce");

  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT(btAnnounce.noMoreAnnounce());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl()
{

  BDE announceList = BDE::list();
  announceList << createAnnounceTier("http://localhost/announce");  
  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl_withQuery()
{
  BDE announceList = BDE::list();
  announceList << createAnnounceTier("http://localhost/announce?k=v");
  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL
    (std::string("http://localhost/announce?k=v&"
		 "info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&"
		 "peer_id=%2Daria2%2Dultrafastdltl&"
		 "uploaded=1572864&downloaded=1310720&left=1572864&compact=1&"
		 "key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&"
		 "supportcrypto=1"),
     btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl_externalIP()
{
  BDE announceList = BDE::list();
  announceList << createAnnounceTier("http://localhost/announce");
  setAnnounceList(_dctx, announceList);

  _option->put(PREF_BT_EXTERNAL_IP, "192.168.1.1");
  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL
    (std::string("http://localhost/announce?"
		 "info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&"
		 "peer_id=%2Daria2%2Dultrafastdltl&"
		 "uploaded=1572864&downloaded=1310720&left=1572864&compact=1&"
		 "key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&"
		 "supportcrypto=1&ip=192.168.1.1"),
     btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testIsAllAnnounceFailed()
{
  BDE announceList = BDE::list();
  announceList << createAnnounceTier("http://localhost/announce");
  announceList << createAnnounceTier("http://backup/announce");
  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

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

  BDE announceList = BDE::list();
  announceList << createAnnounceTier(&urls[0], &urls[arrayLength(urls)]);
  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _btRuntime->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testURLOrderInCompletedEvent()
{
  const char* urls[] = { "http://localhost1/announce",
			 "http://localhost2/announce" };

  BDE announceList = BDE::list();
  announceList << createAnnounceTier(&urls[0], &urls[arrayLength(urls)]);
  setAnnounceList(_dctx, announceList);

  DefaultBtAnnounce btAnnounce(_dctx, _option);
  btAnnounce.setPieceStorage(_pieceStorage);
  btAnnounce.setPeerStorage(_peerStorage);
  btAnnounce.setBtRuntime(_btRuntime);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.generateKey();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  _pieceStorage->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=AAAAAAAA&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_malformed()
{
  try {
    std::string res = "i123e";
    DefaultBtAnnounce(_dctx, _option).processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_failureReason()
{
  try {
    std::string res = "d14:failure reason11:hello worlde";
    DefaultBtAnnounce(_dctx, _option).processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
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
  
  DefaultBtAnnounce an(_dctx, _option);
  an.processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), an.getTrackerID());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, an.getInterval());
  CPPUNIT_ASSERT_EQUAL((time_t)1800, an.getMinInterval());
  CPPUNIT_ASSERT_EQUAL((unsigned int)100, an.getComplete());
  CPPUNIT_ASSERT_EQUAL((unsigned int)200, an.getIncomplete());
}

} // namespace aria2
