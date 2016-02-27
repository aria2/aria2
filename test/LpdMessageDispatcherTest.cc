#include "LpdMessageDispatcher.h"

#include <cstring>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "Exception.h"
#include "util.h"
#include "LpdMessageDispatcher.h"
#include "SocketCore.h"
#include "BtConstants.h"

namespace aria2 {

class LpdMessageDispatcherTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LpdMessageDispatcherTest);
  CPPUNIT_TEST(testCreateLpdRequest);
  CPPUNIT_TEST(testSendMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreateLpdRequest();
  void testSendMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LpdMessageDispatcherTest);

void LpdMessageDispatcherTest::testCreateLpdRequest()
{
  std::string infoHashString = "cd41c7fdddfd034a15a04d7ff881216e01c4ceaf";
  CPPUNIT_ASSERT_EQUAL(
      std::string("BT-SEARCH * HTTP/1.1\r\n"
                  "Host: 239.192.152.143:6771\r\n"
                  "Port: 6000\r\n"
                  "Infohash: cd41c7fdddfd034a15a04d7ff881216e01c4ceaf\r\n"
                  "\r\n\r\n"),
      bittorrent::createLpdRequest("239.192.152.143", 6771,
                                   fromHex(infoHashString), 6000));
}

void LpdMessageDispatcherTest::testSendMessage()
{
  std::shared_ptr<SocketCore> recvsock(new SocketCore(SOCK_DGRAM));
#ifdef __MINGW32__
  recvsock->bindWithFamily(LPD_MULTICAST_PORT, AF_INET);
#else  // !__MINGW32__
  recvsock->bind(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT, AF_INET);
#endif // !__MINGW32__
  recvsock->joinMulticastGroup(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT, "");
  recvsock->setNonBlockingMode();

  LpdMessageDispatcher d("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf", 6000,
                         LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT);
  d.init("", 0, 1);

  CPPUNIT_ASSERT(d.sendMessage());

  unsigned char buf[200];

  Endpoint remoteEndpoint;
  ssize_t nbytes;
  int trycnt;
  for (trycnt = 0; trycnt < 5; ++trycnt) {
    nbytes = recvsock->readDataFrom(buf, sizeof(buf), remoteEndpoint);
    if (nbytes == 0) {
      util::sleep(1);
    }
    else {
      break;
    }
  }
  if (trycnt == 5) {
    CPPUNIT_FAIL("[TIMEOUT] No Multicast packet received.");
  }
  buf[nbytes] = '\0';
  std::stringstream temp;
  temp << "BT-SEARCH * HTTP/1.1\r\n"
       << "Host: " << LPD_MULTICAST_ADDR << ":" << LPD_MULTICAST_PORT << "\r\n"
       << "Port: " << d.getPort() << "\r\n"
       << "Infohash: " << util::toHex(d.getInfoHash()) << "\r\n"
       << "\r\n\r\n";
  CPPUNIT_ASSERT_EQUAL(temp.str(), std::string(&buf[0], &buf[nbytes]));
}

} // namespace aria2
