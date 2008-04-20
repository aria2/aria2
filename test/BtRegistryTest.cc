#include "BtRegistry.h"
#include "Exception.h"
#include "MockBtContext.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "MockBtAnnounce.h"
#include "MockBtProgressInfoFile.h"
#include "BtRuntime.h"
#include "FileEntry.h"
#include "PeerObject.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "BtMessageDispatcher.h"
#include "BtMessageReceiver.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtRegistryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRegistryTest);
  CPPUNIT_TEST(testGetBtContext);
  CPPUNIT_TEST(testGetPeerStorage);
  CPPUNIT_TEST(testGetPieceStorage);
  CPPUNIT_TEST(testGetBtRuntime);
  CPPUNIT_TEST(testGetBtAnnounce);
  CPPUNIT_TEST(testGetBtProgressInfoFile);
  CPPUNIT_TEST(testGetPeerObjectCluster);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp()
  {
    BtRegistry::unregisterAll();
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

  void testGetBtContext();
  void testGetPeerStorage();
  void testGetPieceStorage();
  void testGetBtRuntime();
  void testGetBtAnnounce();
  void testGetBtProgressInfoFile();
  void testGetPeerObjectCluster();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetBtContext()
{
  CPPUNIT_ASSERT(BtRegistry::getBtContext("test").isNull());
  SharedHandle<BtContext> btContext(new MockBtContext());
  BtRegistry::registerBtContext("test", btContext);
  CPPUNIT_ASSERT_EQUAL(btContext.get(),
		       BtRegistry::getBtContext("test").get());
}

void BtRegistryTest::testGetPeerStorage() {
  CPPUNIT_ASSERT(!BtRegistry::getPeerStorage("test").get());

  SharedHandle<PeerStorage> peerStorage(new MockPeerStorage());

  BtRegistry::registerPeerStorage("test", peerStorage);
  CPPUNIT_ASSERT_EQUAL(peerStorage.get(),
		       BtRegistry::getPeerStorage("test").get());
}

void BtRegistryTest::testGetPieceStorage() {
  CPPUNIT_ASSERT(!BtRegistry::getPieceStorage("test").get());

  SharedHandle<PieceStorage> pieceStorage(new MockPieceStorage());

  BtRegistry::registerPieceStorage("test", pieceStorage);
  CPPUNIT_ASSERT_EQUAL(pieceStorage.get(),
		       BtRegistry::getPieceStorage("test").get());
}

void BtRegistryTest::testGetBtRuntime() {
  CPPUNIT_ASSERT(!BtRegistry::getBtRuntime("test").get());

  SharedHandle<BtRuntime> runtime;

  BtRegistry::registerBtRuntime("test", runtime);
  CPPUNIT_ASSERT_EQUAL(runtime.get(),
		       BtRegistry::getBtRuntime("test").get());
}

void BtRegistryTest::testGetBtAnnounce() {
  CPPUNIT_ASSERT(!BtRegistry::getBtAnnounce("test").get());
  
  SharedHandle<BtAnnounce> btAnnounce(new MockBtAnnounce());

  BtRegistry::registerBtAnnounce("test", btAnnounce);
  CPPUNIT_ASSERT_EQUAL(btAnnounce.get(),
		       BtRegistry::getBtAnnounce("test").get());
}

void BtRegistryTest::testGetBtProgressInfoFile() {
  CPPUNIT_ASSERT(!BtRegistry::getBtProgressInfoFile("test").get());

  SharedHandle<BtProgressInfoFile> btProgressInfoFile(new MockBtProgressInfoFile());

  BtRegistry::registerBtProgressInfoFile("test", btProgressInfoFile);
  CPPUNIT_ASSERT_EQUAL(btProgressInfoFile.get(),
  		       BtRegistry::getBtProgressInfoFile("test").get());
}

void BtRegistryTest::testGetPeerObjectCluster() {
  CPPUNIT_ASSERT(!BtRegistry::getPeerObjectCluster("test").get());

  SharedHandle<PeerObjectCluster> cluster(new PeerObjectCluster());
  BtRegistry::registerPeerObjectCluster("test", cluster);

  CPPUNIT_ASSERT(BtRegistry::getPeerObjectCluster("test").get());
  
  BtRegistry::unregisterPeerObjectCluster("test");

  CPPUNIT_ASSERT(!BtRegistry::getPeerObjectCluster("test").get());
}

} // namespace aria2
