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

namespace aria2 {

namespace xmlrpc {

class XmlRpcMethodTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcMethodTest);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testChangeOption);
  CPPUNIT_TEST(testChangeGlobalOption);
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
    _e.reset(new DownloadEngine(SharedHandle<EventPoll>(new SelectEventPoll())));
    _e->option = _option.get();
    _e->_requestGroupMan.reset
      (new RequestGroupMan(std::deque<SharedHandle<RequestGroup> >(),
			   1, _option.get()));
  }

  void tearDown() {}

  void testAddUri();
  void testChangeOption();
  void testChangeGlobalOption();
  void testNoSuchMethod();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcMethodTest);

void XmlRpcMethodTest::testAddUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req("aria2.addUri", BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost/");
  XmlRpcResponse res = m.execute(req, _e.get());
  CPPUNIT_ASSERT_EQUAL(0, res._code);
  const std::deque<SharedHandle<RequestGroup> > rgs =
    _e->_requestGroupMan->getReservedGroups();
  CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
		       rgs.front()->getRemainingUris().front());
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

} // namespace xmlrpc

} // namespace aria2
