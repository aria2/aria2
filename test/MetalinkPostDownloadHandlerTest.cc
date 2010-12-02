#include "MetalinkPostDownloadHandler.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "Option.h"
#include "DownloadContext.h"
#include "FileEntry.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"

namespace aria2 {

class MetalinkPostDownloadHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetalinkPostDownloadHandlerTest);
  CPPUNIT_TEST(testCanHandle_extension);
  CPPUNIT_TEST(testCanHandle_contentType);
  CPPUNIT_TEST(testGetNextRequestGroups);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    option_.reset(new Option());
  }

  void testCanHandle_extension();
  void testCanHandle_contentType();
  void testGetNextRequestGroups();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetalinkPostDownloadHandlerTest );

void MetalinkPostDownloadHandlerTest::testCanHandle_extension()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(0, 0, "test.metalink"));
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  MetalinkPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->getFirstFileEntry()->setPath("test.metalink2");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void MetalinkPostDownloadHandlerTest::testCanHandle_contentType()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0, "test"));
  dctx->getFirstFileEntry()->setContentType("application/metalink+xml");
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);

  MetalinkPostDownloadHandler handler;

  CPPUNIT_ASSERT(handler.canHandle(&rg));

  dctx->getFirstFileEntry()->setContentType("application/octet-stream");
  CPPUNIT_ASSERT(!handler.canHandle(&rg));
}

void MetalinkPostDownloadHandlerTest::testGetNextRequestGroups()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(1024, 0, A2_TEST_DIR"/test.xml"));
  RequestGroup rg(option_);
  rg.setDownloadContext(dctx);
  rg.initPieceStorage();
  rg.getPieceStorage()->getDiskAdaptor()->enableReadOnly();

  MetalinkPostDownloadHandler handler;
  std::vector<SharedHandle<RequestGroup> > groups;
  handler.getNextRequestGroups(groups, &rg);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL((size_t)6/* 5 + 1 torrent file download */, groups.size());
#else
  CPPUNIT_ASSERT_EQUAL((size_t)5, groups.size());
#endif // ENABLE_BITTORRENT
}

} // namespace aria2
