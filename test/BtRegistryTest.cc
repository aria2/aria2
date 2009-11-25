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

namespace aria2 {

class BtRegistryTest:public CppUnit::TestFixture {

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


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetDownloadContext()
{
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(btRegistry.getDownloadContext(1).isNull());
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  BtObject btObject;
  btObject._downloadContext = dctx;
  btRegistry.put(1, btObject);
  CPPUNIT_ASSERT_EQUAL(dctx.get(), btRegistry.getDownloadContext(1).get());
}

static void addTwoDownloadContext(BtRegistry& btRegistry)
{
  SharedHandle<DownloadContext> dctx1(new DownloadContext());
  SharedHandle<DownloadContext> dctx2(new DownloadContext());
  BtObject btObject1;
  btObject1._downloadContext = dctx1;
  BtObject btObject2;
  btObject2._downloadContext = dctx2;
  btRegistry.put(1, btObject1);
  btRegistry.put(2, btObject2);
}

void BtRegistryTest::testGetDownloadContext_infoHash()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  BDE attrs1 = BDE::dict();
  attrs1[bittorrent::INFO_HASH] = std::string("hash1");
  BDE attrs2 = BDE::dict();
  attrs2[bittorrent::INFO_HASH] = std::string("hash2");
  btRegistry.getDownloadContext(1)->setAttribute
    (bittorrent::BITTORRENT, attrs1);
  btRegistry.getDownloadContext(2)->setAttribute
    (bittorrent::BITTORRENT, attrs2);

  CPPUNIT_ASSERT(!btRegistry.getDownloadContext("hash1").isNull());
  CPPUNIT_ASSERT(btRegistry.getDownloadContext("hash1").get() ==
		 btRegistry.getDownloadContext(1).get());
  CPPUNIT_ASSERT(btRegistry.getDownloadContext("not exists").isNull());
}

void BtRegistryTest::testGetAllDownloadContext()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);

  std::vector<SharedHandle<DownloadContext> > result;
  btRegistry.getAllDownloadContext(std::back_inserter(result));
  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
}

void BtRegistryTest::testRemove()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  CPPUNIT_ASSERT(btRegistry.remove(1));
  CPPUNIT_ASSERT(btRegistry.get(1).isNull());
  CPPUNIT_ASSERT(!btRegistry.get(2).isNull());
}

void BtRegistryTest::testRemoveAll()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  btRegistry.removeAll();
  CPPUNIT_ASSERT(btRegistry.get(1).isNull());
  CPPUNIT_ASSERT(btRegistry.get(2).isNull());
}

} // namespace aria2
