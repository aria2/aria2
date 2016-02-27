#include "ServerStatMan.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "ServerStat.h"
#include "Exception.h"
#include "util.h"
#include "BufferedFile.h"
#include "TestUtil.h"

namespace aria2 {

class ServerStatManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ServerStatManTest);
  CPPUNIT_TEST(testAddAndFind);
  CPPUNIT_TEST(testSave);
  CPPUNIT_TEST(testLoad);
  CPPUNIT_TEST(testRemoveStaleServerStat);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testAddAndFind();
  void testSave();
  void testLoad();
  void testRemoveStaleServerStat();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServerStatManTest);

void ServerStatManTest::testAddAndFind()
{
  std::shared_ptr<ServerStat> localhost_http(
      new ServerStat("localhost", "http"));
  std::shared_ptr<ServerStat> localhost_ftp(new ServerStat("localhost", "ftp"));
  std::shared_ptr<ServerStat> mirror(new ServerStat("mirror", "http"));

  ServerStatMan ssm;
  CPPUNIT_ASSERT(ssm.add(localhost_http));
  CPPUNIT_ASSERT(!ssm.add(localhost_http));
  CPPUNIT_ASSERT(ssm.add(localhost_ftp));
  CPPUNIT_ASSERT(ssm.add(mirror));

  {
    std::shared_ptr<ServerStat> r = ssm.find("localhost", "http");
    CPPUNIT_ASSERT(r);
    CPPUNIT_ASSERT_EQUAL(std::string("localhost"), r->getHostname());
    CPPUNIT_ASSERT_EQUAL(std::string("http"), r->getProtocol());
  }
  {
    std::shared_ptr<ServerStat> r = ssm.find("mirror", "ftp");
    CPPUNIT_ASSERT(!r);
  }
}

void ServerStatManTest::testSave()
{
  std::shared_ptr<ServerStat> localhost_http(
      new ServerStat("localhost", "http"));
  localhost_http->setDownloadSpeed(25000);
  localhost_http->setSingleConnectionAvgSpeed(100);
  localhost_http->setMultiConnectionAvgSpeed(101);
  localhost_http->setCounter(5);
  localhost_http->setLastUpdated(Time(1210000000));
  std::shared_ptr<ServerStat> localhost_ftp(new ServerStat("localhost", "ftp"));
  localhost_ftp->setDownloadSpeed(30000);
  localhost_ftp->setLastUpdated(Time(1210000001));
  std::shared_ptr<ServerStat> mirror(new ServerStat("mirror", "http"));
  mirror->setDownloadSpeed(0);
  mirror->setStatus(ServerStat::A2_ERROR);
  mirror->setLastUpdated(Time(1210000002));

  ServerStatMan ssm;
  CPPUNIT_ASSERT(ssm.add(localhost_http));
  CPPUNIT_ASSERT(ssm.add(localhost_ftp));
  CPPUNIT_ASSERT(ssm.add(mirror));

  const char* filename = A2_TEST_OUT_DIR "/aria2_ServerStatManTest_testSave";
  CPPUNIT_ASSERT(ssm.save(filename));
  CPPUNIT_ASSERT_EQUAL(std::string("host=localhost, protocol=ftp,"
                                   " dl_speed=30000,"
                                   " sc_avg_speed=0,"
                                   " mc_avg_speed=0,"
                                   " last_updated=1210000001,"
                                   " counter=0,"
                                   " status=OK\n"

                                   "host=localhost, protocol=http,"
                                   " dl_speed=25000,"
                                   " sc_avg_speed=100,"
                                   " mc_avg_speed=101,"
                                   " last_updated=1210000000,"
                                   " counter=5,"
                                   " status=OK\n"

                                   "host=mirror, protocol=http,"
                                   " dl_speed=0,"
                                   " sc_avg_speed=0,"
                                   " mc_avg_speed=0,"
                                   " last_updated=1210000002,"
                                   " counter=0,"
                                   " status=ERROR\n"),
                       readFile(filename));
}

void ServerStatManTest::testLoad()
{
  const char* filename = A2_TEST_OUT_DIR "/aria2_ServerStatManTest_testLoad";
  std::string in =
      "host=localhost, protocol=ftp, dl_speed=30000, last_updated=1210000001, "
      "status=OK\n"
      "host=localhost, protocol=http, dl_speed=25000, sc_avg_speed=101, "
      "mc_avg_speed=102, last_updated=1210000000, counter=6, status=OK\n"
      "host=mirror, protocol=http, dl_speed=0, last_updated=1210000002, "
      "status=ERROR\n";
  BufferedFile fp(filename, BufferedFile::WRITE);
  CPPUNIT_ASSERT_EQUAL((size_t)in.size(), fp.write(in.data(), in.size()));
  CPPUNIT_ASSERT(fp.close() != EOF);

  ServerStatMan ssm;
  CPPUNIT_ASSERT(ssm.load(filename));

  std::shared_ptr<ServerStat> localhost_http = ssm.find("localhost", "http");
  CPPUNIT_ASSERT(localhost_http);
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), localhost_http->getHostname());
  CPPUNIT_ASSERT_EQUAL(std::string("http"), localhost_http->getProtocol());
  CPPUNIT_ASSERT_EQUAL(25000, localhost_http->getDownloadSpeed());
  CPPUNIT_ASSERT_EQUAL(101, localhost_http->getSingleConnectionAvgSpeed());
  CPPUNIT_ASSERT_EQUAL(102, localhost_http->getMultiConnectionAvgSpeed());
  CPPUNIT_ASSERT_EQUAL(6, localhost_http->getCounter());
  CPPUNIT_ASSERT_EQUAL(static_cast<time_t>(1210000000),
                       localhost_http->getLastUpdated().getTimeFromEpoch());
  CPPUNIT_ASSERT_EQUAL(ServerStat::OK, localhost_http->getStatus());

  std::shared_ptr<ServerStat> mirror = ssm.find("mirror", "http");
  CPPUNIT_ASSERT(mirror);
  CPPUNIT_ASSERT_EQUAL(ServerStat::A2_ERROR, mirror->getStatus());
}

void ServerStatManTest::testRemoveStaleServerStat()
{
  Time now;
  std::shared_ptr<ServerStat> localhost_http(
      new ServerStat("localhost", "http"));
  localhost_http->setDownloadSpeed(25000);
  localhost_http->setLastUpdated(now);
  std::shared_ptr<ServerStat> localhost_ftp(new ServerStat("localhost", "ftp"));
  localhost_ftp->setDownloadSpeed(30000);
  localhost_ftp->setLastUpdated(Time(1210000001));
  std::shared_ptr<ServerStat> mirror(new ServerStat("mirror", "http"));
  mirror->setDownloadSpeed(0);
  mirror->setStatus(ServerStat::A2_ERROR);
  mirror->setLastUpdated(Time(1210000002));

  ServerStatMan ssm;
  CPPUNIT_ASSERT(ssm.add(localhost_http));
  CPPUNIT_ASSERT(ssm.add(localhost_ftp));
  CPPUNIT_ASSERT(ssm.add(mirror));

  ssm.removeStaleServerStat(24_h);

  CPPUNIT_ASSERT(ssm.find("localhost", "http"));
  CPPUNIT_ASSERT(!ssm.find("localhost", "ftp"));
  CPPUNIT_ASSERT(!ssm.find("mirror", "http"));
}

} // namespace aria2
