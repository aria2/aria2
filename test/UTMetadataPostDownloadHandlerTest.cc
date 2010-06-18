#include "UTMetadataPostDownloadHandler.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "FileEntry.h"
#include "bittorrent_helper.h"
#include "A2STR.h"
#include "ByteArrayDiskWriterFactory.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "util.h"
#include "MessageDigestHelper.h"
#include "prefs.h"

namespace aria2 {

class UTMetadataPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> _option;
  SharedHandle<DownloadContext> _dctx;
  SharedHandle<RequestGroup> _requestGroup;
public:
  void setUp()
  {
    _option.reset(new Option());
    _option->put("HELLO", "WORLD");
    _option->put(PREF_DIR, ".");
    _dctx.reset(new DownloadContext(0, 0, "something"));
    _requestGroup.reset(new RequestGroup(_option));
    _requestGroup->setDownloadContext(_dctx);
  }

  void testCanHandle();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UTMetadataPostDownloadHandlerTest );

void UTMetadataPostDownloadHandlerTest::testCanHandle()
{
  UTMetadataPostDownloadHandler handler;

  CPPUNIT_ASSERT(!handler.canHandle(_requestGroup.get()));

  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  _dctx->setAttribute(bittorrent::BITTORRENT, attrs);

  CPPUNIT_ASSERT(handler.canHandle(_requestGroup.get()));

  // Only checks whether metadata is empty or not
  attrs->metadata = "metadata";

  CPPUNIT_ASSERT(!handler.canHandle(_requestGroup.get()));
}

void UTMetadataPostDownloadHandlerTest::testGetNextRequestGroups()
{
  File trfile("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf.torrent");
  if(trfile.exists()) {
    trfile.remove();
  }
  std::string metadata =
    "d6:lengthi384e4:name19:aria2-0.8.2.tar.bz212:piece lengthi128e"
    "6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCe";
  unsigned char infoHash[20];
  MessageDigestHelper::digest
    (infoHash, sizeof(infoHash), MessageDigestContext::SHA1,
     reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size());
  _dctx->getFirstFileEntry()->setLength(metadata.size());
  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  attrs->infoHash = std::string(&infoHash[0], &infoHash[20]);
  std::vector<std::vector<std::string> > announceList;
  std::vector<std::string> announceTier;
  announceTier.push_back("http://tracker");
  announceList.push_back(announceTier);
  attrs->announceList = announceList;
  _dctx->setAttribute(bittorrent::BITTORRENT, attrs);
  _requestGroup->setDiskWriterFactory
    (SharedHandle<DiskWriterFactory>(new ByteArrayDiskWriterFactory()));
  _requestGroup->initPieceStorage();
  _requestGroup->getPieceStorage()->getDiskAdaptor()->writeData
    (reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size(),
     0);

  UTMetadataPostDownloadHandler handler;
  std::vector<SharedHandle<RequestGroup> > results;
  handler.getNextRequestGroups(results, _requestGroup.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1, results.size());
  SharedHandle<RequestGroup> newRg = results.front();
  SharedHandle<DownloadContext> newDctx = newRg->getDownloadContext();
  SharedHandle<TorrentAttribute> newAttrs =
    bittorrent::getTorrentAttrs(newDctx);
  CPPUNIT_ASSERT_EQUAL(bittorrent::getInfoHashString(_dctx),
                       bittorrent::getInfoHashString(newDctx));
  const std::vector<std::vector<std::string> >& newAnnounceList =
    newAttrs->announceList;
  CPPUNIT_ASSERT_EQUAL((size_t)1, newAnnounceList.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker"), newAnnounceList[0][0]);
  CPPUNIT_ASSERT_EQUAL(_option->get("Hello"),
                       newRg->getOption()->get("Hello"));
  CPPUNIT_ASSERT
    (std::find(_requestGroup->followedBy().begin(),
               _requestGroup->followedBy().end(),
               newRg->getGID()) != _requestGroup->followedBy().end());
  CPPUNIT_ASSERT(!trfile.exists());

  results.clear();

  _requestGroup->getOption()->put(PREF_BT_SAVE_METADATA, V_TRUE);
  handler.getNextRequestGroups(results, _requestGroup.get());
  CPPUNIT_ASSERT(trfile.exists());

  results.clear();

  // See failure with bad metadata
  metadata = "d6:lengthi384e4:name19:aria2-0.8.2.tar.bz212:piece lengthi128e";
  _requestGroup->initPieceStorage();
  _requestGroup->getPieceStorage()->getDiskAdaptor()->writeData
    (reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size(),
     0);
  try {
    handler.getNextRequestGroups(results, _requestGroup.get());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

} // namespace aria2
