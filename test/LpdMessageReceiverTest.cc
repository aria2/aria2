#include "LpdMessageReceiver.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "Exception.h"
#include "util.h"
#include "LpdMessageReceiver.h"
#include "SocketCore.h"
#include "BtConstants.h"
#include "LpdMessage.h"
#include "LpdMessageDispatcher.h"
#include "Peer.h"

namespace aria2 {

class LpdMessageReceiverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LpdMessageReceiverTest);
  CPPUNIT_TEST(testReceiveMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void testReceiveMessage();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LpdMessageReceiverTest);

void LpdMessageReceiverTest::testReceiveMessage()
{
  LpdMessageReceiver rcv(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT);
  CPPUNIT_ASSERT(rcv.init(""));

  std::shared_ptr<SocketCore> sendsock(new SocketCore(SOCK_DGRAM));
  sendsock->create(AF_INET);
  // Mingw32 build needs to set interface explicitly.
  sendsock->setMulticastInterface("");
  sendsock->setMulticastTtl(0);

  std::string infoHashString = "cd41c7fdddfd034a15a04d7ff881216e01c4ceaf";
  std::string infoHash = fromHex(infoHashString);
  std::string request = bittorrent::createLpdRequest(
      LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT, infoHash, 6000);

  sendsock->writeData(request.c_str(), request.size(), LPD_MULTICAST_ADDR,
                      LPD_MULTICAST_PORT);

  rcv.getSocket()->isReadable(5);
  auto msg = rcv.receiveMessage();
  CPPUNIT_ASSERT(msg);
  CPPUNIT_ASSERT_EQUAL(std::string("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf"),
                       util::toHex(msg->infoHash));
  CPPUNIT_ASSERT_EQUAL((uint16_t)6000, msg->peer->getPort());

  // Bad infohash
  std::string badInfoHashString = "cd41c7fdddfd034a15a04d7ff881216e01c4ce";
  request = bittorrent::createLpdRequest(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT,
                                         fromHex(badInfoHashString), 6000);
  sendsock->writeData(request.c_str(), request.size(), LPD_MULTICAST_ADDR,
                      LPD_MULTICAST_PORT);

  rcv.getSocket()->isReadable(5);
  msg = rcv.receiveMessage();
  CPPUNIT_ASSERT(!msg);

  // Bad port
  request = bittorrent::createLpdRequest(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT,
                                         infoHash, 0);
  sendsock->writeData(request.c_str(), request.size(), LPD_MULTICAST_ADDR,
                      LPD_MULTICAST_PORT);

  rcv.getSocket()->isReadable(5);
  msg = rcv.receiveMessage();
  CPPUNIT_ASSERT(!msg);

  // No data available
  msg = rcv.receiveMessage();
  CPPUNIT_ASSERT(!msg);
}

} // namespace aria2
