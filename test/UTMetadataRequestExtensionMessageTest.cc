#include "UTMetadataRequestExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "DownloadContext.h"
#include "MockBtMessage.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtMessageFactory.h"
#include "bittorrent_helper.h"
#include "BtHandshakeMessage.h"
#include "UTMetadataRejectExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "extension_message_test_helper.h"

namespace aria2 {

class UTMetadataRequestExtensionMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRequestExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetExtensionName);
  CPPUNIT_TEST(testGetBencodedData);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction_reject);
  CPPUNIT_TEST(testDoReceivedAction_data);
  CPPUNIT_TEST_SUITE_END();
public:
  SharedHandle<DownloadContext> _dctx;
  SharedHandle<WrapExtBtMessageFactory> _messageFactory;
  SharedHandle<MockBtMessageDispatcher> _dispatcher;
  SharedHandle<Peer> _peer;

  void setUp()
  {
    _messageFactory.reset(new WrapExtBtMessageFactory());
    _dispatcher.reset(new MockBtMessageDispatcher());
    _dctx.reset(new DownloadContext());
    BDE attrs = BDE::dict();
    _dctx->setAttribute(bittorrent::BITTORRENT, attrs);
    _peer.reset(new Peer("host", 6880));
    _peer->allocateSessionResource(0, 0);
    _peer->setExtension("ut_metadata", 1);
  }

  template<typename T>
  SharedHandle<T> getFirstDispatchedMessage()
  {
    SharedHandle<WrapExtBtMessage> wrapmsg =
      dynamic_pointer_cast<WrapExtBtMessage>
      (_dispatcher->messageQueue.front());
    
    SharedHandle<T> msg = dynamic_pointer_cast<T>(wrapmsg->_m);
    return msg;
  }

  void testGetExtensionMessageID();
  void testGetExtensionName();
  void testGetBencodedData();
  void testToString();
  void testDoReceivedAction_reject();
  void testDoReceivedAction_data();
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestExtensionMessageTest);

void UTMetadataRequestExtensionMessageTest::testGetExtensionMessageID()
{
  UTMetadataRequestExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg.getExtensionMessageID());
}

void UTMetadataRequestExtensionMessageTest::testGetExtensionName()
{
  UTMetadataRequestExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_metadata"), msg.getExtensionName());
}

void UTMetadataRequestExtensionMessageTest::testGetBencodedData()
{
  UTMetadataRequestExtensionMessage msg(1);
  msg.setIndex(99);
  CPPUNIT_ASSERT_EQUAL
    (std::string("d8:msg_typei0e5:piecei99ee"), msg.getPayload());
}

void UTMetadataRequestExtensionMessageTest::testToString()
{
  UTMetadataRequestExtensionMessage msg(1);
  msg.setIndex(100);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_metadata request piece=100"),
		       msg.toString());
}

void UTMetadataRequestExtensionMessageTest::testDoReceivedAction_reject()
{
  UTMetadataRequestExtensionMessage msg(1);
  msg.setIndex(10);
  msg.setDownloadContext(_dctx);
  msg.setPeer(_peer);
  msg.setBtMessageFactory(_messageFactory);
  msg.setBtMessageDispatcher(_dispatcher);
  msg.doReceivedAction();

  SharedHandle<UTMetadataRejectExtensionMessage> m =
    getFirstDispatchedMessage<UTMetadataRejectExtensionMessage>();

  CPPUNIT_ASSERT(!m.isNull());
  CPPUNIT_ASSERT_EQUAL((size_t)10, m->getIndex());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID());
}

void UTMetadataRequestExtensionMessageTest::testDoReceivedAction_data()
{
  UTMetadataRequestExtensionMessage msg(1);
  msg.setIndex(1);
  msg.setDownloadContext(_dctx);
  msg.setPeer(_peer);
  msg.setBtMessageFactory(_messageFactory);
  msg.setBtMessageDispatcher(_dispatcher);

  size_t metadataSize = METADATA_PIECE_SIZE*2;
  BDE& attrs = _dctx->getAttribute(bittorrent::BITTORRENT);
  std::string first(METADATA_PIECE_SIZE, '0');
  std::string second(METADATA_PIECE_SIZE, '1');
  attrs[bittorrent::METADATA] = first+second;
  attrs[bittorrent::METADATA_SIZE] = metadataSize;

  msg.doReceivedAction();

  SharedHandle<UTMetadataDataExtensionMessage> m =
    getFirstDispatchedMessage<UTMetadataDataExtensionMessage>();

  CPPUNIT_ASSERT(!m.isNull());
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
  CPPUNIT_ASSERT_EQUAL(second, m->getData());
  CPPUNIT_ASSERT_EQUAL(metadataSize, m->getTotalSize());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID());

  _dispatcher->messageQueue.clear();

  msg.setIndex(2);

  metadataSize += 100;
  std::string third(100, '2');
  attrs[bittorrent::METADATA] = attrs[bittorrent::METADATA].s()+third;
  attrs[bittorrent::METADATA_SIZE] = metadataSize;

  msg.doReceivedAction();

  m = getFirstDispatchedMessage<UTMetadataDataExtensionMessage>();

  CPPUNIT_ASSERT(!m.isNull());
  CPPUNIT_ASSERT_EQUAL((size_t)2, m->getIndex());
  CPPUNIT_ASSERT_EQUAL(third, m->getData());
  CPPUNIT_ASSERT_EQUAL(metadataSize, m->getTotalSize());

  msg.setIndex(3);

  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    // success
  }
}

} // namespace aria2
