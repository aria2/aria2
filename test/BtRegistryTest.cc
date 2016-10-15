#include "BtRegistry.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "DownloadContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "MockBtAnnounce.h"
#include "MockBtProgressInfoFile.h"
#include "BtRuntime.h"
#include "FileEntry.h"
#include "bittorrent_helper.h"
#include "UDPTrackerRequest.h"

namespace aria2 {

class BtRegistryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRegistryTest);
  CPPUNIT_TEST(testGetDownloadContext);
  CPPUNIT_TEST(testGetDownloadContext_infoHash);
  CPPUNIT_TEST(testGetAllDownloadContext);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testRemoveAll);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testGetDownloadContext();
  void testGetDownloadContext_infoHash();
  void testGetAllDownloadContext();
  void testRemove();
  void testRemoveAll();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtRegistryTest);

void BtRegistryTest::testGetDownloadContext()
{
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getDownloadContext(1));
  auto dctx = std::make_shared<DownloadContext>();
  auto btObject = make_unique<BtObject>();
  btObject->downloadContext = dctx;
  btRegistry.put(1, std::move(btObject));
  CPPUNIT_ASSERT_EQUAL(dctx.get(), btRegistry.getDownloadContext(1).get());
}

namespace {
void addTwoDownloadContext(BtRegistry& btRegistry)
{
  auto dctx1 = std::make_shared<DownloadContext>();
  auto dctx2 = std::make_shared<DownloadContext>();
  auto btObject1 = make_unique<BtObject>();
  btObject1->downloadContext = dctx1;
  auto btObject2 = make_unique<BtObject>();
  btObject2->downloadContext = dctx2;
  btRegistry.put(1, std::move(btObject1));
  btRegistry.put(2, std::move(btObject2));
}
} // namespace

void BtRegistryTest::testGetDownloadContext_infoHash()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  {
    auto attrs1 = make_unique<TorrentAttribute>();
    attrs1->infoHash = "hash1";
    auto attrs2 = make_unique<TorrentAttribute>();
    attrs2->infoHash = "hash2";
    btRegistry.getDownloadContext(1)->setAttribute(CTX_ATTR_BT,
                                                   std::move(attrs1));
    btRegistry.getDownloadContext(2)->setAttribute(CTX_ATTR_BT,
                                                   std::move(attrs2));
  }
  CPPUNIT_ASSERT(btRegistry.getDownloadContext("hash1"));
  CPPUNIT_ASSERT(btRegistry.getDownloadContext("hash1").get() ==
                 btRegistry.getDownloadContext(1).get());
  CPPUNIT_ASSERT(!btRegistry.getDownloadContext("not exists"));
}

void BtRegistryTest::testGetAllDownloadContext()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);

  std::vector<std::shared_ptr<DownloadContext>> result;
  btRegistry.getAllDownloadContext(std::back_inserter(result));
  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
}

void BtRegistryTest::testRemove()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  CPPUNIT_ASSERT(btRegistry.remove(1));
  CPPUNIT_ASSERT(!btRegistry.get(1));
  CPPUNIT_ASSERT(btRegistry.get(2));
}

void BtRegistryTest::testRemoveAll()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  btRegistry.removeAll();
  CPPUNIT_ASSERT(!btRegistry.get(1));
  CPPUNIT_ASSERT(!btRegistry.get(2));
}

} // namespace aria2
