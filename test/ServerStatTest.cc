#include "ServerStat.h"

#include <iostream>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"

namespace aria2 {

class ServerStatTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ServerStatTest);
  CPPUNIT_TEST(testSetStatus);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testSetStatus();
  void testToString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ServerStatTest);

void ServerStatTest::testSetStatus()
{
  ServerStat ss("localhost", "http");
  CPPUNIT_ASSERT_EQUAL(ServerStat::OK, ss.getStatus());
  ss.setStatus("ERROR");
  CPPUNIT_ASSERT_EQUAL(ServerStat::A2_ERROR, ss.getStatus());
  // See undefined status string will not change current status.
  ss.setStatus("__BADSTATUS");
  CPPUNIT_ASSERT_EQUAL(ServerStat::A2_ERROR, ss.getStatus());
  ss.setStatus("OK");
  CPPUNIT_ASSERT_EQUAL(ServerStat::OK, ss.getStatus());
  // See undefined status string will not change current status.
  ss.setStatus("__BADSTATUS");
  CPPUNIT_ASSERT_EQUAL(ServerStat::OK, ss.getStatus());  
}

void ServerStatTest::testToString()
{
  ServerStat localhost_http("localhost", "http");
  localhost_http.setDownloadSpeed(90000);
  localhost_http.setLastUpdated(Time(1000));
  localhost_http.setSingleConnectionAvgSpeed(101);
  localhost_http.setMultiConnectionAvgSpeed(102);
  localhost_http.setCounter(5);

  CPPUNIT_ASSERT_EQUAL
    (std::string
     ("host=localhost, protocol=http, dl_speed=90000,"
      " sc_avg_speed=101, mc_avg_speed=102,"
      " last_updated=1000, counter=5, status=OK"),
     localhost_http.toString());

  ServerStat localhost_ftp("localhost", "ftp");
  localhost_ftp.setDownloadSpeed(10000);
  localhost_ftp.setLastUpdated(Time(1210000000));
  localhost_ftp.setStatus("ERROR");

  CPPUNIT_ASSERT_EQUAL
    (std::string
     ("host=localhost, protocol=ftp, dl_speed=10000,"
      " sc_avg_speed=0, mc_avg_speed=0,"
      " last_updated=1210000000, counter=0, status=ERROR"),
     localhost_ftp.toString());
}

} // namespace aria2
