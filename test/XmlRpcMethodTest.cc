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
  CPPUNIT_TEST(testChangeOption_withoutGid);
  CPPUNIT_TEST(testChangeGlobalOption);
  CPPUNIT_TEST(testChangeGlobalOption_withBadOption);
  CPPUNIT_TEST(testTellStatus_withoutGid);
  CPPUNIT_TEST(testTellWaiting);
  CPPUNIT_TEST(testTellWaiting_fail);
  CPPUNIT_TEST(testNoSuchMethod);
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
    _e.reset(new DownloadEngine(SharedHandle<EventPoll>(new SelectEventPoll())));
    _e->option = _option.get();
    _e->_requestGroupMan.reset
      (new RequestGroupMan(std::deque<SharedHandle<RequestGroup> >(),
			   1, _option.get()));
  }

  void tearDown() {}

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
  void testChangeOption_withoutGid();
  void testChangeGlobalOption();
  void testChangeGlobalOption_withBadOption();
  void testTellStatus_withoutGid();
  void testTellWaiting();
  void testTellWaiting_fail();
  void testNoSuchMethod();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcMethodTest);

void XmlRpcMethodTest::testAddUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost/");
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    const std::deque<SharedHandle<RequestGroup> > rgs =
      _e->_requestGroupMan->getReservedGroups();
    CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
			 rgs.front()->getRemainingUris().front());
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink"),
			 _e->_requestGroupMan->findReservedGroup(2)->
			 getDownloadContext()->getDir());
  }
}

void XmlRpcMethodTest::testAddUri_withoutUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddUri_notUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("not uri");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddUri_withBadOption()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
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
  XmlRpcRequest req1("aria2.addUri", BDE::list());
  req1._params << BDE::list();
  req1._params[0] << BDE("http://uri1");
  XmlRpcResponse res1 = m.execute(req1, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res1._code);
  
  XmlRpcRequest req2("aria2.addUri", BDE::list());
  req2._params << BDE::list();
  req2._params[0] << BDE("http://uri2");
  req2._params << BDE::dict();
  req2._params << BDE((int64_t)0);
  m.execute(req2, _e.get());

  std::string uri =
    _e->_requestGroupMan->getReservedGroups()[0]->getRemainingUris()[0];

  CPPUNIT_ASSERT_EQUAL(std::string("http://uri2"), uri);
}

void XmlRpcMethodTest::testAddUri_withBadPosition()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
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
  XmlRpcRequest req("aria2.addTorrent", BDE::list());
  req._params << BDE(readFile("single.torrent"));
  BDE uris = BDE::list();
  uris << BDE("http://localhost/aria2-0.8.2.tar.bz2");
  req._params << uris;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), res._param.s());

    SharedHandle<RequestGroup> group = _e->_requestGroupMan->findReservedGroup(1);
    CPPUNIT_ASSERT(!group.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-0.8.2.tar.bz2"),
			 group->getFilePath());
    CPPUNIT_ASSERT_EQUAL((size_t)1, group->getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/aria2-0.8.2.tar.bz2"),
			 group->getRemainingUris()[0]);
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink/aria2-0.8.2.tar.bz2"),
			 _e->_requestGroupMan->findReservedGroup(2)->getFilePath());
  }
}

void XmlRpcMethodTest::testAddTorrent_withoutTorrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req("aria2.addTorrent", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddTorrent_notBase64Torrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req("aria2.addTorrent", BDE::list());
  req._params << BDE("not torrent");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddTorrent_withPosition()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req1("aria2.addTorrent", BDE::list());
  req1._params << BDE(readFile("test.torrent"));
  req1._params << BDE::list();
  req1._params << BDE::dict();
  XmlRpcResponse res1 = m.execute(req1, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res1._code);

  XmlRpcRequest req2("aria2.addTorrent", BDE::list());
  req2._params << BDE(readFile("single.torrent"));
  req2._params << BDE::list();
  req2._params << BDE::dict();
  req2._params << BDE((int64_t)0);
  m.execute(req2, _e.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1,
		       _e->_requestGroupMan->getReservedGroups()[0]->
		       getDownloadContext()->getFileEntries().size());
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void XmlRpcMethodTest::testAddMetalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req("aria2.addMetalink", BDE::list());
  req._params << BDE(readFile("2files.metalink"));
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL((size_t)2, res._param.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1"), res._param[0].s());
    CPPUNIT_ASSERT_EQUAL(std::string("2"), res._param[1].s());

    SharedHandle<RequestGroup> tar = _e->_requestGroupMan->findReservedGroup(1);
    CPPUNIT_ASSERT(!tar.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-5.0.0.tar.bz2"),
			 tar->getFilePath());
    SharedHandle<RequestGroup> deb = _e->_requestGroupMan->findReservedGroup(2);
    CPPUNIT_ASSERT(!deb.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-5.0.0.deb"),
			 deb->getFilePath());
  }
  // with options
  BDE opt = BDE::dict();
  opt[PREF_DIR] = BDE("/sink");
  req._params << opt;
  {
    XmlRpcResponse res = m.execute(req, _e.get());
    CPPUNIT_ASSERT_EQUAL(0, res._code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink/aria2-5.0.0.tar.bz2"),
			 _e->_requestGroupMan->findReservedGroup(3)->getFilePath());
  }
}

void XmlRpcMethodTest::testAddMetalink_withoutMetalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req("aria2.addMetalink", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddMetalink_notBase64Metalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req("aria2.addMetalink", BDE::list());
  req._params << BDE("not metalink");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testAddMetalink_withPosition()
{
  AddUriXmlRpcMethod m1;
  XmlRpcRequest req1("aria2.addUri", BDE::list());
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
		       _e->_requestGroupMan->getReservedGroups()[0]->
		       getFilePath());
}

#endif // ENABLE_METALINK

void XmlRpcMethodTest::testChangeOption()
{
  SharedHandle<RequestGroup> group
    (new RequestGroup(_option, std::deque<std::string>()));
  _e->_requestGroupMan->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req("aria2.changeOption", BDE::list());
  req._params << BDE("1");
  BDE opt = BDE::dict();
  opt[PREF_MAX_DOWNLOAD_LIMIT] = BDE("100K");
#ifdef ENABLE_BITTORRENT
  opt[PREF_MAX_UPLOAD_LIMIT] = BDE("50K");
#endif // ENABLE_BITTORRENT
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
		       group->getMaxDownloadSpeedLimit());
#ifdef ENABLE_BITTORRENT
   CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024, group->getMaxUploadSpeedLimit());
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeOption_withBadOption()
{
  SharedHandle<RequestGroup> group
    (new RequestGroup(_option, std::deque<std::string>()));
  _e->_requestGroupMan->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req("aria2.changeOption", BDE::list());
  req._params << BDE("1");
  BDE opt = BDE::dict();
  opt[PREF_MAX_DOWNLOAD_LIMIT] = BDE("badvalue");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeOption_withoutGid()
{
  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req("aria2.changeOption", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

void XmlRpcMethodTest::testChangeGlobalOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req("aria2.changeGlobalOption", BDE::list());
  BDE opt = BDE::dict();
  opt[PREF_MAX_OVERALL_DOWNLOAD_LIMIT] = BDE("100K");
#ifdef ENABLE_BITTORRENT
  opt[PREF_MAX_OVERALL_UPLOAD_LIMIT] = BDE("50K");
#endif // ENABLE_BITTORRENT
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
		       _e->_requestGroupMan->getMaxOverallDownloadSpeedLimit());
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024,
		       _e->_requestGroupMan->getMaxOverallUploadSpeedLimit());
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeGlobalOption_withBadOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req("aria2.changeGlobalOption", BDE::list());
  BDE opt = BDE::dict();
  opt[PREF_MAX_OVERALL_DOWNLOAD_LIMIT] = BDE("badvalue");
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
  XmlRpcRequest req("aria2.tellStatus", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

static void addUri(const std::string& uri,
		   const SharedHandle<DownloadEngine>& e)
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE(uri);
  CPPUNIT_ASSERT_EQUAL(0, m.execute(req, e.get())._code);
}

void XmlRpcMethodTest::testTellWaiting()
{
  addUri("http://1/", _e);
  addUri("http://2/", _e);
  addUri("http://3/", _e);
  addUri("http://4/", _e);

  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req("aria2.tellWaiting", BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)2);
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res._param.size());
  CPPUNIT_ASSERT_EQUAL(std::string("2"), res._param[0]["gid"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("3"), res._param[1]["gid"].s());
  // waiting.size() == offset+num 
  req = XmlRpcRequest("aria2.tellWaiting", BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)3);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)3, res._param.size());
  // waiting.size() < offset+num 
  req = XmlRpcRequest("aria2.tellWaiting", BDE::list());
  req._params << BDE((int64_t)1);
  req._params << BDE((int64_t)4);
  res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((size_t)3, res._param.size());
}

void XmlRpcMethodTest::testTellWaiting_fail()
{
  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req("aria2.tellWaiting", BDE::list());
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(1, res._code);
}

} // namespace xmlrpc

} // namespace aria2
