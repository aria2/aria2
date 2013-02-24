#include "DefaultBtAnnounce.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "Option.h"
#include "util.h"
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
#include "UDPTrackerRequest.h"
#include "SocketCore.h"

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
  CPPUNIT_TEST(testProcessUDPTrackerResponse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<MockPieceStorage> pieceStorage_;
  SharedHandle<MockPeerStorage> peerStorage_;
  SharedHandle<BtRuntime> btRuntime_;
  Option* option_;
public:
  void setUp() {
    option_ = new Option();

    int64_t totalLength = 4*1024*1024;
    int32_t pieceLength = 256*1024;

    static const unsigned char infoHash[] = {  0x01, 0x23, 0x45, 0x67,
                                               0x89, 0xab, 0xcd, 0xef,
                                               0x01, 0x23, 0x45, 0x67,
                                               0x89, 0xab, 0xcd, 0xef,
                                               0x01, 0x23, 0x45, 0x67 };

    std::string peerId = "-aria2-ultrafastdltl";

    dctx_.reset(new DownloadContext(pieceLength, totalLength));
    SharedHandle<TorrentAttribute> torrentAttrs(new TorrentAttribute());
    torrentAttrs->infoHash = std::string(vbegin(infoHash), vend(infoHash));
    dctx_->setAttribute(CTX_ATTR_BT, torrentAttrs);
    dctx_->getNetStat().updateDownloadLength(pieceLength*5);
    dctx_->getNetStat().updateUploadLength(pieceLength*6);
    bittorrent::setStaticPeerId(peerId);

    pieceStorage_.reset(new MockPieceStorage());
    pieceStorage_->setTotalLength(totalLength);
    pieceStorage_->setCompletedLength(pieceLength*10);

    peerStorage_.reset(new MockPeerStorage());
    btRuntime_.reset(new BtRuntime());
  }

  void tearDown()
  {
    delete option_;
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
  void testProcessUDPTrackerResponse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtAnnounceTest);

namespace {
template<typename InputIterator>
SharedHandle<List> createAnnounceTier
(InputIterator first, InputIterator last)
{
  SharedHandle<List> announceTier = List::g();
  for(; first != last; ++first) {
    announceTier->append(String::g(*first));
  }
  return announceTier;
}
} // namespace

namespace {
SharedHandle<List> createAnnounceTier(const std::string& uri)
{
  SharedHandle<List> announceTier = List::g();
  announceTier->append(String::g(uri));
  return announceTier;
}
} // namespace

namespace {
void setAnnounceList(const SharedHandle<DownloadContext>& dctx,
                     const SharedHandle<List>& announceList)
{
  std::vector<std::vector<std::string> > dest;
  for(List::ValueType::const_iterator tierIter = announceList->begin(),
        eoi = announceList->end(); tierIter != eoi; ++tierIter) {
    std::vector<std::string> ntier;
    const List* tier = downcast<List>(*tierIter);
    for(List::ValueType::const_iterator uriIter = tier->begin(),
          eoi2 = tier->end(); uriIter != eoi2; ++uriIter) {
      const String* uri = downcast<String>(*uriIter);
      ntier.push_back(uri->s());
    }
    dest.push_back(ntier);
  }
  bittorrent::getTorrentAttrs(dctx)->announceList.swap(dest);
}
} // namespace

void DefaultBtAnnounceTest::testNoMoreAnnounce()
{
  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier("http://localhost/announce"));
  announceList->append(createAnnounceTier("http://backup/announce"));

  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  pieceStorage_->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  btRuntime_->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT(btAnnounce.noMoreAnnounce());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl()
{

  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier("http://localhost/announce"));
  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);
  SharedHandle<UDPTrackerRequest> req;

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());
  req = btAnnounce.createUDPTrackerRequest("localhost", 80, 6989);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), req->remoteAddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)80, req->remotePort);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE, req->action);
  CPPUNIT_ASSERT_EQUAL(bittorrent::getInfoHashString(dctx_),
                       util::toHex(req->infohash));
  CPPUNIT_ASSERT_EQUAL(std::string("-aria2-ultrafastdltl"), req->peerId);
  CPPUNIT_ASSERT_EQUAL((int64_t)1310720, req->downloaded);
  CPPUNIT_ASSERT_EQUAL((int64_t)1572864, req->left);
  CPPUNIT_ASSERT_EQUAL((int64_t)1572864, req->uploaded);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_EVT_STARTED, req->event);
  CPPUNIT_ASSERT_EQUAL((uint32_t)0, req->ip);
  CPPUNIT_ASSERT_EQUAL((int32_t)50, req->numWant);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6989, req->port);
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, req->extensions);

  btAnnounce.announceSuccess();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&supportcrypto=1"), btAnnounce.getAnnounceUrl());
  req = btAnnounce.createUDPTrackerRequest("localhost", 80, 6989);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE, req->action);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_EVT_NONE, req->event);

  btAnnounce.announceSuccess();

  pieceStorage_->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());
  req = btAnnounce.createUDPTrackerRequest("localhost", 80, 6989);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE, req->action);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_EVT_COMPLETED, req->event);

  btAnnounce.announceSuccess();

  btRuntime_->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());
  req = btAnnounce.createUDPTrackerRequest("localhost", 80, 6989);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_ACT_ANNOUNCE, req->action);
  CPPUNIT_ASSERT_EQUAL((int)UDPT_EVT_STOPPED, req->event);
}

void DefaultBtAnnounceTest::testGetAnnounceUrl_withQuery()
{
  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier("http://localhost/announce?k=v"));
  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL
    (std::string("http://localhost/announce?k=v&"
                 "info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&"
                 "peer_id=%2Daria2%2Dultrafastdltl&"
                 "uploaded=1572864&downloaded=1310720&left=1572864&compact=1&"
                 "key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&"
                 "supportcrypto=1"),
     btAnnounce.getAnnounceUrl());
}

void DefaultBtAnnounceTest::testGetAnnounceUrl_externalIP()
{
  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier("http://localhost/announce"));
  setAnnounceList(dctx_, announceList);

  option_->put(PREF_BT_EXTERNAL_IP, "192.168.1.1");
  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL
    (std::string("http://localhost/announce?"
                 "info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&"
                 "peer_id=%2Daria2%2Dultrafastdltl&"
                 "uploaded=1572864&downloaded=1310720&left=1572864&compact=1&"
                 "key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&"
                 "supportcrypto=1&ip=192.168.1.1"),
     btAnnounce.getAnnounceUrl());

  SharedHandle<UDPTrackerRequest> req;
  req = btAnnounce.createUDPTrackerRequest("localhost", 80, 6989);
  char host[NI_MAXHOST];
  int rv = inetNtop(AF_INET, &req->ip, host, sizeof(host));
  CPPUNIT_ASSERT_EQUAL(0, rv);
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.1"), std::string(host));
}

void DefaultBtAnnounceTest::testIsAllAnnounceFailed()
{
  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier("http://localhost/announce"));
  announceList->append(createAnnounceTier("http://backup/announce"));
  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://backup/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

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

  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier(vbegin(urls), vend(urls)));
  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  btRuntime_->setHalt(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=0&no_peer_id=1&port=6989&event=stopped&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testURLOrderInCompletedEvent()
{
  const char* urls[] = { "http://localhost1/announce",
                         "http://localhost2/announce" };

  SharedHandle<List> announceList = List::g();
  announceList->append(createAnnounceTier(vbegin(urls), vend(urls)));
  setAnnounceList(dctx_, announceList);

  DefaultBtAnnounce btAnnounce(dctx_, option_);
  btAnnounce.setPieceStorage(pieceStorage_);
  btAnnounce.setPeerStorage(peerStorage_);
  btAnnounce.setBtRuntime(btRuntime_);
  btAnnounce.setRandomizer(SharedHandle<Randomizer>(new FixedNumberRandomizer()));
  btAnnounce.setTcpPort(6989);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=started&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();

  pieceStorage_->setAllDownloadFinished(true);

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost1/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceFailure();

  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost2/announce?info_hash=%01%23Eg%89%AB%CD%EF%01%23Eg%89%AB%CD%EF%01%23Eg&peer_id=%2Daria2%2Dultrafastdltl&uploaded=1572864&downloaded=1310720&left=1572864&compact=1&key=fastdltl&numwant=50&no_peer_id=1&port=6989&event=completed&supportcrypto=1"), btAnnounce.getAnnounceUrl());

  btAnnounce.announceSuccess();
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_malformed()
{
  try {
    std::string res = "i123e";
    DefaultBtAnnounce(dctx_, option_).processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultBtAnnounceTest::testProcessAnnounceResponse_failureReason()
{
  try {
    std::string res = "d14:failure reason11:hello worlde";
    DefaultBtAnnounce(dctx_, option_).processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
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
    "10:incompletei200e";
  res += "5:peers6:";
  res += fromHex("c0a800011ae1");
  res += "6:peers618:";
  res += fromHex("100210354527354678541237324732171ae1");
  res += "e";

  DefaultBtAnnounce an(dctx_, option_);
  an.setPeerStorage(peerStorage_);
  an.setBtRuntime(btRuntime_);
  an.processAnnounceResponse(reinterpret_cast<const unsigned char*>(res.c_str()), res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), an.getTrackerID());
  CPPUNIT_ASSERT_EQUAL((time_t)3000, an.getInterval());
  CPPUNIT_ASSERT_EQUAL((time_t)1800, an.getMinInterval());
  CPPUNIT_ASSERT_EQUAL(100, an.getComplete());
  CPPUNIT_ASSERT_EQUAL(200, an.getIncomplete());
  CPPUNIT_ASSERT_EQUAL((size_t)2, peerStorage_->getUnusedPeers().size());
  SharedHandle<Peer> peer = peerStorage_->getUnusedPeers()[0];
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), peer->getIPAddress());
  peer = peerStorage_->getUnusedPeers()[1];
  CPPUNIT_ASSERT_EQUAL(std::string("1002:1035:4527:3546:7854:1237:3247:3217"),
                       peer->getIPAddress());
}

void DefaultBtAnnounceTest::testProcessUDPTrackerResponse()
{
  SharedHandle<UDPTrackerRequest> req(new UDPTrackerRequest());
  req->action = UDPT_ACT_ANNOUNCE;
  SharedHandle<UDPTrackerReply> reply(new UDPTrackerReply());
  reply->interval = 1800;
  reply->leechers = 200;
  reply->seeders = 100;
  for(int i = 0; i < 2; ++i) {
    reply->peers.push_back(std::make_pair("192.168.0."+util::uitos(i+1),
                                          6890+i));
  }
  req->reply = reply;
  DefaultBtAnnounce an(dctx_, option_);
  an.setPeerStorage(peerStorage_);
  an.setBtRuntime(btRuntime_);
  an.processUDPTrackerResponse(req);
  CPPUNIT_ASSERT_EQUAL((time_t)1800, an.getInterval());
  CPPUNIT_ASSERT_EQUAL((time_t)1800, an.getMinInterval());
  CPPUNIT_ASSERT_EQUAL(100, an.getComplete());
  CPPUNIT_ASSERT_EQUAL(200, an.getIncomplete());
  CPPUNIT_ASSERT_EQUAL((size_t)2, peerStorage_->getUnusedPeers().size());
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Peer> peer;
    peer = peerStorage_->getUnusedPeers()[i];
    CPPUNIT_ASSERT_EQUAL("192.168.0."+util::uitos(i+1), peer->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)(6890+i), peer->getPort());
  }
}

} // namespace aria2
