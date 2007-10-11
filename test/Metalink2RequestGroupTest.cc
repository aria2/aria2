#include "Metalink2RequestGroup.h"
#include "SingleFileDownloadContext.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroup.h"
#include <cppunit/extensions/HelperMacros.h>

class Metalink2RequestGroupTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(Metalink2RequestGroupTest);
  CPPUNIT_TEST(testGenerate);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<Option> _option;

public:
  void setUp()
  {
    _option = new Option();
    _option->put(PREF_SPLIT, "1");
  }

  void testGenerate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Metalink2RequestGroupTest );

void Metalink2RequestGroupTest::testGenerate()
{
  RequestGroups groups = Metalink2RequestGroup(_option.get()).generate("test.xml");
  // first file
  {
    RequestGroupHandle rg = groups[0];
    Strings uris = rg->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL(string("ftp://ftphost/aria2-0.5.2.tar.bz2"), uris[0]);
    CPPUNIT_ASSERT_EQUAL(string("http://httphost/aria2-0.5.2.tar.bz2"), uris[1]);
    SingleFileDownloadContextHandle dctx = rg->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL((int64_t)0, dctx->getTotalLength());
  }
  // second file
  {
    RequestGroupHandle rg = groups[1];
    Strings uris = rg->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    SingleFileDownloadContextHandle dctx = rg->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
    CPPUNIT_ASSERT_EQUAL(string("sha1"), dctx->getPieceHashAlgo());
    CPPUNIT_ASSERT_EQUAL((size_t)2, dctx->getPieceHashes().size());
    CPPUNIT_ASSERT_EQUAL((int32_t)262144, dctx->getPieceLength());
  }

  // fifth file <- downloading .torrent file
  {
    RequestGroupHandle rg = groups[4];
    Strings uris = rg->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(string("http://host/torrent-http.integrated.torrent"),
			 uris[0]);
    SingleFileDownloadContextHandle dctx = rg->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
  }
  // sixth file <- depends on thrid file
  {
    RequestGroupHandle rg = groups[5];
    Strings uris = rg->getUris();
    CPPUNIT_ASSERT_EQUAL((size_t)1, uris.size());
    CPPUNIT_ASSERT_EQUAL(string("http://host/torrent-http.integrated"), uris[0]);
    SingleFileDownloadContextHandle dctx = rg->getDownloadContext();
    CPPUNIT_ASSERT(!dctx.isNull());
  }
}
