#include "Metalink2RequestGroup.h"

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadContext.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroup.h"
#include "FileEntry.h"
#include "Signature.h"

namespace aria2 {

class Metalink2RequestGroupTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Metalink2RequestGroupTest);
  CPPUNIT_TEST(testGenerate);
  CPPUNIT_TEST(testGenerate_with_local_metaurl);
  CPPUNIT_TEST(testGenerate_groupByMetaurl);
  CPPUNIT_TEST(testGenerate_dosDirTraversal);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<Option> option_;

public:
  void setUp()
  {
    option_.reset(new Option());
    option_->put(PREF_SPLIT, "1");
  }

  void testGenerate();
  void testGenerate_with_local_metaurl();
  void testGenerate_groupByMetaurl();
  void testGenerate_dosDirTraversal();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Metalink2RequestGroupTest);

void Metalink2RequestGroupTest::testGenerate()
{
  std::vector<std::shared_ptr<RequestGroup>> groups;
  option_->put(PREF_DIR, "/tmp");
  Metalink2RequestGroup().generate(groups, A2_TEST_DIR "/test.xml", option_);
  // first file
  {
    std::shared_ptr<RequestGroup> rg = groups[0];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    std::sort(uris.begin(), uris.end());
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
                         uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
                         uris[1]);

    const std::shared_ptr<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(dctx);
    CPPUNIT_ASSERT_EQUAL((int64_t)0LL, dctx->getTotalLength());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), dctx->getHashType());
    CPPUNIT_ASSERT_EQUAL(
        std::string("a96cf3f0266b91d87d5124cf94326422800b627d"),
        util::toHex(dctx->getDigest()));
    CPPUNIT_ASSERT(dctx->getSignature());
    CPPUNIT_ASSERT_EQUAL(std::string("pgp"), dctx->getSignature()->getType());
  }
  // second file
  {
    std::shared_ptr<RequestGroup> rg = groups[1];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());

    const std::shared_ptr<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(dctx);
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), dctx->getPieceHashType());
    CPPUNIT_ASSERT_EQUAL((size_t)2, dctx->getPieceHashes().size());
    CPPUNIT_ASSERT_EQUAL(262144, dctx->getPieceLength());
    CPPUNIT_ASSERT_EQUAL(std::string("sha-1"), dctx->getHashType());
    CPPUNIT_ASSERT_EQUAL(
        std::string("4c255b0ed130f5ea880f0aa061c3da0487e251cc"),
        util::toHex(dctx->getDigest()));
    CPPUNIT_ASSERT(!dctx->getSignature());
  }

#ifdef ENABLE_BITTORRENT
  // fifth file <- downloading .torrent file
  {
    std::shared_ptr<RequestGroup> rg = groups[4];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("http://host/torrent-http.integrated.torrent"), uris[0]);
    const std::shared_ptr<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(dctx);
    CPPUNIT_ASSERT_EQUAL(groups[5]->getGID(), rg->belongsTo());
  }
#endif // ENABLE_BITTORRENT

  // sixth file <- depends on fifth file to download .torrent file.
  {
#ifdef ENABLE_BITTORRENT
    std::shared_ptr<RequestGroup> rg = groups[5];
#else
    std::shared_ptr<RequestGroup> rg = groups[4];
#endif // ENABLE_BITTORRENT
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://host/torrent-http.integrated"),
                         uris[0]);

    const std::shared_ptr<DownloadContext>& dctx = rg->getDownloadContext();

    CPPUNIT_ASSERT(dctx);
  }
}

void Metalink2RequestGroupTest::testGenerate_with_local_metaurl()
{
  std::vector<std::shared_ptr<RequestGroup>> groups;
  option_->put(PREF_DIR, "/tmp");
  // local metaurl does not work without --metalink-base-uri option.
  // Make sure that it does not crash with local metaurl.
  Metalink2RequestGroup().generate(groups, A2_TEST_DIR "/local-metaurl.meta4",
                                   option_);
  CPPUNIT_ASSERT_EQUAL((size_t)1, groups.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/README"),
                       groups[0]
                           ->getDownloadContext()
                           ->getFirstFileEntry()
                           ->getRemainingUris()[0]);
}

void Metalink2RequestGroupTest::testGenerate_groupByMetaurl()
{
  std::vector<std::shared_ptr<RequestGroup>> groups;
  Metalink2RequestGroup().generate(
      groups, A2_TEST_DIR "/metalink4-groupbymetaurl.xml", option_);
  CPPUNIT_ASSERT_EQUAL((size_t)3, groups.size());

#ifdef ENABLE_BITTORRENT
  // first RequestGroup is torrent for second RequestGroup
  {
    std::shared_ptr<RequestGroup> rg = groups[0];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://torrent"), uris[0]);
  }
  // second
  {
    std::shared_ptr<RequestGroup> rg = groups[1];
    std::shared_ptr<DownloadContext> dctx = rg->getDownloadContext();
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
        dctx->getFileEntries();
    CPPUNIT_ASSERT_EQUAL((size_t)2, fileEntries.size());
    CPPUNIT_ASSERT_EQUAL(std::string("./file1"), fileEntries[0]->getPath());
    CPPUNIT_ASSERT_EQUAL(std::string("file1"),
                         fileEntries[0]->getOriginalName());
    CPPUNIT_ASSERT_EQUAL(std::string("file1"), fileEntries[0]->getSuffixPath());
    CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries[0]->getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file1p1"),
                         fileEntries[0]->getRemainingUris()[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("./file3"), fileEntries[1]->getPath());
    CPPUNIT_ASSERT_EQUAL(std::string("file3"),
                         fileEntries[1]->getOriginalName());
    CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries[1]->getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file3p1"),
                         fileEntries[1]->getRemainingUris()[0]);
  }
  // third
  {
    std::shared_ptr<RequestGroup> rg = groups[2];
    std::shared_ptr<DownloadContext> dctx = rg->getDownloadContext();
    const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
        dctx->getFileEntries();
    CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries.size());
    CPPUNIT_ASSERT_EQUAL(std::string("./file2"), fileEntries[0]->getPath());
    CPPUNIT_ASSERT_EQUAL((size_t)1, fileEntries[0]->getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file2p1"),
                         fileEntries[0]->getRemainingUris()[0]);
  }
#else // !ENABLE_BITTORRENT
  {
    std::shared_ptr<RequestGroup> rg = groups[0];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file1p1"), uris[0]);
  }
  {
    std::shared_ptr<RequestGroup> rg = groups[1];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file2p1"), uris[0]);
  }
  {
    std::shared_ptr<RequestGroup> rg = groups[2];
    auto uris = rg->getDownloadContext()->getFirstFileEntry()->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://file3p1"), uris[0]);
  }

#endif // !ENABLE_BITTORRENT
}

void Metalink2RequestGroupTest::testGenerate_dosDirTraversal()
{
#ifdef __MINGW32__
#  ifdef ENABLE_BITTORRENT
  std::vector<std::shared_ptr<RequestGroup>> groups;
  option_->put(PREF_DIR, "/tmp");
  Metalink2RequestGroup().generate(
      groups, A2_TEST_DIR "/metalink4-dosdirtraversal.xml", option_);
  CPPUNIT_ASSERT_EQUAL((size_t)3, groups.size());
  std::shared_ptr<RequestGroup> rg = groups[0];
  std::shared_ptr<FileEntry> file =
      rg->getDownloadContext()->getFirstFileEntry();
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/..%5C..%5Cexample.ext"),
                       file->getPath());

  rg = groups[2];
  file = rg->getDownloadContext()->getFileEntries()[0];
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/..%5C..%5Cfile1.ext"),
                       file->getPath());
  file = rg->getDownloadContext()->getFileEntries()[1];
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/..%5C..%5Cfile2.ext"),
                       file->getPath());
#  endif // ENABLE_BITTORRENT
#endif   // __MINGW32__
}

} // namespace aria2
