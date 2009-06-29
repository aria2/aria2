#include "Metalink2RequestGroup.h"

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroup.h"
#include "FileEntry.h"

namespace aria2 {

class Metalink2RequestGroupTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Metalink2RequestGroupTest);
  CPPUNIT_TEST(testGenerate);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> _option;

public:
  void setUp()
  {
    _option.reset(new Option());
    _option->put(PREF_SPLIT, "1");
  }

  void testGenerate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Metalink2RequestGroupTest );

void Metalink2RequestGroupTest::testGenerate()
{
  std::deque<SharedHandle<RequestGroup> > groups;
  _option->put(PREF_DIR, "/tmp");
  Metalink2RequestGroup().generate(groups, "test.xml", _option);
  // first file
  {
    SharedHandle<RequestGroup> rg = groups[0];
    std::deque<std::string> uris;
    rg->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    std::sort(uris.begin(), uris.end());
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL
      (std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"), uris[0]);
    CPPUNIT_ASSERT_EQUAL
      (std::string("http://httphost/aria2-0.5.2.tar.bz2"), uris[1]);

    const SharedHandle<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL((uint64_t)0ULL, dctx->getTotalLength());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), dctx->getDir());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"), dctx->getChecksumHashAlgo());
    CPPUNIT_ASSERT_EQUAL
      (std::string("a96cf3f0266b91d87d5124cf94326422800b627d"),
			 dctx->getChecksum());
#endif // ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT(!dctx->getSignature().isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("pgp"), dctx->getSignature()->getType());
  }
  // second file
  {
    SharedHandle<RequestGroup> rg = groups[1];
    std::deque<std::string> uris;
    rg->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());

    const SharedHandle<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), dctx->getDir());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT_EQUAL(std::string("sha1"), dctx->getPieceHashAlgo());
    CPPUNIT_ASSERT_EQUAL((size_t)2, dctx->getPieceHashes().size());
    CPPUNIT_ASSERT_EQUAL((size_t)262144, dctx->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(std::string(""), dctx->getChecksumHashAlgo());
    CPPUNIT_ASSERT_EQUAL(std::string(""), dctx->getChecksum());
#endif // ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT(dctx->getSignature().isNull());
  }

#ifdef ENABLE_BITTORRENT
  // fifth file <- downloading .torrent file
  {
    SharedHandle<RequestGroup> rg = groups[4];
    std::deque<std::string> uris;
    rg->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL
      (std::string("http://host/torrent-http.integrated.torrent"), uris[0]);
    const SharedHandle<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(!dctx.isNull());
    // PREF_DIR has no effect for internal torrent file download
    CPPUNIT_ASSERT_EQUAL(std::string("."), dctx->getDir());
  }
#endif // ENABLE_BITTORRENT

  // sixth file <- depends on thrid file
  {
#ifdef ENABLE_BITTORRENT
    SharedHandle<RequestGroup> rg = groups[5];
#else
    SharedHandle<RequestGroup> rg = groups[4];
#endif // ENABLE_BITTORRENT
    std::deque<std::string> uris;
    rg->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL
      (std::string("http://host/torrent-http.integrated"), uris[0]);

    const SharedHandle<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), dctx->getDir());
  }
}

} // namespace aria2
