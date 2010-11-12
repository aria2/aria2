#include "DefaultBtMessageFactory.h"

#include <cstring>

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "bittorrent_helper.h"
#include "DownloadContext.h"
#include "MockExtensionMessageFactory.h"
#include "BtExtendedMessage.h"
#include "BtPortMessage.h"
#include "Exception.h"
#include "FileEntry.h"

namespace aria2 {

class DefaultBtMessageFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultBtMessageFactoryTest);
  CPPUNIT_TEST(testCreateBtMessage_BtExtendedMessage);
  CPPUNIT_TEST(testCreatePortMessage);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<Peer> peer_;
  SharedHandle<MockExtensionMessageFactory> exmsgFactory_;
  SharedHandle<DefaultBtMessageFactory> factory_;
public:
  void setUp()
  {
    dctx_.reset(new DownloadContext());

    peer_.reset(new Peer("192.168.0.1", 6969));
    peer_->allocateSessionResource(1024, 1024*1024);
    peer_->setExtendedMessagingEnabled(true);

    exmsgFactory_.reset(new MockExtensionMessageFactory());

    factory_.reset(new DefaultBtMessageFactory());
    factory_->setDownloadContext(dctx_);
    factory_->setPeer(peer_);
    factory_->setExtensionMessageFactory(exmsgFactory_);
  }

  void testCreateBtMessage_BtExtendedMessage();
  void testCreatePortMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultBtMessageFactoryTest);

void DefaultBtMessageFactoryTest::testCreateBtMessage_BtExtendedMessage()
{
  // payload:{4:name3:foo}->11bytes
  std::string payload = "4:name3:foo";
  char msg[17];// 6+11bytes
  bittorrent::createPeerMessageString((unsigned char*)msg, sizeof(msg),
                                      13, 20);
  msg[5] = 1; // Set dummy extended message ID 1
  memcpy(msg+6, payload.c_str(), payload.size());
  
  SharedHandle<BtExtendedMessage> m
    (dynamic_pointer_cast<BtExtendedMessage>
     (factory_->createBtMessage((const unsigned char*)msg+4, sizeof(msg))));

  try {
    // disable extended messaging
    peer_->setExtendedMessagingEnabled(false);
    factory_->createBtMessage((const unsigned char*)msg+4, sizeof(msg));
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultBtMessageFactoryTest::testCreatePortMessage()
{
  {
    unsigned char data[7];
    bittorrent::createPeerMessageString(data, sizeof(data), 3, 9);
    bittorrent::setShortIntParam(&data[5], 6881);
    try {
      SharedHandle<BtPortMessage> m
        (dynamic_pointer_cast<BtPortMessage>
         (factory_->createBtMessage(&data[4], sizeof(data)-4)));
      CPPUNIT_ASSERT(m);
      CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getPort());
    } catch(Exception& e) {
      CPPUNIT_FAIL(e.stackTrace());
    }
  }
  {
    SharedHandle<BtPortMessage> m
      (dynamic_pointer_cast<BtPortMessage>(factory_->createPortMessage(6881)));
    CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getPort());
  }
}

} // namespace aria2
