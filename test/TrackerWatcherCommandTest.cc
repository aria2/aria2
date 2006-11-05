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
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class TrackerWatcherCommandTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TrackerWatcherCommandTest);
  CPPUNIT_TEST(testCreateCommand);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreateCommand();
};


CPPUNIT_TEST_SUITE_REGISTRATION( TrackerWatcherCommandTest );

void TrackerWatcherCommandTest::testCreateCommand() {
  try {
    Option* op = new Option();
    op->put(PREF_TRACKER_MAX_TRIES, "10");
    
    BtContextHandle btContext(new DefaultBtContext());
    btContext->load("test.torrent");
    
    BtRuntimeHandle btRuntime;
    BtRegistry::registerBtRuntime(btContext->getInfoHashAsString(), btRuntime);
    
    PieceStorageHandle pieceStorage(new DefaultPieceStorage(btContext, op));
    BtRegistry::registerPieceStorage(btContext->getInfoHashAsString(),
				     pieceStorage);

    PeerStorageHandle peerStorage(new DefaultPeerStorage(btContext, op));
    BtRegistry::registerPeerStorage(btContext->getInfoHashAsString(),
				    peerStorage);

    BtAnnounceHandle btAnnounce(new DefaultBtAnnounce(btContext, op));
    BtRegistry::registerBtAnnounce(btContext->getInfoHashAsString(), btAnnounce);
    TorrentConsoleDownloadEngine* te = new TorrentConsoleDownloadEngine();
    te->option = op;
    te->segmentMan = new SegmentMan();
    te->segmentMan->option = op;
    ByteArrayDiskWriter* byteArrayDiskWriter = new ByteArrayDiskWriter();
    te->segmentMan->diskWriter = byteArrayDiskWriter;

    TrackerWatcherCommand command(1, te, btContext);

    CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
    //cerr << btAnnounce->getAnnounceUrl() << endl;
    
    btAnnounce->announceSuccess();
    btAnnounce->resetAnnounce();
    te->segmentMan->init();
    
    btRuntime->setHalt(true);
    
    CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
    //cerr << btAnnounce->getAnnounceUrl() << endl;
    
    btAnnounce->announceSuccess();
    btAnnounce->resetAnnounce();
    te->segmentMan->init();
    
    CPPUNIT_ASSERT(btAnnounce->noMoreAnnounce());
    
  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
  }
}
