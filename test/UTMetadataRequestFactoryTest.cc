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

class UTMetadataRequestFactoryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRequestFactoryTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreate();

  class MockPieceStorage2 : public MockPieceStorage {
  public:
    std::deque<size_t> missingIndexes;

    virtual std::shared_ptr<Piece>
    getMissingPiece(const std::shared_ptr<Peer>& peer,
                    const std::vector<size_t>& exlucdedIndexes,
                    cuid_t cuid) CXX11_OVERRIDE
    {
      if (missingIndexes.empty()) {
        return nullptr;
      }
      else {
        size_t index = missingIndexes.front();
        missingIndexes.pop_front();
        return std::make_shared<Piece>(index, 0);
      }
    }
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestFactoryTest);

void UTMetadataRequestFactoryTest::testCreate()
{
  UTMetadataRequestFactory factory;
  DownloadContext dctx{METADATA_PIECE_SIZE, METADATA_PIECE_SIZE * 2};
  factory.setDownloadContext(&dctx);
  MockPieceStorage2 ps;
  ps.missingIndexes.push_back(0);
  ps.missingIndexes.push_back(1);
  WrapExtBtMessageFactory messageFactory;
  factory.setBtMessageFactory(&messageFactory);
  auto peer = std::make_shared<Peer>("peer", 6880);
  peer->allocateSessionResource(0, 0);
  factory.setPeer(peer);
  UTMetadataRequestTracker tracker;
  factory.setUTMetadataRequestTracker(&tracker);

  auto msgs = factory.create(1, &ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  msgs = factory.create(1, &ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  msgs = factory.create(1, &ps);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msgs.size());
}

} // namespace aria2
