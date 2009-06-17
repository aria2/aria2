#include "BtRegistry.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "MockBtAnnounce.h"
#include "MockBtProgressInfoFile.h"
#include "BtRuntime.h"
#include "FileEntry.h"

namespace aria2 {

class BtRegistryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRegistryTest);
  CPPUNIT_TEST(testGetBtContext);
  CPPUNIT_TEST(testGetAllBtContext);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST(testRemoveAll);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testGetBtContext();
  void testGetAllBtContext();
  void testRemove();
  void testRemoveAll();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetBtContext()
{
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(btRegistry.getBtContext("test").isNull());
  SharedHandle<BtContext> btContext(new MockBtContext());
  BtObject btObject;
  btObject._btContext = btContext;
  btRegistry.put("test", btObject);
  CPPUNIT_ASSERT_EQUAL(btContext.get(),
		       btRegistry.getBtContext("test").get());
}

static void addTwoBtContext(BtRegistry& btRegistry)
{
  SharedHandle<BtContext> btContext1(new MockBtContext());
  SharedHandle<BtContext> btContext2(new MockBtContext());
  BtObject btObject1;
  btObject1._btContext = btContext1;
  BtObject btObject2;
  btObject2._btContext = btContext2;
  btRegistry.put("ctx1", btObject1);
  btRegistry.put("ctx2", btObject2);
}

void BtRegistryTest::testGetAllBtContext()
{
  BtRegistry btRegistry;
  addTwoBtContext(btRegistry);

  std::vector<SharedHandle<BtContext> > result;
  btRegistry.getAllBtContext(std::back_inserter(result));
  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
}

void BtRegistryTest::testRemove()
{
  BtRegistry btRegistry;
  addTwoBtContext(btRegistry);
  CPPUNIT_ASSERT(btRegistry.remove("ctx1"));
  CPPUNIT_ASSERT(btRegistry.get("ctx1").isNull());
  CPPUNIT_ASSERT(!btRegistry.get("ctx2").isNull());
}

void BtRegistryTest::testRemoveAll()
{
  BtRegistry btRegistry;
  addTwoBtContext(btRegistry);
  btRegistry.removeAll();
  CPPUNIT_ASSERT(btRegistry.get("ctx1").isNull());
  CPPUNIT_ASSERT(btRegistry.get("ctx2").isNull());
}

} // namespace aria2
