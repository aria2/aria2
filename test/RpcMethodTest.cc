#include "RpcMethod.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadEngine.h"
#include "SelectEventPoll.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "RpcMethodImpl.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "RpcRequest.h"
#include "RpcResponse.h"
#include "prefs.h"
#include "TestUtil.h"
#include "DownloadContext.h"
#include "FeatureConfig.h"
#include "util.h"
#include "array_fun.h"
#include "download_helper.h"
#include "FileEntry.h"
#ifdef ENABLE_BITTORRENT
# include "BtRegistry.h"
# include "BtRuntime.h"
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace rpc {

class RpcMethodTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RpcMethodTest);
  CPPUNIT_TEST(testAddUri);
  CPPUNIT_TEST(testAddUri_withoutUri);
  CPPUNIT_TEST(testAddUri_notUri);
  CPPUNIT_TEST(testAddUri_withBadOption);
  CPPUNIT_TEST(testAddUri_withPosition);
  CPPUNIT_TEST(testAddUri_withBadPosition);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testAddTorrent);
  CPPUNIT_TEST(testAddTorrent_withoutTorrent);
  CPPUNIT_TEST(testAddTorrent_notBase64Torrent);
  CPPUNIT_TEST(testAddTorrent_withPosition);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  CPPUNIT_TEST(testAddMetalink);
  CPPUNIT_TEST(testAddMetalink_withoutMetalink);
  CPPUNIT_TEST(testAddMetalink_notBase64Metalink);
  CPPUNIT_TEST(testAddMetalink_withPosition);
#endif // ENABLE_METALINK
  CPPUNIT_TEST(testChangeOption);
  CPPUNIT_TEST(testChangeOption_withBadOption);
  CPPUNIT_TEST(testChangeOption_withNotAllowedOption);
  CPPUNIT_TEST(testChangeOption_withoutGid);
  CPPUNIT_TEST(testChangeGlobalOption);
  CPPUNIT_TEST(testChangeGlobalOption_withBadOption);
  CPPUNIT_TEST(testChangeGlobalOption_withNotAllowedOption);
  CPPUNIT_TEST(testTellStatus_withoutGid);
  CPPUNIT_TEST(testTellWaiting);
  CPPUNIT_TEST(testTellWaiting_fail);
  CPPUNIT_TEST(testGetVersion);
  CPPUNIT_TEST(testNoSuchMethod);
  CPPUNIT_TEST(testGatherStoppedDownload);
  CPPUNIT_TEST(testGatherProgressCommon);
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testGatherBitTorrentMetadata);
#endif // ENABLE_BITTORRENT
  CPPUNIT_TEST(testChangePosition);
  CPPUNIT_TEST(testChangePosition_fail);
  CPPUNIT_TEST(testGetSessionInfo);
  CPPUNIT_TEST(testChangeUri);
  CPPUNIT_TEST(testChangeUri_fail);
  CPPUNIT_TEST(testPause);
  CPPUNIT_TEST(testSystemMulticall);
  CPPUNIT_TEST(testSystemMulticall_fail);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<DownloadEngine> e_;
  SharedHandle<Option> option_;
public:
  void setUp()
  {
    RequestGroup::resetGIDCounter();
    option_.reset(new Option());
    option_->put(PREF_DIR, A2_TEST_OUT_DIR"/aria2_RpcMethodTest");
    option_->put(PREF_PIECE_LENGTH, "1048576");
    File(option_->get(PREF_DIR)).mkdirs();
    e_.reset
      (new DownloadEngine(SharedHandle<EventPoll>(new SelectEventPoll())));
    e_->setOption(option_.get());
    e_->setRequestGroupMan
      (SharedHandle<RequestGroupMan>
       (new RequestGroupMan(std::vector<SharedHandle<RequestGroup> >(),
                            1, option_.get())));
  }

  void testAddUri();
  void testAddUri_withoutUri();
  void testAddUri_notUri();
  void testAddUri_withBadOption();
  void testAddUri_withPosition();
  void testAddUri_withBadPosition();
#ifdef ENABLE_BITTORRENT
  void testAddTorrent();
  void testAddTorrent_withoutTorrent();
  void testAddTorrent_notBase64Torrent();
  void testAddTorrent_withPosition();
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  void testAddMetalink();
  void testAddMetalink_withoutMetalink();
  void testAddMetalink_notBase64Metalink();
  void testAddMetalink_withPosition();
#endif // ENABLE_METALINK
  void testChangeOption();
  void testChangeOption_withBadOption();
  void testChangeOption_withNotAllowedOption();
  void testChangeOption_withoutGid();
  void testChangeGlobalOption();
  void testChangeGlobalOption_withBadOption();
  void testChangeGlobalOption_withNotAllowedOption();
  void testTellStatus_withoutGid();
  void testTellWaiting();
  void testTellWaiting_fail();
  void testGetVersion();
  void testNoSuchMethod();
  void testGatherStoppedDownload();
  void testGatherProgressCommon();
#ifdef ENABLE_BITTORRENT
  void testGatherBitTorrentMetadata();
#endif // ENABLE_BITTORRENT
  void testChangePosition();
  void testChangePosition_fail();
  void testGetSessionInfo();
  void testChangeUri();
  void testChangeUri_fail();
  void testPause();
  void testSystemMulticall();
  void testSystemMulticall_fail();
};


CPPUNIT_TEST_SUITE_REGISTRATION(RpcMethodTest);

namespace {
std::string getString(const Dict* dict, const std::string& key)
{
  return downcast<String>(dict->get(key))->s();
}
} // namespace

void RpcMethodTest::testAddUri()
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost/");
  req.params->append(urisParam);
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const std::deque<SharedHandle<RequestGroup> > rgs =
      e_->getRequestGroupMan()->getReservedGroups();
    CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
                         rgs.front()->getDownloadContext()->
                         getFirstFileEntry()->getRemainingUris().front());
  }
  // with options
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_DIR->k, "/sink");
  req.params->append(opt);
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink"),
                         e_->getRequestGroupMan()->findReservedGroup(2)->
                         getOption()->get(PREF_DIR));
  }
}

void RpcMethodTest::testAddUri_withoutUri()
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_notUri()
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("not uri");
  req.params->append(urisParam);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_withBadOption()
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost");
  req.params->append(urisParam);
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_FILE_ALLOCATION->k, "badvalue");
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_withPosition()
{
  AddUriRpcMethod m;
  RpcRequest req1(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam1 = List::g();
  urisParam1->append("http://uri1");
  req1.params->append(urisParam1);
  RpcResponse res1 = m.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);
  
  RpcRequest req2(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam2 = List::g();
  urisParam2->append("http://uri2");
  req2.params->append(urisParam2);
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  m.execute(req2, e_.get());

  std::string uri =
    e_->getRequestGroupMan()->getReservedGroups()[0]->
    getDownloadContext()->getFirstFileEntry()->getRemainingUris()[0];

  CPPUNIT_ASSERT_EQUAL(std::string("http://uri2"), uri);
}

void RpcMethodTest::testAddUri_withBadPosition()
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost/");
  req.params->append(urisParam);
  req.params->append(Dict::g());
  req.params->append(Integer::g(-1));
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

#ifdef ENABLE_BITTORRENT
void RpcMethodTest::testAddTorrent()
{
  File(e_->getOption()->get(PREF_DIR)+
       "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").remove();
  AddTorrentRpcMethod m;
  RpcRequest req(AddTorrentRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(A2_TEST_DIR"/single.torrent"));
  SharedHandle<List> uris = List::g();
  uris->append("http://localhost/aria2-0.8.2.tar.bz2");
  req.params->append(uris);
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT
      (File(e_->getOption()->get(PREF_DIR)+
            "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").exists());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), downcast<String>(res.param)->s());

    SharedHandle<RequestGroup> group =
      e_->getRequestGroupMan()->findReservedGroup(1);
    CPPUNIT_ASSERT(group);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR)+"/aria2-0.8.2.tar.bz2",
                         group->getFirstFilePath());
    CPPUNIT_ASSERT_EQUAL((size_t)1,
                         group->getDownloadContext()->getFirstFileEntry()->
                         getRemainingUris().size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/aria2-0.8.2.tar.bz2"),
                         group->getDownloadContext()->getFirstFileEntry()->
                         getRemainingUris()[0]);
  }
  // with options
  std::string dir = A2_TEST_OUT_DIR"/aria2_RpcMethodTest_testAddTorrent";
  File(dir).mkdirs();
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_DIR->k, dir);
  File(dir+"/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").remove();
  req.params->append(opt);
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL
      (dir+"/aria2-0.8.2.tar.bz2",
       e_->getRequestGroupMan()->findReservedGroup(2)->getFirstFilePath());
    CPPUNIT_ASSERT
      (File(dir+"/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").exists());
  }
}

void RpcMethodTest::testAddTorrent_withoutTorrent()
{
  AddTorrentRpcMethod m;
  RpcRequest req(AddTorrentRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddTorrent_notBase64Torrent()
{
  AddTorrentRpcMethod m;
  RpcRequest req(AddTorrentRpcMethod::getMethodName(), List::g());
  req.params->append("not torrent");
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddTorrent_withPosition()
{
  AddTorrentRpcMethod m;
  RpcRequest req1(AddTorrentRpcMethod::getMethodName(), List::g());
  req1.params->append(readFile(A2_TEST_DIR"/test.torrent"));
  req1.params->append(List::g());
  req1.params->append(Dict::g());
  RpcResponse res1 = m.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  RpcRequest req2(AddTorrentRpcMethod::getMethodName(), List::g());
  req2.params->append(readFile(A2_TEST_DIR"/single.torrent"));
  req2.params->append(List::g());
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  m.execute(req2, e_.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       e_->getRequestGroupMan()->getReservedGroups()[0]->
                       getDownloadContext()->getFileEntries().size());
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void RpcMethodTest::testAddMetalink()
{
  File(e_->getOption()->get(PREF_DIR)+
       "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").remove();
  AddMetalinkRpcMethod m;
  RpcRequest req(AddMetalinkRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(A2_TEST_DIR"/2files.metalink"));
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const List* resParams = downcast<List>(res.param);
    CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
    CPPUNIT_ASSERT_EQUAL(std::string("1"), downcast<String>(resParams->get(0))->s());
    CPPUNIT_ASSERT_EQUAL(std::string("2"), downcast<String>(resParams->get(1))->s());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT
      (File(e_->getOption()->get(PREF_DIR)+
            "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").exists());
#endif // ENABLE_MESSAGE_DIGEST

    SharedHandle<RequestGroup> tar =
      e_->getRequestGroupMan()->findReservedGroup(1);
    CPPUNIT_ASSERT(tar);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR)+"/aria2-5.0.0.tar.bz2",
                         tar->getFirstFilePath());
    SharedHandle<RequestGroup> deb =
      e_->getRequestGroupMan()->findReservedGroup(2);
    CPPUNIT_ASSERT(deb);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR)+"/aria2-5.0.0.deb",
                         deb->getFirstFilePath());
  }
  // with options
  std::string dir = A2_TEST_OUT_DIR"/aria2_RpcMethodTest_testAddMetalink";
  File(dir).mkdirs();
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_DIR->k, dir);
  File(dir+"/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").remove();
  req.params->append(opt);
  {
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(dir+"/aria2-5.0.0.tar.bz2",
                         e_->getRequestGroupMan()->findReservedGroup(3)->
                         getFirstFilePath());
#ifdef ENABLE_MESSAGE_DIGEST
    CPPUNIT_ASSERT
      (File(dir+"/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").exists());
#endif // ENABLE_MESSAGE_DIGEST
  }
}

void RpcMethodTest::testAddMetalink_withoutMetalink()
{
  AddMetalinkRpcMethod m;
  RpcRequest req(AddMetalinkRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddMetalink_notBase64Metalink()
{
  AddMetalinkRpcMethod m;
  RpcRequest req(AddMetalinkRpcMethod::getMethodName(), List::g());
  req.params->append("not metalink");
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddMetalink_withPosition()
{
  AddUriRpcMethod m1;
  RpcRequest req1(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam1 = List::g();
  urisParam1->append("http://uri");
  req1.params->append(urisParam1);
  RpcResponse res1 = m1.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  AddMetalinkRpcMethod m2;
  RpcRequest req2("ari2.addMetalink", List::g());
  req2.params->append(readFile(A2_TEST_DIR"/2files.metalink"));
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  RpcResponse res2 = m2.execute(req2, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res2.code);

  CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR)+"/aria2-5.0.0.tar.bz2",
                       e_->getRequestGroupMan()->getReservedGroups()[0]->
                       getFirstFilePath());
}

#endif // ENABLE_METALINK

void RpcMethodTest::testChangeOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  RpcRequest req(ChangeOptionRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT->k, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_BT_MAX_PEERS->k, "100");
  opt->put(PREF_BT_REQUEST_PEER_SPEED_LIMIT->k, "300K");
  opt->put(PREF_MAX_UPLOAD_LIMIT->k, "50K");

  SharedHandle<BtObject> btObject(new BtObject());
  btObject->btRuntime = SharedHandle<BtRuntime>(new BtRuntime());
  e_->getBtRegistry()->put(group->getGID(), btObject);
#endif // ENABLE_BITTORRENT
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());

  SharedHandle<Option> option = group->getOption();

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(100*1024,
                       group->getMaxDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       option->get(PREF_MAX_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(std::string("307200"),
                       option->get(PREF_BT_REQUEST_PEER_SPEED_LIMIT));

  CPPUNIT_ASSERT_EQUAL(std::string("100"), option->get(PREF_BT_MAX_PEERS));
  CPPUNIT_ASSERT_EQUAL(100, btObject->btRuntime->getMaxPeers());

  CPPUNIT_ASSERT_EQUAL(50*1024,
                       group->getMaxUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       option->get(PREF_MAX_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void RpcMethodTest::testChangeOption_withBadOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  RpcRequest req(ChangeOptionRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT->k, "badvalue");
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeOption_withNotAllowedOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  RpcRequest req(ChangeOptionRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "100K");
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());
  // The unacceptable options are just ignored.
  CPPUNIT_ASSERT_EQUAL(0, res.code);
}

void RpcMethodTest::testChangeOption_withoutGid()
{
  ChangeOptionRpcMethod m;
  RpcRequest req(ChangeOptionRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeGlobalOption()
{
  ChangeGlobalOptionRpcMethod m;
  RpcRequest req
    (ChangeGlobalOptionRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_MAX_OVERALL_UPLOAD_LIMIT->k, "50K");
#endif // ENABLE_BITTORRENT
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL
    (100*1024,
     e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       e_->getOption()->get(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL
    (50*1024,
     e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       e_->getOption()->get(PREF_MAX_OVERALL_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void RpcMethodTest::testChangeGlobalOption_withBadOption()
{
  ChangeGlobalOptionRpcMethod m;
  RpcRequest req
    (ChangeGlobalOptionRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "badvalue");
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeGlobalOption_withNotAllowedOption()
{
  ChangeGlobalOptionRpcMethod m;
  RpcRequest req
    (ChangeGlobalOptionRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_ENABLE_RPC->k, "100K");
  req.params->append(opt);
  RpcResponse res = m.execute(req, e_.get());
  // The unacceptable options are just ignored.
  CPPUNIT_ASSERT_EQUAL(0, res.code);
}

void RpcMethodTest::testNoSuchMethod()
{
  NoSuchMethodRpcMethod m;
  RpcRequest req("make.hamburger", List::g());
  RpcResponse res = m.execute(req, 0);
  CPPUNIT_ASSERT_EQUAL(1, res.code);
  CPPUNIT_ASSERT_EQUAL(std::string("No such method: make.hamburger"),
                       getString(downcast<Dict>(res.param), "faultString"));
}

void RpcMethodTest::testTellStatus_withoutGid()
{
  TellStatusRpcMethod m;
  RpcRequest req(TellStatusRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

namespace {
void addUri(const std::string& uri, const SharedHandle<DownloadEngine>& e)
{
  AddUriRpcMethod m;
  RpcRequest req(AddUriRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append(uri);
  req.params->append(urisParam);
  CPPUNIT_ASSERT_EQUAL(0, m.execute(req, e.get()).code);
}
} // namespace

#ifdef ENABLE_BITTORRENT
namespace {
void addTorrent
(const std::string& torrentFile, const SharedHandle<DownloadEngine>& e)
{
  AddTorrentRpcMethod m;
  RpcRequest req(AddTorrentRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(torrentFile));
  RpcResponse res = m.execute(req, e.get());
}
} // namespace
#endif // ENABLE_BITTORRENT

void RpcMethodTest::testTellWaiting()
{
  addUri("http://1/", e_);
  addUri("http://2/", e_);
  addUri("http://3/", e_);
#ifdef ENABLE_BITTORRENT
  addTorrent(A2_TEST_DIR"/single.torrent", e_);
#else // !ENABLE_BITTORRENT
  addUri("http://4/", e_);
#endif // !ENABLE_BITTORRENT

  TellWaitingRpcMethod m;
  RpcRequest req(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(2));
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       getString(downcast<Dict>(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"),
                       getString(downcast<Dict>(resParams->get(1)), "gid"));
  // waiting.size() == offset+num 
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(3));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // waiting.size() < offset+num 
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(4));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());

  // offset = INT32_MAX
  req.params->set(0, Integer::g(INT32_MAX));
  req.params->set(1, Integer::g(1));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // num = INT32_MAX
  req.params->set(0, Integer::g(1));
  req.params->set(1, Integer::g(INT32_MAX));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // offset=INT32_MAX and num = INT32_MAX
  req.params->set(0, Integer::g(INT32_MAX));
  req.params->set(1, Integer::g(INT32_MAX));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // offset=INT32_MIN and num = INT32_MAX
  req.params->set(0, Integer::g(INT32_MIN));
  req.params->set(1, Integer::g(INT32_MAX));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());

  // negative offset
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(-1));
  req.params->append(Integer::g(2));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("4"),
                       getString(downcast<Dict>(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"),
                       getString(downcast<Dict>(resParams->get(1)), "gid"));
  // negative offset and size < num
  req.params->set(1, Integer::g(100));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)4, resParams->size());
  // nagative offset and normalized offset < 0
  req.params->set(0, Integer::g(-5));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // nagative offset and normalized offset == 0
  req.params->set(0, Integer::g(-4));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)1, resParams->size());
}

void RpcMethodTest::testTellWaiting_fail()
{
  TellWaitingRpcMethod m;
  RpcRequest req(TellWaitingRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testGetVersion()
{
  GetVersionRpcMethod m;
  RpcRequest req(GetVersionRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const Dict* resParams = downcast<Dict>(res.param);
  CPPUNIT_ASSERT_EQUAL(std::string(PACKAGE_VERSION),
                       getString(resParams, "version"));
  const List* featureList = downcast<List>(resParams->get("enabledFeatures"));
  std::string features;
  for(List::ValueType::const_iterator i = featureList->begin();
      i != featureList->end(); ++i) {
    const String* s = downcast<String>(*i);
    features += s->s();
    features += ", ";
  }
  
  CPPUNIT_ASSERT_EQUAL(FeatureConfig::getInstance()->featureSummary()+", ",
                       features);
}

void RpcMethodTest::testGatherStoppedDownload()
{
  std::vector<SharedHandle<FileEntry> > fileEntries;
  std::vector<a2_gid_t> followedBy;
  followedBy.push_back(3);
  followedBy.push_back(4);
  SharedHandle<DownloadResult> d(new DownloadResult());
  d->gid = 1;
  d->fileEntries = fileEntries;
  d->inMemoryDownload = false;
  d->sessionDownloadLength = UINT64_MAX;
  d->sessionTime = 1000;
  d->result = error_code::FINISHED;
  d->followedBy = followedBy;
  d->belongsTo = 2;
  SharedHandle<Dict> entry = Dict::g();
  std::vector<std::string> keys;
  gatherStoppedDownload(entry, d, keys);

  const List* followedByRes = downcast<List>(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"), downcast<String>(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("4"), downcast<String>(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       downcast<String>(entry->get("belongsTo"))->s());

  keys.push_back("gid");

  entry = Dict::g();
  gatherStoppedDownload(entry, d, keys);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
}

void RpcMethodTest::testGatherProgressCommon()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0,"aria2.tar.bz2"));
  std::string uris[] = { "http://localhost/aria2.tar.bz2" };
  dctx->getFirstFileEntry()->addUris(vbegin(uris), vend(uris));
  SharedHandle<RequestGroup> group(new RequestGroup(util::copy(option_)));
  group->setDownloadContext(dctx);
  std::vector<SharedHandle<RequestGroup> > followedBy;
  for(int i = 0; i < 2; ++i) {
    followedBy.push_back(SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))));
  }

  group->followedBy(followedBy.begin(), followedBy.end());
  group->belongsTo(2);

  SharedHandle<Dict> entry = Dict::g();
  std::vector<std::string> keys;
  gatherProgressCommon(entry, group, keys);
  
  const List* followedByRes = downcast<List>(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[0]->getGID()),
                       downcast<String>(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[1]->getGID()),
                       downcast<String>(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       downcast<String>(entry->get("belongsTo"))->s());
  const List* files = downcast<List>(entry->get("files"));
  CPPUNIT_ASSERT_EQUAL((size_t)1, files->size());
  const Dict* file = downcast<Dict>(files->get(0));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       downcast<String>(file->get("path"))->s());
  CPPUNIT_ASSERT_EQUAL(uris[0],
                       downcast<String>
                       (downcast<Dict>
                        (downcast<List>(file->get("uris"))->get(0))
                        ->get("uri"))
                       ->s());
  CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR),
                       downcast<String>(entry->get("dir"))->s());

  keys.push_back("gid");
  entry = Dict::g();
  gatherProgressCommon(entry, group, keys);

  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
  
}

#ifdef ENABLE_BITTORRENT
void RpcMethodTest::testGatherBitTorrentMetadata()
{
  SharedHandle<Option> option(new Option());
  option->put(PREF_DIR, ".");
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  bittorrent::load(A2_TEST_DIR"/test.torrent", dctx, option);
  SharedHandle<Dict> btDict = Dict::g();
  gatherBitTorrentMetadata(btDict, bittorrent::getTorrentAttrs(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
                       downcast<String>(btDict->get("comment"))->s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1123456789,
                       downcast<Integer>(btDict->get("creationDate"))->i());
  CPPUNIT_ASSERT_EQUAL(std::string("multi"),
                       downcast<String>(btDict->get("mode"))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"),
                       downcast<String>
                       (downcast<Dict>
                        (btDict->get("info"))
                        ->get("name"))
                       ->s());
  const List* announceList = downcast<List>(btDict->get("announceList"));
  CPPUNIT_ASSERT_EQUAL((size_t)3, announceList->size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
                       downcast<String>(downcast<List>(announceList->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"),
                       downcast<String>(downcast<List>(announceList->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"),
                       downcast<String>(downcast<List>(announceList->get(2))->get(0))->s());
  // Remove some keys
  SharedHandle<TorrentAttribute> modBtAttrs = bittorrent::getTorrentAttrs(dctx);
  modBtAttrs->comment.clear();
  modBtAttrs->creationDate = 0;
  modBtAttrs->mode.clear();
  modBtAttrs->metadata.clear();
  btDict = Dict::g();
  gatherBitTorrentMetadata(btDict, modBtAttrs);
  CPPUNIT_ASSERT(!btDict->containsKey("comment"));
  CPPUNIT_ASSERT(!btDict->containsKey("creationDate"));
  CPPUNIT_ASSERT(!btDict->containsKey("mode"));
  CPPUNIT_ASSERT(!btDict->containsKey("info"));
  CPPUNIT_ASSERT(btDict->containsKey("announceList"));
}
#endif // ENABLE_BITTORRENT

void RpcMethodTest::testChangePosition()
{
  e_->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))));
  e_->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(util::copy(option_))));

  ChangePositionRpcMethod m;
  RpcRequest req(ChangePositionRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  req.params->append(Integer::g(1));
  req.params->append("POS_SET");
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)1, downcast<Integer>(res.param)->i());
  CPPUNIT_ASSERT_EQUAL
    ((a2_gid_t)1, e_->getRequestGroupMan()->getReservedGroups()[1]->getGID());
}

void RpcMethodTest::testChangePosition_fail()
{
  ChangePositionRpcMethod m;
  RpcRequest req(ChangePositionRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->append("1");
  req.params->append(Integer::g(2));
  req.params->append("bad keyword");
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeUri()
{
  SharedHandle<FileEntry> files[3];
  for(int i = 0; i < 3; ++i) {
    files[i].reset(new FileEntry());
  }
  files[1]->addUri("http://example.org/aria2.tar.bz2");
  files[1]->addUri("http://example.org/mustremove1");
  files[1]->addUri("http://example.org/mustremove2");
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setFileEntries(&files[0], &files[3]);
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  group->setDownloadContext(dctx);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriRpcMethod m;
  RpcRequest req(ChangeUriRpcMethod::getMethodName(), List::g());
  req.params->append("1"); // GID
  req.params->append(Integer::g(2)); // index of FileEntry
  SharedHandle<List> removeuris = List::g();
  removeuris->append("http://example.org/mustremove1");
  removeuris->append("http://example.org/mustremove2");
  removeuris->append("http://example.org/notexist");
  req.params->append(removeuris);
  SharedHandle<List> adduris = List::g();
  adduris->append("http://example.org/added1");
  adduris->append("http://example.org/added2");
  adduris->append("baduri");
  adduris->append("http://example.org/added3");
  req.params->append(adduris);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)3, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[0]->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[2]->getRemainingUris().size());
  std::deque<std::string> uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)4, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/aria2.tar.bz2"),uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added2"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added3"), uris[3]);

  // Change adduris
  adduris = List::g();
  adduris->append("http://example.org/added1-1");
  adduris->append("http://example.org/added1-2");
  req.params->set(3, adduris);
  // Set position parameter
  req.params->append(Integer::g(2));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)6, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[3]);

  // Change index of FileEntry
  req.params->set(1, Integer::g(1));
  // Set position far beyond the size of uris in FileEntry.
  req.params->set(4, Integer::g(1000));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)0, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  uris = files[0]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[1]);
}

void RpcMethodTest::testChangeUri_fail()
{
  SharedHandle<FileEntry> files[3];
  for(int i = 0; i < 3; ++i) {
    files[i].reset(new FileEntry());
  }
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  dctx->setFileEntries(&files[0], &files[3]);
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  group->setDownloadContext(dctx);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriRpcMethod m;
  RpcRequest req(ChangeUriRpcMethod::getMethodName(), List::g());
  req.params->append("1"); // GID
  req.params->append(Integer::g(1)); // index of FileEntry
  SharedHandle<List> removeuris = List::g();
  req.params->append(removeuris);
  SharedHandle<List> adduris = List::g();
  req.params->append(adduris);
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);

  req.params->set(1, Integer::g(0));
  res = m.execute(req, e_.get());
  // RPC request fails because 2nd argument is less than 1.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->set(1, Integer::g(1));
  req.params->set(0, String::g("2"));
  res = m.execute(req, e_.get());  
  // RPC request fails because GID#2 does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->set(0, String::g("1"));
  req.params->set(1, Integer::g(4));
  res = m.execute(req, e_.get());  
  // RPC request fails because FileEntry#3 does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->set(1, String::g("0"));
  res = m.execute(req, e_.get());  
  // RPC request fails because index of FileEntry is string.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->set(1, Integer::g(1));
  req.params->set(2, String::g("http://url"));
  res = m.execute(req, e_.get());  
  // RPC request fails because 3rd param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->set(2, List::g());
  req.params->set(3, String::g("http://url"));
  res = m.execute(req, e_.get());  
  // RPC request fails because 4th param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testGetSessionInfo()
{
  GetSessionInfoRpcMethod m;
  RpcRequest req(GetSessionInfoRpcMethod::getMethodName(), List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(util::toHex(e_->getSessionId()),
                       getString(downcast<Dict>(res.param), "sessionId"));
}

void RpcMethodTest::testPause()
{
  const std::string URIS[] = {
    "http://url1",
    "http://url2",
    "http://url3",
  };
  std::vector<std::string> uris(vbegin(URIS), vend(URIS));
  option_->put(PREF_FORCE_SEQUENTIAL, A2_V_TRUE);
  std::vector<SharedHandle<RequestGroup> > groups;
  createRequestGroupForUri(groups, option_, uris);
  CPPUNIT_ASSERT_EQUAL((size_t)3, groups.size());  
  e_->getRequestGroupMan()->addReservedGroup(groups);
  {
    PauseRpcMethod m;
    RpcRequest req(PauseRpcMethod::getMethodName(), List::g());
    req.params->append("1");
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(groups[0]->isPauseRequested());
  {
    UnpauseRpcMethod m;
    RpcRequest req(UnpauseRpcMethod::getMethodName(), List::g());
    req.params->append("1");
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(!groups[0]->isPauseRequested());
  {
    PauseAllRpcMethod m;
    RpcRequest req(PauseAllRpcMethod::getMethodName(), List::g());
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
  {
    UnpauseAllRpcMethod m;
    RpcRequest req(UnpauseAllRpcMethod::getMethodName(), List::g());
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(!groups[i]->isPauseRequested());
  }
  {
    ForcePauseAllRpcMethod m;
    RpcRequest req(ForcePauseAllRpcMethod::getMethodName(), List::g());
    RpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
}

void RpcMethodTest::testSystemMulticall()
{
  SystemMulticallRpcMethod m;
  RpcRequest req("system.multicall", List::g());
  SharedHandle<List> reqparams = List::g();
  req.params->append(reqparams);
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", AddUriRpcMethod::getMethodName());
    SharedHandle<List> params = List::g();
    SharedHandle<List> urisParam = List::g();
    urisParam->append("http://localhost/"+util::itos(i));
    params->append(urisParam);
    dict->put("params", params);
    reqparams->append(dict);
  }
  {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", "not exists");
    dict->put("params", List::g());
    reqparams->append(dict);
  }
  {
    reqparams->append("not struct");
  }
  {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", "system.multicall");
    dict->put("params", List::g());
    reqparams->append(dict);
  }
  {
    // missing params
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", GetVersionRpcMethod::getMethodName());
    reqparams->append(dict);
  }
  {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", GetVersionRpcMethod::getMethodName());
    dict->put("params", List::g());
    reqparams->append(dict);
  }
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)7, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("1"),
                       downcast<String>(downcast<List>(resParams->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       downcast<String>(downcast<List>(resParams->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       downcast<Integer>
                       (downcast<Dict>(resParams->get(2))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       downcast<Integer>
                       (downcast<Dict>(resParams->get(3))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       downcast<Integer>
                       (downcast<Dict>(resParams->get(4))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT(downcast<List>(resParams->get(5)));
  CPPUNIT_ASSERT(downcast<List>(resParams->get(6)));
}

void RpcMethodTest::testSystemMulticall_fail()
{
  SystemMulticallRpcMethod m;
  RpcRequest req("system.multicall", List::g());
  RpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

} // namespace rpc

} // namespace aria2
