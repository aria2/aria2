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
  CPPUNIT_TEST(testGetPeerStorage);
  CPPUNIT_TEST(testGetPieceStorage);
  CPPUNIT_TEST(testGetBtRuntime);
  CPPUNIT_TEST(testGetBtAnnounce);
  CPPUNIT_TEST(testGetBtProgressInfoFile);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void testGetBtContext();
  void testGetPeerStorage();
  void testGetPieceStorage();
  void testGetBtRuntime();
  void testGetBtAnnounce();
  void testGetBtProgressInfoFile();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetBtContext()
{
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(btRegistry.getBtContext("test").isNull());
  SharedHandle<BtContext> btContext(new MockBtContext());
  btRegistry.registerBtContext("test", btContext);
  CPPUNIT_ASSERT_EQUAL(btContext.get(),
		       btRegistry.getBtContext("test").get());
}

void BtRegistryTest::testGetPeerStorage() {
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getPeerStorage("test").get());

  SharedHandle<PeerStorage> peerStorage(new MockPeerStorage());

  btRegistry.registerPeerStorage("test", peerStorage);
  CPPUNIT_ASSERT_EQUAL(peerStorage.get(),
		       btRegistry.getPeerStorage("test").get());
}

void BtRegistryTest::testGetPieceStorage() {
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getPieceStorage("test").get());

  SharedHandle<PieceStorage> pieceStorage(new MockPieceStorage());

  btRegistry.registerPieceStorage("test", pieceStorage);
  CPPUNIT_ASSERT_EQUAL(pieceStorage.get(),
		       btRegistry.getPieceStorage("test").get());
}

void BtRegistryTest::testGetBtRuntime() {
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getBtRuntime("test").get());

  SharedHandle<BtRuntime> runtime;

  btRegistry.registerBtRuntime("test", runtime);
  CPPUNIT_ASSERT_EQUAL(runtime.get(),
		       btRegistry.getBtRuntime("test").get());
}

void BtRegistryTest::testGetBtAnnounce() {
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getBtAnnounce("test").get());
  
  SharedHandle<BtAnnounce> btAnnounce(new MockBtAnnounce());

  btRegistry.registerBtAnnounce("test", btAnnounce);
  CPPUNIT_ASSERT_EQUAL(btAnnounce.get(),
		       btRegistry.getBtAnnounce("test").get());
}

void BtRegistryTest::testGetBtProgressInfoFile() {
  BtRegistry btRegistry;
  CPPUNIT_ASSERT(!btRegistry.getBtProgressInfoFile("test").get());

  SharedHandle<BtProgressInfoFile> btProgressInfoFile(new MockBtProgressInfoFile());

  btRegistry.registerBtProgressInfoFile("test", btProgressInfoFile);
  CPPUNIT_ASSERT_EQUAL(btProgressInfoFile.get(),
  		       btRegistry.getBtProgressInfoFile("test").get());
}

} // namespace aria2
