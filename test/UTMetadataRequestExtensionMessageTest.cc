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
#include "extension_message_test_helper.h"
#include "DlAbortEx.h"

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
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<WrapExtBtMessageFactory> messageFactory_;
  SharedHandle<MockBtMessageDispatcher> dispatcher_;
  SharedHandle<Peer> peer_;

  void setUp()
  {
    messageFactory_.reset(new WrapExtBtMessageFactory());
    dispatcher_.reset(new MockBtMessageDispatcher());
    dctx_.reset(new DownloadContext());
    SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
    dctx_->setAttribute(bittorrent::BITTORRENT, attrs);
    peer_.reset(new Peer("host", 6880));
    peer_->allocateSessionResource(0, 0);
    peer_->setExtension("ut_metadata", 1);
  }

  template<typename T>
  SharedHandle<T> getFirstDispatchedMessage()
  {
    SharedHandle<WrapExtBtMessage> wrapmsg =
      dynamic_pointer_cast<WrapExtBtMessage>
      (dispatcher_->messageQueue.front());
    
    SharedHandle<T> msg = dynamic_pointer_cast<T>(wrapmsg->m_);
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
  msg.setDownloadContext(dctx_);
  msg.setPeer(peer_);
  msg.setBtMessageFactory(messageFactory_.get());
  msg.setBtMessageDispatcher(dispatcher_.get());
  msg.doReceivedAction();

  SharedHandle<UTMetadataRejectExtensionMessage> m =
    getFirstDispatchedMessage<UTMetadataRejectExtensionMessage>();

  CPPUNIT_ASSERT(m);
  CPPUNIT_ASSERT_EQUAL((size_t)10, m->getIndex());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID());
}

void UTMetadataRequestExtensionMessageTest::testDoReceivedAction_data()
{
  UTMetadataRequestExtensionMessage msg(1);
  msg.setIndex(1);
  msg.setDownloadContext(dctx_);
  msg.setPeer(peer_);
  msg.setBtMessageFactory(messageFactory_.get());
  msg.setBtMessageDispatcher(dispatcher_.get());

  size_t metadataSize = METADATA_PIECE_SIZE*2;
  SharedHandle<TorrentAttribute> attrs = bittorrent::getTorrentAttrs(dctx_);
  std::string first(METADATA_PIECE_SIZE, '0');
  std::string second(METADATA_PIECE_SIZE, '1');
  attrs->metadata = first+second;
  attrs->metadataSize = metadataSize;

  msg.doReceivedAction();

  SharedHandle<UTMetadataDataExtensionMessage> m =
    getFirstDispatchedMessage<UTMetadataDataExtensionMessage>();

  CPPUNIT_ASSERT(m);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
  CPPUNIT_ASSERT_EQUAL(second, m->getData());
  CPPUNIT_ASSERT_EQUAL(metadataSize, m->getTotalSize());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID());

  dispatcher_->messageQueue.clear();

  msg.setIndex(2);

  metadataSize += 100;
  std::string third(100, '2');
  attrs->metadata = first+second+third;
  attrs->metadataSize = metadataSize;

  msg.doReceivedAction();

  m = getFirstDispatchedMessage<UTMetadataDataExtensionMessage>();

  CPPUNIT_ASSERT(m);
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
