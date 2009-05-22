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
  CPPUNIT_TEST(testAddTorrent);
  CPPUNIT_TEST(testAddTorrent_withoutTorrent);
  CPPUNIT_TEST(testAddTorrent_notBase64Torrent);
  CPPUNIT_TEST(testAddMetalink);
  CPPUNIT_TEST(testAddMetalink_withoutMetalink);
  CPPUNIT_TEST(testAddMetalink_notBase64Metalink);
  CPPUNIT_TEST(testChangeOption);
  CPPUNIT_TEST(testChangeOption_withBadOption);
  CPPUNIT_TEST(testChangeOption_withoutGid);
  CPPUNIT_TEST(testChangeGlobalOption);
  CPPUNIT_TEST(testChangeGlobalOption_withBadOption);
  CPPUNIT_TEST(testTellStatus_withoutGid);
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
  void testAddTorrent();
  void testAddTorrent_withoutTorrent();
  void testAddTorrent_notBase64Torrent();
  void testAddMetalink();
  void testAddMetalink_withoutMetalink();
  void testAddMetalink_notBase64Metalink();
  void testChangeOption();
  void testChangeOption_withBadOption();
  void testChangeOption_withoutGid();
  void testChangeGlobalOption();
  void testChangeGlobalOption_withBadOption();
  void testTellStatus_withoutGid();
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
  opt[PREF_MAX_UPLOAD_LIMIT] = BDE("50K");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
		       group->getMaxDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024, group->getMaxUploadSpeedLimit());
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
  opt[PREF_MAX_OVERALL_UPLOAD_LIMIT] = BDE("50K");
  req._params << opt;
  XmlRpcResponse res = m.execute(req, _e.get());

  CPPUNIT_ASSERT_EQUAL(0, res._code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
		       _e->_requestGroupMan->getMaxOverallDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024,
		       _e->_requestGroupMan->getMaxOverallUploadSpeedLimit());
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

} // namespace xmlrpc

} // namespace aria2
