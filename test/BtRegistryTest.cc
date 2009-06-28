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

namespace aria2 {

class BtRegistryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRegistryTest);
  CPPUNIT_TEST(testGetDownloadContext);
  CPPUNIT_TEST(testGetAllDownloadContext);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testRemoveAll);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testGetDownloadContext();
  void testGetAllDownloadContext();
  void testRemove();
  void testRemoveAll();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetDownloadContext()
{
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(btRegistry.getDownloadContext("test").isNull());
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  BtObject btObject;
  btObject._downloadContext = dctx;
  btRegistry.put("test", btObject);
  CPPUNIT_ASSERT_EQUAL(dctx.get(), btRegistry.getDownloadContext("test").get());
}

static void addTwoDownloadContext(BtRegistry& btRegistry)
{
  SharedHandle<DownloadContext> dctx1(new DownloadContext());
  SharedHandle<DownloadContext> dctx2(new DownloadContext());
  BtObject btObject1;
  btObject1._downloadContext = dctx1;
  BtObject btObject2;
  btObject2._downloadContext = dctx2;
  btRegistry.put("ctx1", btObject1);
  btRegistry.put("ctx2", btObject2);
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
  CPPUNIT_ASSERT(btRegistry.remove("ctx1"));
  CPPUNIT_ASSERT(btRegistry.get("ctx1").isNull());
  CPPUNIT_ASSERT(!btRegistry.get("ctx2").isNull());
}

void BtRegistryTest::testRemoveAll()
{
  BtRegistry btRegistry;
  addTwoDownloadContext(btRegistry);
  btRegistry.removeAll();
  CPPUNIT_ASSERT(btRegistry.get("ctx1").isNull());
  CPPUNIT_ASSERT(btRegistry.get("ctx2").isNull());
}

} // namespace aria2
