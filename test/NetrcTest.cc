#include "Netrc.h"
#include "Exception.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class NetrcTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NetrcTest);
  CPPUNIT_TEST(testFindAuthenticator);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParse_fileNotFound);
  CPPUNIT_TEST(testParse_emptyfile);
  CPPUNIT_TEST(testParse_malformedNetrc);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testFindAuthenticator();
  void testParse();
  void testParse_fileNotFound();
  void testParse_emptyfile();
  void testParse_malformedNetrc();
};


CPPUNIT_TEST_SUITE_REGISTRATION( NetrcTest );

void NetrcTest::testFindAuthenticator()
{
  Netrc netrc;
  netrc.addAuthenticator
    (SharedHandle<Authenticator>(new Authenticator("host1", "tujikawa", "tujikawapasswd", "tujikawaaccount")));
  netrc.addAuthenticator
    (SharedHandle<Authenticator>(new Authenticator("host2", "aria2", "aria2password", "aria2account")));
  netrc.addAuthenticator
    (SharedHandle<Authenticator>(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount")));

  SharedHandle<Authenticator> aria2auth = netrc.findAuthenticator("host2");
  CPPUNIT_ASSERT(!aria2auth.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), aria2auth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2password"), aria2auth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2account"), aria2auth->getAccount());

  SharedHandle<Authenticator> defaultauth = netrc.findAuthenticator("host3");
  CPPUNIT_ASSERT(!defaultauth.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("default"), defaultauth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("defaultpassword"), defaultauth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("defaultaccount"), defaultauth->getAccount());
}

void NetrcTest::testParse()
{
  Netrc netrc;
  netrc.parse("sample.netrc");
  std::deque<SharedHandle<Authenticator> >::const_iterator itr = netrc.getAuthenticators().begin();

  SharedHandle<Authenticator> tujikawaauth = *itr;
  CPPUNIT_ASSERT(!tujikawaauth.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("host1"), tujikawaauth->getMachine());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawa"), tujikawaauth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawapassword"), tujikawaauth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawaaccount"), tujikawaauth->getAccount());
  ++itr;
  SharedHandle<Authenticator> aria2auth = *itr;
  CPPUNIT_ASSERT(!aria2auth.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("host2"), aria2auth->getMachine());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), aria2auth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2password"), aria2auth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2account"), aria2auth->getAccount());
  ++itr;
  SharedHandle<Authenticator> defaultauth = *itr;
  CPPUNIT_ASSERT(!defaultauth.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("anonymous"), defaultauth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("ARIA2@USER"), defaultauth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("ARIA2@ACCT"), defaultauth->getAccount());
}

void NetrcTest::testParse_fileNotFound()
{
  Netrc netrc;
  try {
    netrc.parse("");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void NetrcTest::testParse_emptyfile()
{
  Netrc netrc;
  netrc.parse("emptyfile");

  CPPUNIT_ASSERT_EQUAL((size_t)0, netrc.getAuthenticators().size());
}

void NetrcTest::testParse_malformedNetrc()
{
  Netrc netrc;
  try {
    netrc.parse("malformed.netrc");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

} // namespace aria2
