#include "UTMetadataRequestFactory.h"

#include <vector>
#include <deque>

#include <cppunit/extensions/HelperMacros.h>

#include "MockPieceStorage.h"
#include "DownloadContext.h"
#include "Peer.h"
#include "BtMessage.h"
#include "extension_message_test_helper.h"
#include "BtHandshakeMessage.h"
#include "ExtensionMessage.h"
#include "UTMetadataRequestTracker.h"

namespace aria2 {

class UTMetadataRequestFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRequestFactoryTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST_SUITE_END();
public:
  void testCreate();

  class MockPieceStorage2:public MockPieceStorage {
  public:
    std::deque<size_t> missingIndexes;

    virtual SharedHandle<Piece> getMissingPiece
    (const SharedHandle<Peer>& peer,
     const std::vector<size_t>& exlucdedIndexes,
     cuid_t cuid)
    {
      if(missingIndexes.empty()) {
        return SharedHandle<Piece>();
      } else {
        size_t index = missingIndexes.front();
        missingIndexes.pop_front();
        return SharedHandle<Piece>(new Piece(index, 0));
      }
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestFactoryTest);

void UTMetadataRequestFactoryTest::testCreate()
{
  UTMetadataRequestFactory factory;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, METADATA_PIECE_SIZE*2));
  factory.setDownloadContext(dctx);
  SharedHandle<MockPieceStorage2> ps(new MockPieceStorage2());
  ps->missingIndexes.push_back(0);
  ps->missingIndexes.push_back(1);
  SharedHandle<WrapExtBtMessageFactory> messageFactory
    (new WrapExtBtMessageFactory());
  factory.setBtMessageFactory(messageFactory.get());
  SharedHandle<Peer> peer(new Peer("peer", 6880));
  peer->allocateSessionResource(0, 0);
  factory.setPeer(peer);
  SharedHandle<UTMetadataRequestTracker> tracker
    (new UTMetadataRequestTracker());
  factory.setUTMetadataRequestTracker(tracker.get());

  std::vector<SharedHandle<BtMessage> > msgs;

  factory.create(msgs, 1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  factory.create(msgs, 1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)2, msgs.size());

  factory.create(msgs, 1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)2, msgs.size());
}

} // namespace aria2
