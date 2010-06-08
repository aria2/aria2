#include "XmlRpcMethod.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadEngine.h"
#include "SelectEventPoll.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "ServerStatMan.h"
#include "RequestGroup.h"
#include "XmlRpcMethodImpl.h"
#include "BDE.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "XmlRpcRequest.h"
#include "XmlRpcResponse.h"
#include "prefs.h"
#include "TestUtil.h"
#include "DownloadContext.h"
#include "FeatureConfig.h"
#include "util.h"
#include "array_fun.h"
#include "download_helper.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#ifdef ENABLE_BITTORRENT
# include "BtRegistry.h"
# include "BtRuntime.h"
# include "PieceStorage.h"
# include "PeerStorage.h"
# include "BtProgressInfoFile.h"
# include "BtAnnounce.h"
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace xmlrpc {

class XmlRpcMethodTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcMethodTest);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testAddUri_withoutUri);
  CPPUNIT_TEST(testAddUri_notUri);
  CPPUNIT_TEST(testAddUri_withBadOption);
  CPPUNIT_TEST(testAddUri_withPosition);
  CPPUNIT_TEST(testAddUri_withBadPosition);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testAddTorrent);
  CPPUNIT_TEST(testAddTorrent_withoutTorrent);
  CPPUNIT_TEST(testAddTorrent_notBase64Torrent);
  CPPUNIT_TEST(testAddTorrent_withPosition);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  CPPUNIT_TEST(testAddMetalink);
  CPPUNIT_TEST(testAddMetalink_withoutMetalink);
  CPPUNIT_TEST(testAddMetalink_notBase64Metalink);
  CPPUNIT_TEST(testAddMetalink_withPosition);
#endif // ENABLE_METALINK
  CPPUNIT_TEST(testChangeOption);
  CPPUNIT_TEST(testChangeOption_withBadOption);
  CPPUNIT_TEST(testChangeOption_withNotAllowedOption);
  CPPUNIT_TEST(testChangeOption_withoutGid);
  CPPUNIT_TEST(testChangeGlobalOption);
  CPPUNIT_TEST(testChangeGlobalOption_withBadOption);
  CPPUNIT_TEST(testChangeGlobalOption_withNotAllowedOption);
  CPPUNIT_TEST(testTellStatus_withoutGid);
  CPPUNIT_TEST(testTellWaiting);
  CPPUNIT_TEST(testTellWaiting_fail);
  CPPUNIT_TEST(testGetVersion);
  CPPUNIT_TEST(testNoSuchMethod);
  CPPUNIT_TEST(testGatherStoppedDownload);
  CPPUNIT_TEST(testGatherProgressCommon);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testGatherBitTorrentMetadata);
#endif // ENABLE_BITTORRENT
  CPPUNIT_TEST(testChangePosition);
  CPPUNIT_TEST(testChangePosition_fail);
  CPPUNIT_TEST(testGetSessionInfo);
  CPPUNIT_TEST(testChangeUri);
  CPPUNIT_TEST(testChangeUri_fail);
  CPPUNIT_TEST(testPause);
  CPPUNIT_TEST(testSystemMulticall);
  CPPUNIT_TEST(testSystemMulticall_fail);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadEngine> _e;
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    RequestGroup::resetGIDCounter();
    _option.reset(new Option());
    _option->put(PREF_DIR, "/tmp");
    _option->put(PREF_SEGMENT_SIZE, "1048576");
    _e.reset
      (new DownloadEngine(SharedHandle<EventPoll>(new SelectEventPoll())));
    _e->setOption(_option.get());
    _e->setRequestGroupMan
      (SharedHandle<RequestGroupMan>
       (new RequestGroupMan(std::vector<SharedHandle<RequestGroup> >(),
                            1, _option.get())));
  }

  void testAddUri();
  void testAddUri_withoutUri();
  void testAddUri_notUri();
  void testAddUri_withBadOption();
  void testAddUri_withPosition();
  void testAddUri_withBadPosition();
#ifdef ENABLE_BITTORRENT
  void testAddTorrent();
  void testAddTorrent_withoutTorrent();
  void testAddTorrent_notBase64Torrent();
  void testAddTorrent_withPosition();
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  void testAddMetalink();
  void testAddMetalink_withoutMetalink();
  void testAddMetalink_notBase64Metalink();
  void testAddMetalink_withPosition();
#endif // ENABLE_METALINK
  void testChangeOption();
  void testChangeOption_withBadOption();
  void testChangeOption_withNotAllowedOption();
  void testChangeOption_withoutGid();
  void testChangeGlobalOption();
  void testChangeGlobalOption_withBadOption();
  void testChangeGlobalOption_withNotAllowedOption();
  void testTellStatus_withoutGid();
  void testTellWaiting();
  void testTellWaiting_fail();
  void testGetVersion();
  void testNoSuchMethod();
  void testGatherStoppedDownload();
  void testGatherProgressCommon();
#ifdef ENABLE_BITTORRENT
  void testGatherBitTorrentMetadata();
#endif // ENABLE_BITTORRENT
  void testChangePosition();
  void testChangePosition_fail();
  void testGetSessionInfo();
  void testChangeUri();
  void testChangeUri_fail();
  void testPause();
  void testSystemMulticall();
  void testSystemMulticall_fail();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcMethodTest);

void XmlRpcMethodTest::testAddUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost/");
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    const std::deque<SharedHandle<RequestGroup> > rgs =
      _e->getRequestGroupMan()->getReservedGroups();
    CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
                         rgs.front()->getDownloadContext()->
                         getFirstFileEntry()->getRemainingUris().front());
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink"),
                         _e->getRequestGroupMan()->findReservedGroup(2)->
                         getDownloadContext()->getDir());
  }
}

void XmlRpcMethodTest::testAddUri_withoutUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddUri_notUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("not uri");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddUri_withBadOption()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost");
  BDE opt = BDE::dict();
  opt[PREF_FILE_ALLOCATION] = BDE("badvalue");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddUri_withPosition()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req1(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req1._params << BDE::list();
  req1._params[0] << BDE("http://uri1");
  XmlRpcResponse res1 = m.execute(req1, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res1._code);
  
  XmlRpcRequest req2(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req2._params << BDE::list();
  req2._params[0] << BDE("http://uri2");
  req2._params << BDE::dict();
  req2._params << BDE((int64_t)0);
  m.execute(req2, _e.get());

  std::string uri =
    _e->getRequestGroupMan()->getReservedGroups()[0]->
    getDownloadContext()->getFirstFileEntry()->getRemainingUris()[0];

  CPPUNIT_ASSERT_EQUAL(std::string("http://uri2"), uri);
}

void XmlRpcMethodTest::testAddUri_withBadPosition()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost/");
  req._params << BDE::dict();
  req._params << BDE((int64_t)-1);
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

#ifdef ENABLE_BITTORRENT
void XmlRpcMethodTest::testAddTorrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE(readFile("single.torrent"));
  BDE uris = BDE::list();
  uris << BDE("http://localhost/aria2-0.8.2.tar.bz2");
  req._params << uris;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), res._param.s());

    SharedHandle<RequestGroup> group =
      _e->getRequestGroupMan()->findReservedGroup(1);
    CPPUNIT_ASSERT(!group.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-0.8.2.tar.bz2"),
                         group->getFirstFilePath());
    CPPUNIT_ASSERT_EQUAL((size_t)1,
                         group->getDownloadContext()->getFirstFileEntry()->
                         getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/aria2-0.8.2.tar.bz2"),
                         group->getDownloadContext()->getFirstFileEntry()->
                         getRemainingUris()[0]);
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink/aria2-0.8.2.tar.bz2"),
                         _e->getRequestGroupMan()->findReservedGroup(2)->
                         getFirstFilePath());
  }
}

void XmlRpcMethodTest::testAddTorrent_withoutTorrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddTorrent_notBase64Torrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE("not torrent");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddTorrent_withPosition()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req1(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  req1._params << BDE(readFile("test.torrent"));
  req1._params << BDE::list();
  req1._params << BDE::dict();
  XmlRpcResponse res1 = m.execute(req1, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res1._code);

  XmlRpcRequest req2(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  req2._params << BDE(readFile("single.torrent"));
  req2._params << BDE::list();
  req2._params << BDE::dict();
  req2._params << BDE((int64_t)0);
  m.execute(req2, _e.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       _e->getRequestGroupMan()->getReservedGroups()[0]->
                       getDownloadContext()->getFileEntries().size());
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void XmlRpcMethodTest::testAddMetalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE(readFile("2files.metalink"));
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL((size_t)2, res._param.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1"), res._param[0].s());
    CPPUNIT_ASSERT_EQUAL(std::string("2"), res._param[1].s());

    SharedHandle<RequestGroup> tar =
      _e->getRequestGroupMan()->findReservedGroup(1);
    CPPUNIT_ASSERT(!tar.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-5.0.0.tar.bz2"),
                         tar->getFirstFilePath());
    SharedHandle<RequestGroup> deb =
      _e->getRequestGroupMan()->findReservedGroup(2);
    CPPUNIT_ASSERT(!deb.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-5.0.0.deb"),
                         deb->getFirstFilePath());
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink/aria2-5.0.0.tar.bz2"),
                         _e->getRequestGroupMan()->findReservedGroup(3)->
                         getFirstFilePath());
  }
}

void XmlRpcMethodTest::testAddMetalink_withoutMetalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddMetalink_notBase64Metalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE("not metalink");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddMetalink_withPosition()
{
  AddUriXmlRpcMethod m1;
  XmlRpcRequest req1(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req1._params << BDE::list();
  req1._params[0] << BDE("http://uri");
  XmlRpcResponse res1 = m1.execute(req1, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res1._code);

  AddMetalinkXmlRpcMethod m2;
  XmlRpcRequest req2("ari2.addMetalink", BDE::list());
  req2._params << BDE(readFile("2files.metalink"));
  req2._params << BDE::dict();
  req2._params << BDE((int64_t)0);
  XmlRpcResponse res2 = m2.execute(req2, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res2._code);

  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-5.0.0.tar.bz2"),
                       _e->getRequestGroupMan()->getReservedGroups()[0]->
                       getFirstFilePath());
}

#endif // ENABLE_METALINK

void XmlRpcMethodTest::testChangeOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  _e->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE("1");
  BDE opt = BDE::dict();
  opt[PREF_MAX_DOWNLOAD_LIMIT] = BDE("100K");
#ifdef ENABLE_BITTORRENT
  opt[PREF_BT_MAX_PEERS] = BDE("100");
  opt[PREF_BT_REQUEST_PEER_SPEED_LIMIT] = BDE("300K");
  opt[PREF_MAX_UPLOAD_LIMIT] = BDE("50K");

  BtObject btObject;
  btObject._btRuntime = SharedHandle<BtRuntime>(new BtRuntime());
  _e->getBtRegistry()->put(group->getGID(), btObject);
#endif // ENABLE_BITTORRENT
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  SharedHandle<Option> option = group->getOption();

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
                       group->getMaxDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       option->get(PREF_MAX_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(std::string("307200"),
                       option->get(PREF_BT_REQUEST_PEER_SPEED_LIMIT));

  CPPUNIT_ASSERT_EQUAL(std::string("100"), option->get(PREF_BT_MAX_PEERS));
  CPPUNIT_ASSERT_EQUAL((unsigned int)100, btObject._btRuntime->getMaxPeers());

  CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024,
                       group->getMaxUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       option->get(PREF_MAX_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeOption_withBadOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  _e->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE("1");
  BDE opt = BDE::dict();
  opt[PREF_MAX_DOWNLOAD_LIMIT] = BDE("badvalue");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeOption_withNotAllowedOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  _e->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE("1");
  BDE opt = BDE::dict();
  opt[PREF_MAX_OVERALL_DOWNLOAD_LIMIT] = BDE("100K");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeOption_withoutGid()
{
  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeGlobalOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), BDE::list());
  BDE opt = BDE::dict();
  opt[PREF_MAX_OVERALL_DOWNLOAD_LIMIT] = BDE("100K");
#ifdef ENABLE_BITTORRENT
  opt[PREF_MAX_OVERALL_UPLOAD_LIMIT] = BDE("50K");
#endif // ENABLE_BITTORRENT
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL
    ((unsigned int)100*1024,
     _e->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       _e->getOption()->get(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL
    ((unsigned int)50*1024,
     _e->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       _e->getOption()->get(PREF_MAX_OVERALL_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeGlobalOption_withBadOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), BDE::list());
  BDE opt = BDE::dict();
  opt[PREF_MAX_OVERALL_DOWNLOAD_LIMIT] = BDE("badvalue");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeGlobalOption_withNotAllowedOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), BDE::list());
  BDE opt = BDE::dict();
  opt[PREF_MAX_DOWNLOAD_LIMIT] = BDE("100K");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testNoSuchMethod()
{
  NoSuchMethodXmlRpcMethod m;
  XmlRpcRequest req("make.hamburger", BDE::none);
  XmlRpcResponse res = m.execute(req, 0);
  CPPUNIT_ASSERT_EQUAL(1, res._code);
  CPPUNIT_ASSERT_EQUAL(std::string("No such method: make.hamburger"),
                       res._param["faultString"].s());
  CPPUNIT_ASSERT_EQUAL
    (std::string("<?xml version=\"1.0\"?>"
                 "<methodResponse>"
                 "<fault>"
                 "<value>"
                 "<struct>"
                 "<member>"
                 "<name>faultCode</name><value><int>1</int></value>"
                 "</member>"
                 "<member>"
                 "<name>faultString</name>"
                 "<value>"
                 "<string>No such method: make.hamburger</string>"
                 "</value>"
                 "</member>"
                 "</struct>"
                 "</value>"
                 "</fault>"
                 "</methodResponse>"),
     res.toXml());
}

void XmlRpcMethodTest::testTellStatus_withoutGid()
{
  TellStatusXmlRpcMethod m;
  XmlRpcRequest req(TellStatusXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

static void addUri(const std::string& uri,
                   const SharedHandle<DownloadEngine>& e)
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE(uri);
  CPPUNIT_ASSERT_EQUAL(0, m.execute(req, e.get())._code);
}

#ifdef ENABLE_BITTORRENT

static void addTorrent
(const std::string& torrentFile, const SharedHandle<DownloadEngine>& e)
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE(readFile(torrentFile));
  XmlRpcResponse res = m.execute(req, e.get());
}

#endif // ENABLE_BITTORRENT

void XmlRpcMethodTest::testTellWaiting()
{
  addUri("http://1/", _e);
  addUri("http://2/", _e);
  addUri("http://3/", _e);
#ifdef ENABLE_BITTORRENT
  addTorrent("single.torrent", _e);
#else // !ENABLE_BITTORRENT
  addUri("http://4/", _e);
#endif // !ENABLE_BITTORRENT

  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req(TellWaitingXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)2);
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res._param.size());
  CPPUNIT_ASSERT_EQUAL(std::string("2"), res._param[0]["gid"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("3"), res._param[1]["gid"].s());
  // waiting.size() == offset+num 
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)3);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)3, res._param.size());
  // waiting.size() < offset+num 
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)4);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)3, res._param.size());
  // negative offset
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), BDE::list());
  req._params << BDE((int64_t)-1);
  req._params << BDE((int64_t)2);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res._param.size());
  CPPUNIT_ASSERT_EQUAL(std::string("4"), res._param[0]["gid"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("3"), res._param[1]["gid"].s());
  // negative offset and size < num
  req._params[1] = BDE((int64_t)100);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)4, res._param.size());
  // nagative offset and normalized offset < 0
  req._params[0] = BDE((int64_t)-5);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)0, res._param.size());
  // nagative offset and normalized offset == 0
  req._params[0] = BDE((int64_t)-4);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)1, res._param.size());
}

void XmlRpcMethodTest::testTellWaiting_fail()
{
  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req(TellWaitingXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testGetVersion()
{
  GetVersionXmlRpcMethod m;
  XmlRpcRequest req(GetVersionXmlRpcMethod::getMethodName(), BDE::none);
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL(std::string(PACKAGE_VERSION), res._param["version"].s());
  const BDE& featureList = res._param["enabledFeatures"];
  std::string features;
  for(BDE::List::const_iterator i = featureList.listBegin();
      i != featureList.listEnd(); ++i) {
    features += (*i).s();
    features += ", ";
  }
  
  CPPUNIT_ASSERT_EQUAL(FeatureConfig::getInstance()->featureSummary()+", ",
                       features);
}

void XmlRpcMethodTest::testGatherStoppedDownload()
{
  std::vector<SharedHandle<FileEntry> > fileEntries;
  std::vector<gid_t> followedBy;
  followedBy.push_back(3);
  followedBy.push_back(4);
  SharedHandle<DownloadResult> d(new DownloadResult());
  d->gid = 1;
  d->fileEntries = fileEntries;
  d->inMemoryDownload = false;
  d->sessionDownloadLength = UINT64_MAX;
  d->sessionTime = 1000;
  d->result = downloadresultcode::FINISHED;
  d->followedBy = followedBy;
  d->belongsTo = 2;
  BDE entry = BDE::dict();
  gatherStoppedDownload(entry, d);

  CPPUNIT_ASSERT_EQUAL(std::string("3"), entry["followedBy"][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("4"), entry["followedBy"][1].s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"), entry["belongsTo"].s());
}

void XmlRpcMethodTest::testGatherProgressCommon()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0,"aria2.tar.bz2"));
  std::string uris[] = { "http://localhost/aria2.tar.bz2" };
  dctx->getFirstFileEntry()->addUris(vbegin(uris), vend(uris));
  dctx->setDir(_option->get(PREF_DIR));
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  group->setDownloadContext(dctx);
  std::vector<SharedHandle<RequestGroup> > followedBy;
  for(int i = 0; i < 2; ++i) {
    followedBy.push_back(SharedHandle<RequestGroup>(new RequestGroup(_option)));
  }

  group->followedBy(followedBy.begin(), followedBy.end());
  group->belongsTo(2);

  BDE entry = BDE::dict();
  gatherProgressCommon(entry, group);
  
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[0]->getGID()),
                       entry["followedBy"][0].s());
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[1]->getGID()),
                       entry["followedBy"][1].s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"), entry["belongsTo"].s());
  CPPUNIT_ASSERT_EQUAL((size_t)1, entry["files"].size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       entry["files"][0]["path"].s());
  CPPUNIT_ASSERT_EQUAL(uris[0], entry["files"][0]["uris"][0]["uri"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), entry["dir"].s());
}

#ifdef ENABLE_BITTORRENT
void XmlRpcMethodTest::testGatherBitTorrentMetadata()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  bittorrent::load("test.torrent", dctx);
  BDE btDict = BDE::dict();
  gatherBitTorrentMetadata(btDict, dctx->getAttribute(bittorrent::BITTORRENT));
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"), btDict["comment"].s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1123456789, btDict["creationDate"].i());
  CPPUNIT_ASSERT_EQUAL(std::string("multi"), btDict["mode"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"), btDict["info"]["name"].s());
  const BDE& announceList = btDict["announceList"];
  CPPUNIT_ASSERT_EQUAL((size_t)3, announceList.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"), announceList[0][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"), announceList[1][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"), announceList[2][0].s());
  // Remove some keys
  BDE modBtAttrs = dctx->getAttribute(bittorrent::BITTORRENT);
  modBtAttrs.removeKey(bittorrent::COMMENT);
  modBtAttrs.removeKey(bittorrent::CREATION_DATE);
  modBtAttrs.removeKey(bittorrent::MODE);
  modBtAttrs.removeKey(bittorrent::METADATA);
  btDict = BDE::dict();
  gatherBitTorrentMetadata(btDict, modBtAttrs);
  CPPUNIT_ASSERT(!btDict.containsKey("comment"));
  CPPUNIT_ASSERT(!btDict.containsKey("creationDate"));
  CPPUNIT_ASSERT(!btDict.containsKey("mode"));
  CPPUNIT_ASSERT(!btDict.containsKey("info"));
  CPPUNIT_ASSERT(btDict.containsKey("announceList"));
}
#endif // ENABLE_BITTORRENT

void XmlRpcMethodTest::testChangePosition()
{
  _e->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(_option)));
  _e->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(_option)));

  ChangePositionXmlRpcMethod m;
  XmlRpcRequest req(ChangePositionXmlRpcMethod::getMethodName(), BDE::list());
  req._params << std::string("1");
  req._params << BDE((int64_t)1);
  req._params << std::string("POS_SET");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((int64_t)1, res._param.i());
  CPPUNIT_ASSERT_EQUAL
    ((gid_t)1, _e->getRequestGroupMan()->getReservedGroups()[1]->getGID());
}

void XmlRpcMethodTest::testChangePosition_fail()
{
  ChangePositionXmlRpcMethod m;
  XmlRpcRequest req(ChangePositionXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);

  req._params << std::string("1");
  req._params << BDE((int64_t)2);
  req._params << std::string("bad keyword");
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeUri()
{
  SharedHandle<FileEntry> files[3];
  for(int i = 0; i < 3; ++i) {
    files[i].reset(new FileEntry());
  }
  files[1]->addUri("http://example.org/aria2.tar.bz2");
  files[1]->addUri("http://example.org/mustremove1");
  files[1]->addUri("http://example.org/mustremove2");
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setFileEntries(&files[0], &files[3]);
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  group->setDownloadContext(dctx);
  _e->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriXmlRpcMethod m;
  XmlRpcRequest req(ChangeUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << std::string("1"); // GID
  req._params << 2; // index of FileEntry
  BDE removeuris = BDE::list();
  removeuris << std::string("http://example.org/mustremove1");
  removeuris << std::string("http://example.org/mustremove2");
  removeuris << std::string("http://example.org/notexist");
  req._params << removeuris;
  BDE adduris = BDE::list();
  adduris << std::string("http://example.org/added1");
  adduris << std::string("http://example.org/added2");
  adduris << std::string("baduri");
  adduris << std::string("http://example.org/added3");
  req._params << adduris;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((int64_t)2, res._param[0].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)3, res._param[1].i());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[0]->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[2]->getRemainingUris().size());
  std::deque<std::string> uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)4, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/aria2.tar.bz2"),uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added2"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added3"), uris[3]);

  // Change adduris
  adduris = BDE::list();
  adduris << std::string("http://example.org/added1-1");
  adduris << std::string("http://example.org/added1-2");
  req._params[3] = adduris;
  // Set position parameter
  req._params << 2;
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, res._param[0].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, res._param[1].i());
  uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)6, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[3]);

  // Change index of FileEntry
  req._params[1] = 1;
  // Set position far beyond the size of uris in FileEntry.
  req._params[4] = 1000;
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, res._param[0].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, res._param[1].i());
  uris = files[0]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[1]);
}

void XmlRpcMethodTest::testChangeUri_fail()
{
  SharedHandle<FileEntry> files[3];
  for(int i = 0; i < 3; ++i) {
    files[i].reset(new FileEntry());
  }
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setFileEntries(&files[0], &files[3]);
  SharedHandle<RequestGroup> group(new RequestGroup(_option));
  group->setDownloadContext(dctx);
  _e->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriXmlRpcMethod m;
  XmlRpcRequest req(ChangeUriXmlRpcMethod::getMethodName(), BDE::list());
  req._params << std::string("1"); // GID
  req._params << 1; // index of FileEntry
  BDE removeuris = BDE::list();
  req._params << removeuris;
  BDE adduris = BDE::list();
  req._params << adduris;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);

  req._params[0] = std::string("2");
  res = m.execute(req, _e.get());  
  // RPC request fails because GID#2 does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res._code);

  req._params[0] = std::string("1");
  req._params[1] = 4;
  res = m.execute(req, _e.get());  
  // RPC request fails because FileEntry#3 does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res._code);

  req._params[1] = std::string("0");
  res = m.execute(req, _e.get());  
  // RPC request fails because index of FileEntry is string.
  CPPUNIT_ASSERT_EQUAL(1, res._code);

  req._params[1] = 1;
  req._params[2] = std::string("http://url");
  res = m.execute(req, _e.get());  
  // RPC request fails because 3rd param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res._code);

  req._params[2] = BDE::list();
  req._params[3] = std::string("http://url");
  res = m.execute(req, _e.get());  
  // RPC request fails because 4th param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testGetSessionInfo()
{
  GetSessionInfoXmlRpcMethod m;
  XmlRpcRequest req(GetSessionInfoXmlRpcMethod::getMethodName(), BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL(util::toHex(_e->getSessionId()),
                       res._param["sessionId"].s());
}

void XmlRpcMethodTest::testPause()
{
  const std::string URIS[] = {
    "http://url1",
    "http://url2",
    "http://url3",
  };
  std::vector<std::string> uris(vbegin(URIS), vend(URIS));
  _option->put(PREF_FORCE_SEQUENTIAL, V_TRUE);
  std::vector<SharedHandle<RequestGroup> > groups;
  createRequestGroupForUri(groups, _option, uris);
  CPPUNIT_ASSERT_EQUAL((size_t)3, groups.size());  
  _e->getRequestGroupMan()->addReservedGroup(groups);
  {
    PauseXmlRpcMethod m;
    XmlRpcRequest req(PauseXmlRpcMethod::getMethodName(), BDE::list());
    req._params << std::string("1");
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
  }
  CPPUNIT_ASSERT(groups[0]->isPauseRequested());
  {
    UnpauseXmlRpcMethod m;
    XmlRpcRequest req(UnpauseXmlRpcMethod::getMethodName(), BDE::list());
    req._params << std::string("1");
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
  }
  CPPUNIT_ASSERT(!groups[0]->isPauseRequested());
  {
    PauseAllXmlRpcMethod m;
    XmlRpcRequest req(PauseAllXmlRpcMethod::getMethodName(), BDE::list());
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
  {
    UnpauseAllXmlRpcMethod m;
    XmlRpcRequest req(UnpauseAllXmlRpcMethod::getMethodName(), BDE::list());
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(!groups[i]->isPauseRequested());
  }
  {
    ForcePauseAllXmlRpcMethod m;
    XmlRpcRequest req(ForcePauseAllXmlRpcMethod::getMethodName(), BDE::list());
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
}

void XmlRpcMethodTest::testSystemMulticall()
{
  SystemMulticallXmlRpcMethod m;
  XmlRpcRequest req("system.multicall", BDE::list());
  BDE reqparams = BDE::list();
  req._params << reqparams;
  for(int i = 0; i < 2; ++i) {
    BDE dict = BDE::dict();
    dict["methodName"] = std::string(AddUriXmlRpcMethod::getMethodName());
    BDE params = BDE::list();
    params << BDE::list();
    params[0] << BDE("http://localhost/"+util::itos(i));
    dict["params"] = params;
    reqparams << dict;
  }
  {
    BDE dict = BDE::dict();
    dict["methodName"] = std::string("not exists");
    dict["params"] = BDE::list();
    reqparams << dict;
  }
  {
    reqparams << std::string("not struct");
  }
  {
    BDE dict = BDE::dict();
    dict["methodName"] = std::string("system.multicall");
    dict["params"] = BDE::list();
    reqparams << dict;
  }
  {
    // missing params
    BDE dict = BDE::dict();
    dict["methodName"] = std::string(GetVersionXmlRpcMethod::getMethodName());
    reqparams << dict;
  }
  {
    BDE dict = BDE::dict();
    dict["methodName"] = std::string(GetVersionXmlRpcMethod::getMethodName());
    dict["params"] = BDE::list();
    reqparams << dict;
  }
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)7, res._param.size());
  CPPUNIT_ASSERT_EQUAL(std::string("1"), res._param[0][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"), res._param[1][0].s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1, res._param[2]["faultCode"].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1, res._param[3]["faultCode"].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1, res._param[4]["faultCode"].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1, res._param[5]["faultCode"].i());
  CPPUNIT_ASSERT(res._param[6].isList());
}

void XmlRpcMethodTest::testSystemMulticall_fail()
{
  SystemMulticallXmlRpcMethod m;
  XmlRpcRequest req("system.multicall", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

} // namespace xmlrpc

} // namespace aria2
