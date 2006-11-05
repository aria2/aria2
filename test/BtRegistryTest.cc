#include "BtRegistry.h"
#include "Exception.h"
#include "MockPeerStorage.h"
#include "MockPieceStorage.h"
#include "MockBtAnnounce.h"
#include "MockBtProgressInfoFile.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtRegistryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtRegistryTest);
  CPPUNIT_TEST(testGetPeerStorage);
  CPPUNIT_TEST(testGetPieceStorage);
  CPPUNIT_TEST(testGetBtRuntime);
  CPPUNIT_TEST(testGetBtAnnounce);
  CPPUNIT_TEST(testGetBtProgressInfoFile);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGetPeerStorage();
  void testGetPieceStorage();
  void testGetBtRuntime();
  void testGetBtAnnounce();
  void testGetBtProgressInfoFile();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtRegistryTest );

void BtRegistryTest::testGetPeerStorage() {
  CPPUNIT_ASSERT(!BtRegistry::getPeerStorage("test").get());

  PeerStorageHandle peerStorage(new MockPeerStorage());

  CPPUNIT_ASSERT(BtRegistry::registerPeerStorage("test",
						 peerStorage));
  CPPUNIT_ASSERT_EQUAL(peerStorage.get(),
		       BtRegistry::getPeerStorage("test").get());
}

void BtRegistryTest::testGetPieceStorage() {
  CPPUNIT_ASSERT(!BtRegistry::getPieceStorage("test").get());

  PieceStorageHandle pieceStorage(new MockPieceStorage());

  CPPUNIT_ASSERT(BtRegistry::registerPieceStorage("test",
						  pieceStorage));
  CPPUNIT_ASSERT_EQUAL(pieceStorage.get(),
		       BtRegistry::getPieceStorage("test").get());
}

void BtRegistryTest::testGetBtRuntime() {
  CPPUNIT_ASSERT(!BtRegistry::getBtRuntime("test").get());

  BtRuntimeHandle runtime;

  CPPUNIT_ASSERT(BtRegistry::registerBtRuntime("test", runtime));
  CPPUNIT_ASSERT_EQUAL(runtime.get(),
		       BtRegistry::getBtRuntime("test").get());
}

void BtRegistryTest::testGetBtAnnounce() {
  CPPUNIT_ASSERT(!BtRegistry::getBtAnnounce("test").get());
  
  BtAnnounceHandle btAnnounce(new MockBtAnnounce());

  CPPUNIT_ASSERT(BtRegistry::registerBtAnnounce("test", btAnnounce));
  CPPUNIT_ASSERT_EQUAL(btAnnounce.get(),
		       BtRegistry::getBtAnnounce("test").get());
}

void BtRegistryTest::testGetBtProgressInfoFile() {
  CPPUNIT_ASSERT(!BtRegistry::getBtProgressInfoFile("test").get());

  BtProgressInfoFileHandle btProgressInfoFile(new MockBtProgressInfoFile());

  CPPUNIT_ASSERT(BtRegistry::registerBtProgressInfoFile("test",
  							btProgressInfoFile));
  CPPUNIT_ASSERT_EQUAL(btProgressInfoFile.get(),
  		       BtRegistry::getBtProgressInfoFile("test").get());
}
