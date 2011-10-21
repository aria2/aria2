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
#include "MessageDigest.h"
#include "message_digest_helper.h"
#include "prefs.h"
#include "RecoverableException.h"

namespace aria2 {

class UTMetadataPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
  SharedHandle<DownloadContext> dctx_;
  SharedHandle<RequestGroup> requestGroup_;
public:
  void setUp()
  {
    option_.reset(new Option());
    option_->put(PREF_DIR, A2_TEST_OUT_DIR);
    dctx_.reset(new DownloadContext(0, 0, A2_TEST_OUT_DIR"/something"));
    requestGroup_.reset(new RequestGroup(option_));
    requestGroup_->setDownloadContext(dctx_);
  }

  void testCanHandle();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( UTMetadataPostDownloadHandlerTest );

void UTMetadataPostDownloadHandlerTest::testCanHandle()
{
  UTMetadataPostDownloadHandler handler;

  CPPUNIT_ASSERT(!handler.canHandle(requestGroup_.get()));

  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  dctx_->setAttribute(bittorrent::BITTORRENT, attrs);

  CPPUNIT_ASSERT(handler.canHandle(requestGroup_.get()));

  // Only checks whether metadata is empty or not
  attrs->metadata = "metadata";

  CPPUNIT_ASSERT(!handler.canHandle(requestGroup_.get()));
}

void UTMetadataPostDownloadHandlerTest::testGetNextRequestGroups()
{
  File trfile(A2_TEST_OUT_DIR"/cd41c7fdddfd034a15a04d7ff881216e01c4ceaf.torrent");
  if(trfile.exists()) {
    trfile.remove();
  }
  std::string metadata =
    "d6:lengthi384e4:name19:aria2-0.8.2.tar.bz212:piece lengthi128e"
    "6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCe";
  unsigned char infoHash[20];
  message_digest::digest
    (infoHash, sizeof(infoHash), MessageDigest::sha1(),
     reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size());
  dctx_->getFirstFileEntry()->setLength(metadata.size());
  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  attrs->infoHash = std::string(&infoHash[0], &infoHash[20]);
  std::vector<std::vector<std::string> > announceList;
  std::vector<std::string> announceTier;
  announceTier.push_back("http://tracker");
  announceList.push_back(announceTier);
  attrs->announceList = announceList;
  dctx_->setAttribute(bittorrent::BITTORRENT, attrs);
  requestGroup_->setDiskWriterFactory
    (SharedHandle<DiskWriterFactory>(new ByteArrayDiskWriterFactory()));
  requestGroup_->initPieceStorage();
  requestGroup_->getPieceStorage()->getDiskAdaptor()->writeData
    (reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size(),
     0);

  UTMetadataPostDownloadHandler handler;
  std::vector<SharedHandle<RequestGroup> > results;
  handler.getNextRequestGroups(results, requestGroup_.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1, results.size());
  SharedHandle<RequestGroup> newRg = results.front();
  SharedHandle<DownloadContext> newDctx = newRg->getDownloadContext();
  SharedHandle<TorrentAttribute> newAttrs =
    bittorrent::getTorrentAttrs(newDctx);
  CPPUNIT_ASSERT_EQUAL(bittorrent::getInfoHashString(dctx_),
                       bittorrent::getInfoHashString(newDctx));
  const std::vector<std::vector<std::string> >& newAnnounceList =
    newAttrs->announceList;
  CPPUNIT_ASSERT_EQUAL((size_t)1, newAnnounceList.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker"), newAnnounceList[0][0]);
  CPPUNIT_ASSERT_EQUAL(option_->get(PREF_DIR),
                       newRg->getOption()->get(PREF_DIR));
  CPPUNIT_ASSERT
    (std::find(requestGroup_->followedBy().begin(),
               requestGroup_->followedBy().end(),
               newRg->getGID()) != requestGroup_->followedBy().end());
  CPPUNIT_ASSERT(!trfile.exists());

  results.clear();

  requestGroup_->getOption()->put(PREF_BT_SAVE_METADATA, A2_V_TRUE);
  handler.getNextRequestGroups(results, requestGroup_.get());
  CPPUNIT_ASSERT(trfile.exists());

  results.clear();

  // See failure with bad metadata
  metadata = "d6:lengthi384e4:name19:aria2-0.8.2.tar.bz212:piece lengthi128e";
  requestGroup_->initPieceStorage();
  requestGroup_->getPieceStorage()->getDiskAdaptor()->writeData
    (reinterpret_cast<const unsigned char*>(metadata.data()), metadata.size(),
     0);
  try {
    handler.getNextRequestGroups(results, requestGroup_.get());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

} // namespace aria2
