#include "UTPexExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "Peer.h"
#include "a2netcompat.h"
#include "util.h"
#include "bittorrent_helper.h"
#include "MockPeerStorage.h"
#include "Exception.h"
#include "FileEntry.h"
#include "wallclock.h"

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
  SharedHandle<MockPeerStorage> peerStorage_;
public:
  void setUp()
  {
    peerStorage_.reset(new MockPeerStorage());
    global::wallclock().reset();
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
  CPPUNIT_ASSERT(msg.addFreshPeer(p1));// added seeder, check add.f flag
  SharedHandle<Peer> p2(new Peer("10.1.1.2", 9999));
  CPPUNIT_ASSERT(msg.addFreshPeer(p2));
  SharedHandle<Peer> p3(new Peer("192.168.0.2", 6882));
  p3->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p3));
  SharedHandle<Peer> p4(new Peer("10.1.1.3", 10000));
  p4->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p4));

  SharedHandle<Peer> p5(new Peer("1002:1035:4527:3546:7854:1237:3247:3217",
                                 6881));
  CPPUNIT_ASSERT(msg.addFreshPeer(p5));
  SharedHandle<Peer> p6(new Peer("2001:db8:bd05:1d2:288a:1fc0:1:10ee", 6882));
  p6->startBadCondition();
  CPPUNIT_ASSERT(msg.addDroppedPeer(p6));

  unsigned char c1[COMPACT_LEN_IPV6];
  unsigned char c2[COMPACT_LEN_IPV6];
  unsigned char c3[COMPACT_LEN_IPV6];
  unsigned char c4[COMPACT_LEN_IPV6];
  unsigned char c5[COMPACT_LEN_IPV6];
  unsigned char c6[COMPACT_LEN_IPV6];
  bittorrent::packcompact(c1, p1->getIPAddress(), p1->getPort());
  bittorrent::packcompact(c2, p2->getIPAddress(), p2->getPort());
  bittorrent::packcompact(c3, p3->getIPAddress(), p3->getPort());
  bittorrent::packcompact(c4, p4->getIPAddress(), p4->getPort());
  bittorrent::packcompact(c5, p5->getIPAddress(), p5->getPort());
  bittorrent::packcompact(c6, p6->getIPAddress(), p6->getPort());

  std::string expected = "d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:"+fromHex("0200")+
    "6:added618:"+std::string(&c5[0], &c5[COMPACT_LEN_IPV6])+
    "8:added6.f1:"+fromHex("00")+
    "7:dropped12:"+std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "8:dropped618:"+std::string(&c6[0], &c6[COMPACT_LEN_IPV6])+
    "e";
  std::string bd = msg.getPayload();
  CPPUNIT_ASSERT_EQUAL(util::percentEncode(expected),
                       util::percentEncode(bd));
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
  SharedHandle<Peer> p2(new Peer("1002:1035:4527:3546:7854:1237:3247:3217",
                                 9999));
  msg.addFreshPeer(p2);
  SharedHandle<Peer> p3(new Peer("192.168.0.2", 6882));
  p3->startBadCondition();
  msg.addDroppedPeer(p3);
  SharedHandle<Peer> p4(new Peer("2001:db8:bd05:1d2:288a:1fc0:1:10ee", 10000));
  p4->startBadCondition();
  msg.addDroppedPeer(p4);
  msg.setPeerStorage(peerStorage_);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((size_t)4, peerStorage_->getPeers().size());
  {
    SharedHandle<Peer> p = peerStorage_->getPeers()[0];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), p->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, p->getPort());
  }
  {
    SharedHandle<Peer> p = peerStorage_->getPeers()[1];
    CPPUNIT_ASSERT_EQUAL(std::string("1002:1035:4527:3546:7854:1237:3247:3217"),
                         p->getIPAddress());
    CPPUNIT_ASSERT_EQUAL((uint16_t)9999, p->getPort());
  }
  {
    SharedHandle<Peer> p = peerStorage_->getPeers()[2];
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"), p->getIPAddress());
  }
  {
    SharedHandle<Peer> p = peerStorage_->getPeers()[3];
    CPPUNIT_ASSERT_EQUAL(std::string("2001:db8:bd05:1d2:288a:1fc0:1:10ee"),
                         p->getIPAddress());
  }
}

void UTPexExtensionMessageTest::testCreate()
{
  unsigned char c1[COMPACT_LEN_IPV6];
  unsigned char c2[COMPACT_LEN_IPV6];
  unsigned char c3[COMPACT_LEN_IPV6];
  unsigned char c4[COMPACT_LEN_IPV6];
  unsigned char c5[COMPACT_LEN_IPV6];
  unsigned char c6[COMPACT_LEN_IPV6];
  bittorrent::packcompact(c1, "192.168.0.1", 6881);
  bittorrent::packcompact(c2, "10.1.1.2", 9999);
  bittorrent::packcompact(c3, "192.168.0.2", 6882);
  bittorrent::packcompact(c4, "10.1.1.3",10000);
  bittorrent::packcompact(c5, "1002:1035:4527:3546:7854:1237:3247:3217", 6997);
  bittorrent::packcompact(c6, "2001:db8:bd05:1d2:288a:1fc0:1:10ee",6998);

  char id[1] = { 1 };

  std::string data = std::string(&id[0], &id[1])+"d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:"+fromHex("0200")+
    "6:added618:"+std::string(&c5[0], &c5[COMPACT_LEN_IPV6])+
    "8:added6.f1:"+fromHex("00")+
    "7:dropped12:"+std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "8:dropped618:"+std::string(&c6[0], &c6[COMPACT_LEN_IPV6])+
    "e";
  
  SharedHandle<UTPexExtensionMessage> msg =
    UTPexExtensionMessage::create
    (reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg->getExtensionMessageID());
  CPPUNIT_ASSERT_EQUAL((size_t)3, msg->getFreshPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"),
                       msg->getFreshPeers()[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, msg->getFreshPeers()[0]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("10.1.1.2"),
                       msg->getFreshPeers()[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)9999, msg->getFreshPeers()[1]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("1002:1035:4527:3546:7854:1237:3247:3217"),
                       msg->getFreshPeers()[2]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6997, msg->getFreshPeers()[2]->getPort());

  CPPUNIT_ASSERT_EQUAL((size_t)3, msg->getDroppedPeers().size());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.2"),
                       msg->getDroppedPeers()[0]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6882, msg->getDroppedPeers()[0]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("10.1.1.3"),
                       msg->getDroppedPeers()[1]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)10000,
                       msg->getDroppedPeers()[1]->getPort());
  CPPUNIT_ASSERT_EQUAL(std::string("2001:db8:bd05:1d2:288a:1fc0:1:10ee"),
                       msg->getDroppedPeers()[2]->getIPAddress());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6998,
                       msg->getDroppedPeers()[2]->getPort());
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
  p2->setFirstContactTime(Timer(Timer().getTime()-61));
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
  CPPUNIT_ASSERT_EQUAL((size_t)50, msg.getMaxFreshPeer());
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
  CPPUNIT_ASSERT_EQUAL((size_t)50, msg.getMaxDroppedPeer());
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
