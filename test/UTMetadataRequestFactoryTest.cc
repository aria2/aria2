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

    virtual std::shared_ptr<Piece> getMissingPiece
    (const std::shared_ptr<Peer>& peer,
     const std::vector<size_t>& exlucdedIndexes,
     cuid_t cuid)
    {
      if(missingIndexes.empty()) {
        return std::shared_ptr<Piece>();
      } else {
        size_t index = missingIndexes.front();
        missingIndexes.pop_front();
        return std::shared_ptr<Piece>(new Piece(index, 0));
      }
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestFactoryTest);

void UTMetadataRequestFactoryTest::testCreate()
{
  UTMetadataRequestFactory factory;
  std::shared_ptr<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, METADATA_PIECE_SIZE*2));
  factory.setDownloadContext(dctx);
  std::shared_ptr<MockPieceStorage2> ps(new MockPieceStorage2());
  ps->missingIndexes.push_back(0);
  ps->missingIndexes.push_back(1);
  std::shared_ptr<WrapExtBtMessageFactory> messageFactory
    (new WrapExtBtMessageFactory());
  factory.setBtMessageFactory(messageFactory.get());
  std::shared_ptr<Peer> peer(new Peer("peer", 6880));
  peer->allocateSessionResource(0, 0);
  factory.setPeer(peer);
  std::shared_ptr<UTMetadataRequestTracker> tracker
    (new UTMetadataRequestTracker());
  factory.setUTMetadataRequestTracker(tracker.get());

  auto msgs = factory.create(1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  msgs = factory.create(1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  msgs = factory.create(1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msgs.size());
}

} // namespace aria2
