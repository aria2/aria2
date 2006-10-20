#include "TrackerWatcherCommand.h"
#include "TorrentConsoleDownloadEngine.h"
#include "MetaFileUtil.h"
#include "Exception.h"
#include "prefs.h"
#include "HttpInitiateConnectionCommand.h"
#include "ByteArrayDiskWriter.h"
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

  TorrentConsoleDownloadEngine* te = new TorrentConsoleDownloadEngine();
  te->option = op;
  te->segmentMan = new SegmentMan();
  te->segmentMan->option = op;
  ByteArrayDiskWriter* byteArrayDiskWriter = new ByteArrayDiskWriter();
  te->segmentMan->diskWriter = byteArrayDiskWriter;
  te->torrentMan = new TorrentMan();
  te->torrentMan->option = op;
  te->torrentMan->setup("test.torrent", Strings());

  TrackerWatcherCommand command(1, te);

  CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
  cerr << te->torrentMan->getAnnounceUrl() << endl;

  te->torrentMan->announceSuccess();
  te->torrentMan->resetAnnounce();
  te->segmentMan->init();

  te->torrentMan->setHalt(true);

  CPPUNIT_ASSERT(dynamic_cast<HttpInitiateConnectionCommand*>(command.createCommand()));
  cerr << te->torrentMan->getAnnounceUrl() << endl;

  te->torrentMan->announceSuccess();
  te->torrentMan->resetAnnounce();
  te->segmentMan->init();

  CPPUNIT_ASSERT(te->torrentMan->noMoreAnnounce());

  } catch(Exception* e) {
    cerr << e->getMsg() << endl;
    delete e;
  }
}
