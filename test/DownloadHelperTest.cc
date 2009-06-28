#include "download_helper.h"

#include <iostream>
#include <string>
#include <deque>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Option.h"
#include "array_fun.h"
#include "prefs.h"
#include "Exception.h"
#include "Util.h"

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
  SharedHandle<Option> _option;
public:
  void setUp()
  {
    _option.reset(new Option());
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
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"), ctx->getBasePath());
  }
  _option->put(PREF_SPLIT, "5");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)5, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    for(size_t i = 0; i < 5-arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i+arrayLength(array)]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)5, group->getNumConcurrentCommand());
  }
  _option->put(PREF_SPLIT, "2");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)2, group->getNumConcurrentCommand());
  }
  _option->put(PREF_FORCE_SEQUENTIAL, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());

    // for alpha server
    SharedHandle<RequestGroup> alphaGroup = result[0];
    std::deque<std::string> alphaURIs;
    alphaGroup->getURIs(alphaURIs);
    CPPUNIT_ASSERT_EQUAL((size_t)2, alphaURIs.size());
    for(size_t i = 0; i < 2; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[0], uris[0]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)2,
			 alphaGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> alphaCtx = alphaGroup->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), alphaCtx->getDir());
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
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  _option->put(PREF_PARAMETERIZED_URI, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());

    CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), uris[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), uris[2]);

    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"), ctx->getBasePath());
  }
}

#ifdef ENABLE_BITTORRENT
void DownloadHelperTest::testCreateRequestGroupForUri_BitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    "test.torrent",
    "http://bravo/file",
    "http://charlie/file"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());

    CPPUNIT_ASSERT_EQUAL(array[0], uris[0]);
    CPPUNIT_ASSERT_EQUAL(array[2], uris[1]);
    CPPUNIT_ASSERT_EQUAL(array[3], uris[2]);

    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getBasePath());

    SharedHandle<RequestGroup> torrentGroup = result[1];
    std::deque<std::string> auxURIs;
    torrentGroup->getURIs(auxURIs);
    CPPUNIT_ASSERT(auxURIs.empty());
    CPPUNIT_ASSERT_EQUAL((unsigned int)3,
			 torrentGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> btctx = torrentGroup->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), btctx->getDir());
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
    "test.xml"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_METALINK_SERVERS, "2");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, _option, uris);
    
    // group1: http://alpha/file, ...
    // group2-7: 6 file entry in Metalink and 1 torrent file download
#ifdef ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)7, result.size());
#else // !ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)6, result.size());
#endif // !ENABLE_BITTORRENT

    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < 3; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getBasePath());

    SharedHandle<RequestGroup> aria2052Group = result[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, // because of maxconnections attribute
			 aria2052Group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> aria2052Ctx =
      aria2052Group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), aria2052Ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-0.5.2.tar.bz2"),
			 aria2052Ctx->getBasePath());
    
    SharedHandle<RequestGroup> aria2051Group = result[2];
    CPPUNIT_ASSERT_EQUAL((unsigned int)2,
			 aria2051Group->getNumConcurrentCommand());
  }
}
#endif // ENABLE_METALINK

void DownloadHelperTest::testCreateRequestGroupForUriList()
{
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_INPUT_FILE, "input_uris.txt");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");

  std::deque<SharedHandle<RequestGroup> > result;
  
  createRequestGroupForUriList(result, _option);

  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());

  SharedHandle<RequestGroup> fileGroup = result[0];
  std::deque<std::string> fileURIs;
  fileGroup->getURIs(fileURIs);
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), fileURIs[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), fileURIs[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), fileURIs[2]);
  CPPUNIT_ASSERT_EQUAL((unsigned int)3, fileGroup->getNumConcurrentCommand());
  SharedHandle<DownloadContext> fileCtx = fileGroup->getDownloadContext();
  CPPUNIT_ASSERT_EQUAL(std::string("/mydownloads"), fileCtx->getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("/mydownloads/myfile.out"),
		       fileCtx->getBasePath());

  SharedHandle<RequestGroup> fileISOGroup = result[1];
  SharedHandle<DownloadContext> fileISOCtx = fileISOGroup->getDownloadContext();
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), fileISOCtx->getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
		       fileISOCtx->getBasePath());
}

#ifdef ENABLE_BITTORRENT
void DownloadHelperTest::testCreateRequestGroupForBitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file"
  };

  std::deque<std::string> auxURIs(&array[0], &array[arrayLength(array)]);
  _option->put(PREF_SPLIT, "5");
  _option->put(PREF_TORRENT_FILE, "test.torrent");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForBitTorrent(result, _option, auxURIs);

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());

    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)5, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    for(size_t i = 0; i < 5-arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i+arrayLength(array)]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)5, group->getNumConcurrentCommand());
  }
  {
    // no URIs are given
    std::deque<SharedHandle<RequestGroup> > result;
    std::deque<std::string> emptyURIs;
    createRequestGroupForBitTorrent(result, _option, emptyURIs);

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)0, uris.size());
  }
  _option->put(PREF_FORCE_SEQUENTIAL, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForBitTorrent(result, _option, auxURIs);

    // See --force-requencial is ignored
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  }
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void DownloadHelperTest::testCreateRequestGroupForMetalink()
{
  _option->put(PREF_SPLIT, "3");
  _option->put(PREF_METALINK_FILE, "test.xml");
  _option->put(PREF_METALINK_SERVERS, "5");
  _option->put(PREF_DIR, "/tmp");
  _option->put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForMetalink(result, _option);

#ifdef ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)6, result.size());
#else // !ENABLE_BITTORRENT
    CPPUNIT_ASSERT_EQUAL((size_t)5, result.size());
#endif // !ENABLE_BITTORRENT
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    std::sort(uris.begin(), uris.end());
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
			 uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
			 uris[1]);
    // See numConcurrentCommand is 1 because of maxconnections attribute.
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, group->getNumConcurrentCommand());
  }
}
#endif // ENABLE_METALINK

} // namespace aria2
