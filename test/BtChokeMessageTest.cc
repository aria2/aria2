#include "BtChokeMessage.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtRequestFactory.h"
#include "Peer.h"
#include "FileEntry.h"
#include "SocketBuffer.h"

namespace aria2 {

class BtChokeMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtChokeMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  std::shared_ptr<Peer> peer;

  void setUp()
  {
    peer.reset(new Peer("host", 6969));
    peer->allocateSessionResource(1_k, 1_m);
  }

  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testToString();

  class MockBtMessageDispatcher2 : public MockBtMessageDispatcher {
  public:
    bool doChokedActionCalled;
    bool doChokingActionCalled;

  public:
    MockBtMessageDispatcher2()
        : doChokedActionCalled{false}, doChokingActionCalled{false}
    {
    }

    virtual void doChokedAction() CXX11_OVERRIDE
    {
      doChokedActionCalled = true;
    }

    virtual void doChokingAction() CXX11_OVERRIDE
    {
      doChokingActionCalled = true;
    }
  };

  class MockBtRequestFactory2 : public MockBtRequestFactory {
  public:
    bool doChokedActionCalled;

  public:
    MockBtRequestFactory2() : doChokedActionCalled{false} {}

    virtual void doChokedAction() CXX11_OVERRIDE
    {
      doChokedActionCalled = true;
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtChokeMessageTest);

void BtChokeMessageTest::testCreate()
{
  unsigned char msg[5];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 0);
  auto pm = BtChokeMessage::create(&msg[4], 1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)0, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 0);
    BtChokeMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 1);
    BtChokeMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtChokeMessageTest::testCreateMessage()
{
  BtChokeMessage msg;
  unsigned char data[5];
  bittorrent::createPeerMessageString(data, sizeof(data), 1, 0);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)5, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtChokeMessageTest::testDoReceivedAction()
{
  BtChokeMessage msg;
  msg.setPeer(peer);

  auto dispatcher = make_unique<MockBtMessageDispatcher2>();
  msg.setBtMessageDispatcher(dispatcher.get());
  auto requestFactory = make_unique<MockBtRequestFactory2>();
  msg.setBtRequestFactory(requestFactory.get());

  msg.doReceivedAction();

  CPPUNIT_ASSERT(dispatcher->doChokedActionCalled);
  CPPUNIT_ASSERT(peer->peerChoking());
}

void BtChokeMessageTest::testToString()
{
  BtChokeMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("choke"), msg.toString());
}

} // namespace aria2
