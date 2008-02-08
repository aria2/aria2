#include "TrackerWatcherCommand.h"
#include "TorrentConsoleDownloadEngine.h"
#include "MetaFileUtil.h"
#include "Exception.h"
#include "prefs.h"
#include "HttpInitiateConnectionCommand.h"
#include "ByteArrayDiskWriter.h"
#include "DefaultBtContext.h"
#include "DefaultBtAnnounce.h"
#include "DefaultPieceStorage.h"
#include "DefaultPeerStorage.h"
#include "BtRegistry.h"
#include "RequestFactory.h"
#include "CUIDCounter.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class TrackerWatcherCommandTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TrackerWatcherCommandTest);
  CPPUNIT_TEST(testCreateCommand);
  CPPUNIT_TEST_SUITE_END();
private:
  Option op;
public:
  TrackerWatcherCommandTest()
  {
  }

  void setUp() 
  {
    CUIDCounterHandle counter = new CUIDCounter();
    CUIDCounterSingletonHolder::instance(counter);

    op.put(PREF_TRACKER_MAX_TRIES, "10");
    RequestFactoryHandle requestFactory = new RequestFactory();
    requestFactory->setOption(&op);
    RequestFactorySingletonHolder::instance(requestFactory);
  }

  void testCreateCommand();
};


CPPUNIT_TEST_SUITE_REGISTRATION( TrackerWatcherCommandTest );

void TrackerWatcherCommandTest::testCreateCommand() {
  try {
    
    BtContextHandle btContext(new DefaultBtContext());
    btContext->load("test.torrent");
    
    SharedHandle<BtRuntime> btRuntime;
    BtRegistry::registerBtRuntime(btContext->getInfoHashAsString(), btRuntime);
    
    PieceStorageHandle pieceStorage(new DefaultPieceStorage(btContext, &op));
    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     pieceStorage);

    PeerStorageHandle peerStorage(new DefaultPeerStorage(btContext, &op));
    BtRegistry::registerPeerStorage(btContext->getInfoHashAsString(),
				    peerStorage);

    BtAnnounceHandle btAnnounce(new DefaultBtAnnounce(btContext, &op));
    BtRegistry::registerBtAnnounce(btContext->getInfoHashAsString(), btAnnounce);
    TorrentConsoleDownloadEngine* te = new TorrentConsoleDownloadEngine();
    te->option = &op;
    te->_requestGroupMan = new RequestGroupMan();

    TrackerWatcherCommand command(1, te, btContext);

    RequestFactorySingletonHolder::instance()->createHttpAuthResolver();

    CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
    //cerr << btAnnounce->getAnnounceUrl() << endl;
    
    btAnnounce->announceSuccess();
    btAnnounce->resetAnnounce();
    te->_requestGroupMan = new RequestGroupMan();
    
    btRuntime->setHalt(true);
    
    CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
    //cerr << btAnnounce->getAnnounceUrl() << endl;
    
    btAnnounce->announceSuccess();
    btAnnounce->resetAnnounce();
    te->_requestGroupMan = new RequestGroupMan();
    
    CPPUNIT_ASSERT(btAnnounce->noMoreAnnounce());
    
  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
  }
}
