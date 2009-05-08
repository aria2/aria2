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

namespace aria2 {

namespace xmlrpc {

class XmlRpcMethodTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcMethodTest);
  CPPUNIT_TEST(testAddURI);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadEngine> _e;
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
    _e.reset(new DownloadEngine(SharedHandle<EventPoll>(new SelectEventPoll())));
    _e->option = _option.get();
    _e->_requestGroupMan.reset
      (new RequestGroupMan(std::deque<SharedHandle<RequestGroup> >(),
			   1, _option.get()));
  }

  void tearDown() {}

  void testAddURI();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcMethodTest);

void XmlRpcMethodTest::testAddURI()
{
  AddURIXmlRpcMethod m;
  XmlRpcRequest req("aria2.addURI", BDE::list());
  req._params << BDE::list();
  req._params[0] << BDE("http://localhost/");
  std::string res = m.execute(req, _e.get());
  const std::deque<SharedHandle<RequestGroup> > rgs =
    _e->_requestGroupMan->getReservedGroups();
  CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
		       rgs.front()->getRemainingUris().front());
  CPPUNIT_ASSERT(res.find("OK") != std::string::npos);
}

} // namespace xmlrpc

} // namespace aria2
