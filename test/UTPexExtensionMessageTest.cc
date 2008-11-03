#include "UTPexExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "a2netcompat.h"
#include "Util.h"
#include "PeerMessageUtil.h"
#include "MockPeerStorage.h"
#include "Exception.h"
#include "FileEntry.h"

namespace aria2 {

class UTPexExtensionMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTPexExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetExtensionName);
  CPPUNIT_TEST(testGetBencodedData);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testAddFreshPeer);
  CPPUNIT_TEST(testAddDroppedPeer);
  CPPUNIT_TEST(testFreshPeersAreFull);
  CPPUNIT_TEST(testDroppedPeersAreFull);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MockPeerStorage> _peerStorage;
public:
  void setUp()
  {
    _peerStorage.reset(new MockPeerStorage());
  }

  void testGetExtensionMessageID();
  void testGetExtensionName();
  void testGetBencodedData();
  void testToString();
  void testDoReceivedAction();
  void testCreate();
  void testAddFreshPeer();
  void testAddDroppedPeer();
  void testFreshPeersAreFull();
  void testDroppedPeersAreFull();
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTPexExtensionMessageTest);

void UTPexExtensionMessageTest::testGetExtensionMessageID()
{
  UTPexExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg.getExtensionMessageID());
}

void UTPexExtensionMessageTest::testGetExtensionName()
{
  UTPexExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_pex"), msg.getExtensionName());
}

void UTPexExtensionMessageTest::testGetBencodedData()
{
  UTPexExtensionMessage msg(1);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  p1->allocateSessionResource(256*1024, 1024*1024);
  p1->setAllBitfield();
  msg.addFreshPeer(p1);// added seeder, check add.f flag
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  msg.addFreshPeer(p2);
  SharedHandle<Peer> p3(new Peer("192.168.0.2", 6882));
  p3->startBadCondition();
  msg.addDroppedPeer(p3);
  SharedHandle<Peer> p4(new Peer("10.1.1.3", 10000));
  p4->startBadCondition();
  msg.addDroppedPeer(p4);

  unsigned char c1[6];
  unsigned char c2[6];
  unsigned char c3[6];
  unsigned char c4[6];
  PeerMessageUtil::createcompact(c1, p1->ipaddr, p1->port);
  PeerMessageUtil::createcompact(c2, p2->ipaddr, p2->port);
  PeerMessageUtil::createcompact(c3, p3->ipaddr, p3->port);
  PeerMessageUtil::createcompact(c4, p4->ipaddr, p4->port);

  std::string expected = "d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:207:dropped12:"+
    std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "e";
  std::string bd = msg.getBencodedData();
  CPPUNIT_ASSERT_EQUAL(Util::urlencode(expected),
		       Util::urlencode(bd));
}

void UTPexExtensionMessageTest::testToString()
{
  UTPexExtensionMessage msg(1);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  p1->allocateSessionResource(256*1024, 1024*1024);
  p1->setAllBitfield();
  msg.addFreshPeer(p1);// added seeder, check add.f flag
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  msg.addFreshPeer(p2);
  SharedHandle<Peer> p3(new Peer("192.168.0.2", 6882));
  p3->startBadCondition();
  msg.addDroppedPeer(p3);
  SharedHandle<Peer> p4(new Peer("10.1.1.3", 10000));
  p4->startBadCondition();
  msg.addDroppedPeer(p4);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_pex added=2, dropped=2"), msg.toString());
}

void UTPexExtensionMessageTest::testDoReceivedAction()
{
  UTPexExtensionMessage msg(1);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  p1->allocateSessionResource(256*1024, 1024*1024);
  p1->setAllBitfield();
  msg.addFreshPeer(p1);// added seeder, check add.f flag
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  msg.addFreshPeer(p2);
  SharedHandle<Peer> p3(new Peer("192.168.0.2", 6882));
  p3->startBadCondition();
  msg.addDroppedPeer(p3);
  SharedHandle<Peer> p4(new Peer("10.1.1.3", 10000));
  p4->startBadCondition();
  msg.addDroppedPeer(p4);
  msg.setPeerStorage(_peerStorage);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)2, _peerStorage->getPeers().size());
  {
    SharedHandle<Peer> p = _peerStorage->getPeers()[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), p->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p->port);
  }
  {
    SharedHandle<Peer> p = _peerStorage->getPeers()[1];
    CPPUNIT_ASSERT_EQUAL(std::string("10.1.1.2"), p->ipaddr);
    CPPUNIT_ASSERT_EQUAL((uint16_t)9999, p->port);
  }
}

void UTPexExtensionMessageTest::testCreate()
{
  unsigned char c1[6];
  unsigned char c2[6];
  unsigned char c3[6];
  unsigned char c4[6];
  PeerMessageUtil::createcompact(c1, "192.168.0.1", 6881);
  PeerMessageUtil::createcompact(c2, "10.1.1.2", 9999);
  PeerMessageUtil::createcompact(c3, "192.168.0.2", 6882);
  PeerMessageUtil::createcompact(c4, "10.1.1.3",10000);

  char id[1] = { 1 };

  std::string data = std::string(&id[0], &id[1])+"d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:207:dropped12:"+
    std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "e";
  
  SharedHandle<UTPexExtensionMessage> msg =
    UTPexExtensionMessage::create
    (reinterpret_cast<const unsigned char*>(data.c_str()), data.size());

  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg->getExtensionMessageID());
  CPPUNIT_ASSERT_EQUAL((size_t)2, msg->getFreshPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
		       msg->getFreshPeers()[0]->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, msg->getFreshPeers()[0]->port);
  CPPUNIT_ASSERT_EQUAL(std::string("10.1.1.2"),
		       msg->getFreshPeers()[1]->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)9999, msg->getFreshPeers()[1]->port);
  CPPUNIT_ASSERT_EQUAL((size_t)2, msg->getDroppedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"),
		       msg->getDroppedPeers()[0]->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)6882, msg->getDroppedPeers()[0]->port);
  CPPUNIT_ASSERT_EQUAL(std::string("10.1.1.3"),
		       msg->getDroppedPeers()[1]->ipaddr);
  CPPUNIT_ASSERT_EQUAL((uint16_t)10000,
		       msg->getDroppedPeers()[1]->port);
  try {
    // 0 length data
    std::string in = "";
    UTPexExtensionMessage::create
      (reinterpret_cast<const unsigned char*>(in.c_str()), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }    
}

void UTPexExtensionMessageTest::testAddFreshPeer()
{
  UTPexExtensionMessage msg(1);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  CPPUNIT_ASSERT(msg.addFreshPeer(p1));
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  p2->setFirstContactTime(Time(Time().getTime()-61));
  CPPUNIT_ASSERT(!msg.addFreshPeer(p2));
  SharedHandle<Peer> p3(new Peer("10.1.1.3", 9999, true));
  CPPUNIT_ASSERT(!msg.addFreshPeer(p3));
}

void UTPexExtensionMessageTest::testAddDroppedPeer()
{
  UTPexExtensionMessage msg(1);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  CPPUNIT_ASSERT(!msg.addDroppedPeer(p1));
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  p2->startBadCondition();
  CPPUNIT_ASSERT(msg.addFreshPeer(p2));
  SharedHandle<Peer> p3(new Peer("10.1.1.3", 9999, true));
  p3->startBadCondition();
  CPPUNIT_ASSERT(!msg.addDroppedPeer(p3));
}

void UTPexExtensionMessageTest::testFreshPeersAreFull()
{
  UTPexExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((size_t)30, msg.getMaxFreshPeer());
  msg.setMaxFreshPeer(2);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  CPPUNIT_ASSERT(msg.addFreshPeer(p1));
  CPPUNIT_ASSERT(!msg.freshPeersAreFull());
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  CPPUNIT_ASSERT(msg.addFreshPeer(p2));
  CPPUNIT_ASSERT(msg.freshPeersAreFull());
  SharedHandle<Peer> p3(new Peer("10.1.1.3", 9999));
  CPPUNIT_ASSERT(msg.addFreshPeer(p3));
  CPPUNIT_ASSERT(msg.freshPeersAreFull());
}

void UTPexExtensionMessageTest::testDroppedPeersAreFull()
{
  UTPexExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((size_t)10, msg.getMaxDroppedPeer());
  msg.setMaxDroppedPeer(2);
  SharedHandle<Peer> p1(new Peer("192.168.0.1", 6881));
  p1->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p1));
  CPPUNIT_ASSERT(!msg.droppedPeersAreFull());
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  p2->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p2));
  CPPUNIT_ASSERT(msg.droppedPeersAreFull());
  SharedHandle<Peer> p3(new Peer("10.1.1.3", 9999));
  p3->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p3));
  CPPUNIT_ASSERT(msg.droppedPeersAreFull());
}

} // namespace aria2
