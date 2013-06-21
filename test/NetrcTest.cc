#include "Netrc.h"

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"

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
    (std::shared_ptr<Authenticator>(new Authenticator("host1", "tujikawa", "tujikawapasswd", "tujikawaaccount")));
  netrc.addAuthenticator
    (std::shared_ptr<Authenticator>(new Authenticator("host2", "aria2", "aria2password", "aria2account")));
  netrc.addAuthenticator
    (std::shared_ptr<Authenticator>(new Authenticator(".my.domain", "dmname", "dmpass", "dmaccount")));
  netrc.addAuthenticator
    (std::shared_ptr<Authenticator>(new DefaultAuthenticator("default", "defaultpassword", "defaultaccount")));

  std::shared_ptr<Authenticator> aria2auth = netrc.findAuthenticator("host2");
  CPPUNIT_ASSERT(aria2auth);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), aria2auth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2password"), aria2auth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2account"), aria2auth->getAccount());

  std::shared_ptr<Authenticator> defaultauth = netrc.findAuthenticator("host3");
  CPPUNIT_ASSERT(defaultauth);
  CPPUNIT_ASSERT_EQUAL(std::string("default"), defaultauth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("defaultpassword"), defaultauth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("defaultaccount"), defaultauth->getAccount());

  std::shared_ptr<Authenticator> domainMatchAuth =
    netrc.findAuthenticator("host3.my.domain");
  CPPUNIT_ASSERT(domainMatchAuth);
  CPPUNIT_ASSERT_EQUAL(std::string("dmname"), domainMatchAuth->getLogin());

  std::shared_ptr<Authenticator> domainMatchAuth2 =
    netrc.findAuthenticator("my.domain");
  CPPUNIT_ASSERT(domainMatchAuth2);
  CPPUNIT_ASSERT_EQUAL(std::string("default"), domainMatchAuth2->getLogin());
}

void NetrcTest::testParse()
{
  Netrc netrc;
  netrc.parse(A2_TEST_DIR"/sample.netrc");
  std::vector<std::shared_ptr<Authenticator> >::const_iterator itr =
    netrc.getAuthenticators().begin();

  std::shared_ptr<Authenticator> tujikawaauth = *itr;
  CPPUNIT_ASSERT(tujikawaauth);
  CPPUNIT_ASSERT_EQUAL(std::string("host1"), tujikawaauth->getMachine());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawa"), tujikawaauth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawapassword"), tujikawaauth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("tujikawaaccount"), tujikawaauth->getAccount());
  ++itr;
  std::shared_ptr<Authenticator> aria2auth = *itr;
  CPPUNIT_ASSERT(aria2auth);
  CPPUNIT_ASSERT_EQUAL(std::string("host2"), aria2auth->getMachine());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), aria2auth->getLogin());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2password"), aria2auth->getPassword());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2account"), aria2auth->getAccount());
  ++itr;
  std::shared_ptr<Authenticator> defaultauth = *itr;
  CPPUNIT_ASSERT(defaultauth);
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
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void NetrcTest::testParse_emptyfile()
{
  Netrc netrc;
  netrc.parse(A2_TEST_DIR"/emptyfile");

  CPPUNIT_ASSERT_EQUAL((size_t)0, netrc.getAuthenticators().size());
}

void NetrcTest::testParse_malformedNetrc()
{
  Netrc netrc;
  try {
    netrc.parse(A2_TEST_DIR"/malformed.netrc");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

} // namespace aria2
