#include "BtPostDownloadHandler.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "FileEntry.h"
#include "bittorrent_helper.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "RequestGroupCriteria.h"

namespace aria2 {

class BtPostDownloadHandlerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle_extension);
  CPPUNIT_TEST(testCanHandle_contentType);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<Option> option_;

public:
  void setUp() { option_.reset(new Option()); }

  void testCanHandle_extension();
  void testCanHandle_contentType();
  void testGetNextRequestGroups();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtPostDownloadHandlerTest);

void BtPostDownloadHandlerTest::testCanHandle_extension()
{
  std::shared_ptr<DownloadContext> dctx(
      new DownloadContext(0, 0, A2_TEST_DIR "/test.torrent"));
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->getFirstFileEntry()->setPath(A2_TEST_DIR "/test.torrent2");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testCanHandle_contentType()
{
  std::shared_ptr<DownloadContext> dctx(new DownloadContext(0, 0, "test"));
  dctx->getFirstFileEntry()->setContentType("application/x-bittorrent");
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);

  BtPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void BtPostDownloadHandlerTest::testGetNextRequestGroups()
{
  std::shared_ptr<DownloadContext> dctx(
      new DownloadContext(1_k, 0, A2_TEST_DIR "/test.torrent"));
  RequestGroup rg(GroupId::create(), option_);
  rg.setDownloadContext(dctx);
  rg.initPieceStorage();
  rg.getPieceStorage()->getDiskAdaptor()->enableReadOnly();

  BtPostDownloadHandler handler;
  std::vector<std::shared_ptr<RequestGroup>> groups;
  handler.getNextRequestGroups(groups, &rg);
  CPPUNIT_ASSERT_EQUAL((size_t)1, groups.size());
  CPPUNIT_ASSERT_EQUAL(
      std::string("248d0a1cd08284299de78d5c1ed359bb46717d8c"),
      bittorrent::getInfoHashString(groups.front()->getDownloadContext()));
  CPPUNIT_ASSERT(std::find(rg.followedBy().begin(), rg.followedBy().end(),
                           groups.front()->getGID()) != rg.followedBy().end());
  for (auto& nrg : groups) {
    CPPUNIT_ASSERT_EQUAL(rg.getGID(), nrg->following());
  }
}

} // namespace aria2
