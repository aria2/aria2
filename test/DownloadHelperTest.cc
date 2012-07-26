#include "download_helper.h"

#include <iostream>
#include <string>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Option.h"
#include "array_fun.h"
#include "prefs.h"
#include "Exception.h"
#include "util.h"
#include "FileEntry.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

class DownloadHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadHelperTest);
  CPPUNIT_TEST(testCreateRequestGroupForUri);
  CPPUNIT_TEST(testCreateRequestGroupForUri_parameterized);
  CPPUNIT_TEST(testCreateRequestGroupForUriList);

#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testCreateRequestGroupForUri_BitTorrent);
  CPPUNIT_TEST(testCreateRequestGroupForBitTorrent);
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
  CPPUNIT_TEST(testCreateRequestGroupForUri_Metalink);
  CPPUNIT_TEST(testCreateRequestGroupForMetalink);
#endif // ENABLE_METALINK

  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    option_.reset(new Option());
  }

  void tearDown() {}

  void testCreateRequestGroupForUri();
  void testCreateRequestGroupForUri_parameterized();
  void testCreateRequestGroupForUriList();

#ifdef ENABLE_BITTORRENT
  void testCreateRequestGroupForUri_BitTorrent();
  void testCreateRequestGroupForBitTorrent();
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
  void testCreateRequestGroupForUri_Metalink();
  void testCreateRequestGroupForMetalink();
#endif // ENABLE_METALINK
};


CPPUNIT_TEST_SUITE_REGISTRATION(DownloadHelperTest);

void DownloadHelperTest::testCreateRequestGroupForUri()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file"
  };
  std::vector<std::string> uris(vbegin(array), vend(array));
  option_->put(PREF_SPLIT, "7");
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "2");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  {
    std::vector<SharedHandle<RequestGroup> > result;
    createRequestGroupForUri(result, option_, uris);
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> xuris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(xuris);
    CPPUNIT_ASSERT_EQUAL((size_t)6, xuris.size());
    for(size_t i = 0; i < 6; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i%3], xuris[i]);
    }
    CPPUNIT_ASSERT_EQUAL(7, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"), ctx->getBasePath());
  }
  option_->put(PREF_SPLIT, "5");
  {
    std::vector<SharedHandle<RequestGroup> > result;
    createRequestGroupForUri(result, option_, uris);
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> xuris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(xuris);
    CPPUNIT_ASSERT_EQUAL((size_t)5, xuris.size());
    for(size_t i = 0; i < 5; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i%3], xuris[i]);
    }
  }
  option_->put(PREF_SPLIT, "2");
  {
    std::vector<SharedHandle<RequestGroup> > result;
    createRequestGroupForUri(result, option_, uris);
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> xuris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(xuris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, xuris.size());
    for(size_t i = 0; i < 3; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i%3], xuris[i]);
    }
  }
  option_->put(PREF_FORCE_SEQUENTIAL, A2_V_TRUE);
  {
    std::vector<SharedHandle<RequestGroup> > result;
    createRequestGroupForUri(result, option_, uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());
    // for alpha server
    SharedHandle<RequestGroup> alphaGroup = result[0];
    std::vector<std::string> alphaURIs;
    alphaGroup->getDownloadContext()->getFirstFileEntry()->getUris(alphaURIs);
    CPPUNIT_ASSERT_EQUAL((size_t)2, alphaURIs.size());
    for(size_t i = 0; i < 2; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[0], alphaURIs[i]);
    }
    CPPUNIT_ASSERT_EQUAL(2, alphaGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> alphaCtx = alphaGroup->getDownloadContext();
    // See filename is not assigned yet
    CPPUNIT_ASSERT_EQUAL(std::string(""), alphaCtx->getBasePath());
  }
}

void DownloadHelperTest::testCreateRequestGroupForUri_parameterized()
{
  std::string array[] = {
    "http://{alpha, bravo}/file",
    "http://charlie/file"
  };
  std::vector<std::string> uris(vbegin(array), vend(array));
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "1");
  option_->put(PREF_SPLIT, "3");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  option_->put(PREF_PARAMETERIZED_URI, A2_V_TRUE);
  {
    std::vector<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, option_, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> uris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());

    CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), uris[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), uris[2]);

    CPPUNIT_ASSERT_EQUAL(3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"), ctx->getBasePath());
  }
}

#ifdef ENABLE_BITTORRENT
void DownloadHelperTest::testCreateRequestGroupForUri_BitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    A2_TEST_DIR"/test.torrent",
    "http://bravo/file",
    "http://charlie/file"
  };
  std::vector<std::string> uris(vbegin(array), vend(array));
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "1");
  option_->put(PREF_SPLIT, "3");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  {
    std::vector<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, option_, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> xuris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(xuris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, xuris.size());

    CPPUNIT_ASSERT_EQUAL(array[0], xuris[0]);
    CPPUNIT_ASSERT_EQUAL(array[2], xuris[1]);
    CPPUNIT_ASSERT_EQUAL(array[3], xuris[2]);

    CPPUNIT_ASSERT_EQUAL(3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
                         ctx->getBasePath());

    SharedHandle<RequestGroup> torrentGroup = result[1];
    std::vector<std::string> auxURIs;
    torrentGroup->getDownloadContext()->getFirstFileEntry()->getUris(auxURIs);
    CPPUNIT_ASSERT(auxURIs.empty());
    CPPUNIT_ASSERT_EQUAL(3, torrentGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> btctx = torrentGroup->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-test"),
                         btctx->getBasePath());    
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void DownloadHelperTest::testCreateRequestGroupForUri_Metalink()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file",
    A2_TEST_DIR"/test.xml"
  };
  std::vector<std::string> uris(vbegin(array), vend(array));
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "1");
  option_->put(PREF_SPLIT, "2");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  {
    std::vector<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, option_, uris);
    
    // group1: http://alpha/file, ...
    // group2-7: 6 file entry in Metalink and 1 torrent file download
#ifdef ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)7, result.size());
#else // !ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)6, result.size());
#endif // !ENABLE_BITTORRENT

    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> xuris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(xuris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, xuris.size());
    for(size_t i = 0; i < 3; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], xuris[i]);
    }
    CPPUNIT_ASSERT_EQUAL(2, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
                         ctx->getBasePath());

    SharedHandle<RequestGroup> aria2052Group = result[1];
    CPPUNIT_ASSERT_EQUAL(1, // because of maxconnections attribute
                         aria2052Group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> aria2052Ctx =
      aria2052Group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-0.5.2.tar.bz2"),
                         aria2052Ctx->getBasePath());
    
    SharedHandle<RequestGroup> aria2051Group = result[2];
    CPPUNIT_ASSERT_EQUAL(2, aria2051Group->getNumConcurrentCommand());
  }
}
#endif // ENABLE_METALINK

void DownloadHelperTest::testCreateRequestGroupForUriList()
{
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "3");
  option_->put(PREF_SPLIT, "3");
  option_->put(PREF_INPUT_FILE, A2_TEST_DIR"/input_uris.txt");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");

  std::vector<SharedHandle<RequestGroup> > result;
  
  createRequestGroupForUriList(result, option_);

  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());

  SharedHandle<RequestGroup> fileGroup = result[0];
  std::vector<std::string> fileURIs;
  fileGroup->getDownloadContext()->getFirstFileEntry()->getUris(fileURIs);
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), fileURIs[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), fileURIs[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), fileURIs[2]);
  CPPUNIT_ASSERT_EQUAL(3, fileGroup->getNumConcurrentCommand());
  SharedHandle<DownloadContext> fileCtx = fileGroup->getDownloadContext();
  CPPUNIT_ASSERT_EQUAL(std::string("/mydownloads/myfile.out"),
                       fileCtx->getBasePath());

  SharedHandle<RequestGroup> fileISOGroup = result[1];
  SharedHandle<DownloadContext> fileISOCtx = fileISOGroup->getDownloadContext();
  // PREF_OUT in option_ must be ignored.
  CPPUNIT_ASSERT_EQUAL(std::string(), fileISOCtx->getBasePath());
}

#ifdef ENABLE_BITTORRENT
void DownloadHelperTest::testCreateRequestGroupForBitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file"
  };

  std::vector<std::string> auxURIs(vbegin(array), vend(array));
  option_->put(PREF_MAX_CONNECTION_PER_SERVER, "2");
  option_->put(PREF_SPLIT, "5");
  option_->put(PREF_TORRENT_FILE, A2_TEST_DIR"/test.torrent");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  option_->put(PREF_BT_EXCLUDE_TRACKER, "http://tracker1");
  {
    std::vector<SharedHandle<RequestGroup> > result;

    createRequestGroupForBitTorrent(result, option_, auxURIs,
                                    option_->get(PREF_TORRENT_FILE));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());

    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> uris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    // See -s option is ignored. See processRootDictionary() in
    // bittorrent_helper.cc
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < A2_ARRAY_LEN(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i]+"/aria2-test/aria2/src/aria2c", uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL(5, group->getNumConcurrentCommand());
    SharedHandle<TorrentAttribute> attrs =
      bittorrent::getTorrentAttrs(group->getDownloadContext());
    // http://tracker1 was deleted.
    CPPUNIT_ASSERT_EQUAL((size_t)2, attrs->announceList.size());
  }
  {
    // no URIs are given
    std::vector<SharedHandle<RequestGroup> > result;
    std::vector<std::string> emptyURIs;
    createRequestGroupForBitTorrent(result, option_, emptyURIs,
                                    option_->get(PREF_TORRENT_FILE));

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> uris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)0, uris.size());
  }
  option_->put(PREF_FORCE_SEQUENTIAL, A2_V_TRUE);
  {
    std::vector<SharedHandle<RequestGroup> > result;

    createRequestGroupForBitTorrent(result, option_, auxURIs,
                                    option_->get(PREF_TORRENT_FILE));

    // See --force-requencial is ignored
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void DownloadHelperTest::testCreateRequestGroupForMetalink()
{
  option_->put(PREF_SPLIT, "5");
  option_->put(PREF_METALINK_FILE, A2_TEST_DIR"/test.xml");
  option_->put(PREF_DIR, "/tmp");
  option_->put(PREF_OUT, "file.out");
  {
    std::vector<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForMetalink(result, option_);

#ifdef ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)6, result.size());
#else // !ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
#endif // !ENABLE_BITTORRENT
    SharedHandle<RequestGroup> group = result[0];
    std::vector<std::string> uris;
    group->getDownloadContext()->getFirstFileEntry()->getUris(uris);
    std::sort(uris.begin(), uris.end());
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
                         uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
                         uris[1]);
    // See numConcurrentCommand is 1 because of maxconnections attribute.
    CPPUNIT_ASSERT_EQUAL(1, group->getNumConcurrentCommand());
  }
}
#endif // ENABLE_METALINK

} // namespace aria2
