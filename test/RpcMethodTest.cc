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
#include "RpcMethodFactory.h"
#ifdef ENABLE_BITTORRENT
#  include "BtRegistry.h"
#  include "BtRuntime.h"
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace rpc {

class RpcMethodTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(RpcMethodTest);
  CPPUNIT_TEST(testAuthorize);
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
  CPPUNIT_TEST(testGetOption);
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
#ifdef ENABLE_BITTORRENT
  CPPUNIT_TEST(testGatherStoppedDownload_bt);
#endif // ENABLE_BITTORRENT
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
  CPPUNIT_TEST(testSystemListMethods);
  CPPUNIT_TEST(testSystemListNotifications);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<DownloadEngine> e_;
  std::shared_ptr<Option> option_;

public:
  void setUp()
  {
    option_ = std::make_shared<Option>();
    option_->put(PREF_DIR, A2_TEST_OUT_DIR "/aria2_RpcMethodTest");
    option_->put(PREF_PIECE_LENGTH, "1048576");
    option_->put(PREF_MAX_DOWNLOAD_RESULT, "10");
    File(option_->get(PREF_DIR)).mkdirs();
    e_ = make_unique<DownloadEngine>(make_unique<SelectEventPoll>());
    e_->setOption(option_.get());
    e_->setRequestGroupMan(make_unique<RequestGroupMan>(
        std::vector<std::shared_ptr<RequestGroup>>{}, 1, option_.get()));
  }

  void testAuthorize();
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
  void testGetOption();
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
#ifdef ENABLE_BITTORRENT
  void testGatherStoppedDownload_bt();
#endif // ENABLE_BITTORRENT
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
  void testSystemListMethods();
  void testSystemListNotifications();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RpcMethodTest);

namespace {
std::string getString(const Dict* dict, const std::string& key)
{
  return downcast<String>(dict->get(key))->s();
}
} // namespace

namespace {
RpcRequest createReq(std::string methodName)
{
  return {std::move(methodName), List::g()};
}
} // namespace

void RpcMethodTest::testAuthorize()
{
  // Select RPC method which takes non-string parameter to make sure
  // that token: prefixed parameter is stripped before the call.
  TellActiveRpcMethod m;
  // no secret token set and no token: prefixed parameter is given
  {
    auto req = createReq(TellActiveRpcMethod::getMethodName());
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  // no secret token set and token: prefixed parameter is given
  {
    auto req = createReq(GetVersionRpcMethod::getMethodName());
    req.params->append("token:foo");
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  e_->getOption()->put(PREF_RPC_SECRET, "foo");
  // secret token set and no token: prefixed parameter is given
  {
    auto req = createReq(GetVersionRpcMethod::getMethodName());
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(1, res.code);
  }
  // secret token set and token: prefixed parameter is given
  {
    auto req = createReq(GetVersionRpcMethod::getMethodName());
    req.params->append("token:foo");
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  // secret token set and bad token: prefixed parameter is given
  {
    auto req = createReq(GetVersionRpcMethod::getMethodName());
    req.params->append("token:foo2");
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(1, res.code);
  }
}

void RpcMethodTest::testAddUri()
{
  AddUriRpcMethod m;
  {
    auto req = createReq(AddUriRpcMethod::getMethodName());
    auto urisParam = List::g();
    urisParam->append("http://localhost/");
    req.params->append(std::move(urisParam));
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const RequestGroupList& rgs = e_->getRequestGroupMan()->getReservedGroups();
    CPPUNIT_ASSERT_EQUAL((size_t)1, rgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/"),
                         (*rgs.begin())
                             ->getDownloadContext()
                             ->getFirstFileEntry()
                             ->getRemainingUris()
                             .front());
  }
  {
    auto req = createReq(AddUriRpcMethod::getMethodName());
    auto urisParam = List::g();
    urisParam->append("http://localhost/");
    req.params->append(std::move(urisParam));
    // with options
    auto opt = Dict::g();
    opt->put(PREF_DIR->k, "/sink");
    req.params->append(std::move(opt));
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    a2_gid_t gid;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(gid, downcast<String>(res.param)->s().c_str()));
    CPPUNIT_ASSERT_EQUAL(std::string("/sink"),
                         findReservedGroup(e_->getRequestGroupMan().get(), gid)
                             ->getOption()
                             ->get(PREF_DIR));
  }
}

void RpcMethodTest::testAddUri_withoutUri()
{
  AddUriRpcMethod m;
  auto res = m.execute(createReq(AddUriRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_notUri()
{
  AddUriRpcMethod m;
  auto req = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam = List::g();
  urisParam->append("not uri");
  req.params->append(std::move(urisParam));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_withBadOption()
{
  AddUriRpcMethod m;
  auto req = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam = List::g();
  urisParam->append("http://localhost");
  req.params->append(std::move(urisParam));
  auto opt = Dict::g();
  opt->put(PREF_FILE_ALLOCATION->k, "badvalue");
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddUri_withPosition()
{
  AddUriRpcMethod m;
  auto req1 = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam1 = List::g();
  urisParam1->append("http://uri1");
  req1.params->append(std::move(urisParam1));
  auto res1 = m.execute(std::move(req1), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  auto req2 = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam2 = List::g();
  urisParam2->append("http://uri2");
  req2.params->append(std::move(urisParam2));
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  m.execute(std::move(req2), e_.get());

  std::string uri = getReservedGroup(e_->getRequestGroupMan().get(), 0)
                        ->getDownloadContext()
                        ->getFirstFileEntry()
                        ->getRemainingUris()[0];

  CPPUNIT_ASSERT_EQUAL(std::string("http://uri2"), uri);
}

void RpcMethodTest::testAddUri_withBadPosition()
{
  AddUriRpcMethod m;
  auto req = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam = List::g();
  urisParam->append("http://localhost/");
  req.params->append(std::move(urisParam));
  req.params->append(Dict::g());
  req.params->append(Integer::g(-1));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

#ifdef ENABLE_BITTORRENT
namespace {
RpcRequest createAddTorrentReq()
{
  auto req = createReq(AddTorrentRpcMethod::getMethodName());
  req.params->append(readFile(A2_TEST_DIR "/single.torrent"));
  auto uris = List::g();
  uris->append("http://localhost/aria2-0.8.2.tar.bz2");
  req.params->append(std::move(uris));
  return req;
}
} // namespace

void RpcMethodTest::testAddTorrent()
{
  File(e_->getOption()->get(PREF_DIR) +
       "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent")
      .remove();
  AddTorrentRpcMethod m;
  {
    // Saving upload metadata is disabled by option.
    auto res = m.execute(createAddTorrentReq(), e_.get());
    CPPUNIT_ASSERT(!File(e_->getOption()->get(PREF_DIR) +
                         "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent")
                        .exists());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(sizeof(a2_gid_t) * 2,
                         downcast<String>(res.param)->s().size());
  }
  e_->getOption()->put(PREF_RPC_SAVE_UPLOAD_METADATA, A2_V_TRUE);
  {
    auto res = m.execute(createAddTorrentReq(), e_.get());
    CPPUNIT_ASSERT(File(e_->getOption()->get(PREF_DIR) +
                        "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent")
                       .exists());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    a2_gid_t gid;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(gid, downcast<String>(res.param)->s().c_str()));

    auto group = findReservedGroup(e_->getRequestGroupMan().get(), gid);
    CPPUNIT_ASSERT(group);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR) +
                             "/aria2-0.8.2.tar.bz2",
                         group->getFirstFilePath());
    CPPUNIT_ASSERT_EQUAL((size_t)1, group->getDownloadContext()
                                        ->getFirstFileEntry()
                                        ->getRemainingUris()
                                        .size());
    CPPUNIT_ASSERT_EQUAL(std::string("http://localhost/aria2-0.8.2.tar.bz2"),
                         group->getDownloadContext()
                             ->getFirstFileEntry()
                             ->getRemainingUris()[0]);
  }
  {
    auto req = createAddTorrentReq();
    // with options
    std::string dir = A2_TEST_OUT_DIR "/aria2_RpcMethodTest_testAddTorrent";
    File(dir).mkdirs();
    auto opt = Dict::g();
    opt->put(PREF_DIR->k, dir);
    File(dir + "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").remove();
    req.params->append(std::move(opt));

    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    a2_gid_t gid;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(gid, downcast<String>(res.param)->s().c_str()));
    CPPUNIT_ASSERT_EQUAL(dir + "/aria2-0.8.2.tar.bz2",
                         findReservedGroup(e_->getRequestGroupMan().get(), gid)
                             ->getFirstFilePath());
    CPPUNIT_ASSERT(
        File(dir + "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent")
            .exists());
  }
}

void RpcMethodTest::testAddTorrent_withoutTorrent()
{
  AddTorrentRpcMethod m;
  auto res =
      m.execute(createReq(AddTorrentRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddTorrent_notBase64Torrent()
{
  AddTorrentRpcMethod m;
  auto req = createReq(AddTorrentRpcMethod::getMethodName());
  req.params->append("not torrent");
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddTorrent_withPosition()
{
  AddTorrentRpcMethod m;
  auto req1 = createReq(AddTorrentRpcMethod::getMethodName());
  req1.params->append(readFile(A2_TEST_DIR "/test.torrent"));
  req1.params->append(List::g());
  req1.params->append(Dict::g());
  auto res1 = m.execute(std::move(req1), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  auto req2 = createReq(AddTorrentRpcMethod::getMethodName());
  req2.params->append(readFile(A2_TEST_DIR "/single.torrent"));
  req2.params->append(List::g());
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  m.execute(std::move(req2), e_.get());

  CPPUNIT_ASSERT_EQUAL((size_t)1,
                       getReservedGroup(e_->getRequestGroupMan().get(), 0)
                           ->getDownloadContext()
                           ->getFileEntries()
                           .size());
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
namespace {
RpcRequest createAddMetalinkReq()
{
  auto req = createReq(AddMetalinkRpcMethod::getMethodName());
  req.params->append(readFile(A2_TEST_DIR "/2files.metalink"));
  return req;
}
} // namespace

void RpcMethodTest::testAddMetalink()
{
  File(e_->getOption()->get(PREF_DIR) +
       "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4")
      .remove();
  AddMetalinkRpcMethod m;
  {
    // Saving upload metadata is disabled by option.
    auto res = m.execute(createAddMetalinkReq(), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const List* resParams = downcast<List>(res.param);
    CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
    a2_gid_t gid1, gid2;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(
               gid1, downcast<String>(resParams->get(0))->s().c_str()));
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(
               gid2, downcast<String>(resParams->get(1))->s().c_str()));
    CPPUNIT_ASSERT(!File(e_->getOption()->get(PREF_DIR) +
                         "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4")
                        .exists());
  }
  e_->getOption()->put(PREF_RPC_SAVE_UPLOAD_METADATA, A2_V_TRUE);
  {
    auto res = m.execute(createAddMetalinkReq(), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const List* resParams = downcast<List>(res.param);
    CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
    a2_gid_t gid3, gid4;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(
               gid3, downcast<String>(resParams->get(0))->s().c_str()));
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(
               gid4, downcast<String>(resParams->get(1))->s().c_str()));
    CPPUNIT_ASSERT(File(e_->getOption()->get(PREF_DIR) +
                        "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4")
                       .exists());

    auto tar = findReservedGroup(e_->getRequestGroupMan().get(), gid3);
    CPPUNIT_ASSERT(tar);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR) +
                             "/aria2-5.0.0.tar.bz2",
                         tar->getFirstFilePath());
    auto deb = findReservedGroup(e_->getRequestGroupMan().get(), gid4);
    CPPUNIT_ASSERT(deb);
    CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR) + "/aria2-5.0.0.deb",
                         deb->getFirstFilePath());
  }
  {
    auto req = createAddMetalinkReq();
    // with options
    std::string dir = A2_TEST_OUT_DIR "/aria2_RpcMethodTest_testAddMetalink";
    File(dir).mkdirs();
    auto opt = Dict::g();
    opt->put(PREF_DIR->k, dir);
    File(dir + "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").remove();
    req.params->append(std::move(opt));

    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const List* resParams = downcast<List>(res.param);
    CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
    a2_gid_t gid5;
    CPPUNIT_ASSERT_EQUAL(
        0, GroupId::toNumericId(
               gid5, downcast<String>(resParams->get(0))->s().c_str()));
    CPPUNIT_ASSERT_EQUAL(dir + "/aria2-5.0.0.tar.bz2",
                         findReservedGroup(e_->getRequestGroupMan().get(), gid5)
                             ->getFirstFilePath());
    CPPUNIT_ASSERT(
        File(dir + "/c908634fbc257fd56f0114912c2772aeeb4064f4.meta4").exists());
  }
}

void RpcMethodTest::testAddMetalink_withoutMetalink()
{
  AddMetalinkRpcMethod m;
  auto res =
      m.execute(createReq(AddMetalinkRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddMetalink_notBase64Metalink()
{
  AddMetalinkRpcMethod m;
  auto req = createReq(AddMetalinkRpcMethod::getMethodName());
  req.params->append("not metalink");
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testAddMetalink_withPosition()
{
  AddUriRpcMethod m1;
  auto req1 = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam1 = List::g();
  urisParam1->append("http://uri");
  req1.params->append(std::move(urisParam1));
  auto res1 = m1.execute(std::move(req1), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  AddMetalinkRpcMethod m2;
  auto req2 = createReq(AddMetalinkRpcMethod::getMethodName());
  req2.params->append(readFile(A2_TEST_DIR "/2files.metalink"));
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  auto res2 = m2.execute(std::move(req2), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res2.code);

  CPPUNIT_ASSERT_EQUAL(
      e_->getOption()->get(PREF_DIR) + "/aria2-5.0.0.tar.bz2",
      getReservedGroup(e_->getRequestGroupMan().get(), 0)->getFirstFilePath());
}

#endif // ENABLE_METALINK

void RpcMethodTest::testGetOption()
{
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  group->getOption()->put(PREF_DIR, "alpha");
  e_->getRequestGroupMan()->addReservedGroup(group);
  auto dr = createDownloadResult(error_code::FINISHED, "http://host/fin");
  dr->option->put(PREF_DIR, "bravo");
  e_->getRequestGroupMan()->addDownloadResult(dr);

  GetOptionRpcMethod m;
  auto req = createReq(GetOptionRpcMethod::getMethodName());
  req.params->append(GroupId::toHex(group->getGID()));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const Dict* resopt = downcast<Dict>(res.param);
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"),
                       downcast<String>(resopt->get(PREF_DIR->k))->s());

  req = createReq(GetOptionRpcMethod::getMethodName());
  req.params->append(dr->gid->toHex());
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resopt = downcast<Dict>(res.param);
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"),
                       downcast<String>(resopt->get(PREF_DIR->k))->s());
  // Invalid GID
  req = createReq(GetOptionRpcMethod::getMethodName());
  req.params->append(GroupId::create()->toHex());
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeOption()
{
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  auto req = createReq(ChangeOptionRpcMethod::getMethodName());
  req.params->append(GroupId::toHex(group->getGID()));
  auto opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT->k, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_BT_MAX_PEERS->k, "100");
  opt->put(PREF_BT_REQUEST_PEER_SPEED_LIMIT->k, "300K");
  opt->put(PREF_MAX_UPLOAD_LIMIT->k, "50K");

  {
    auto btObject = make_unique<BtObject>();
    btObject->btRuntime = std::make_shared<BtRuntime>();
    e_->getBtRegistry()->put(group->getGID(), std::move(btObject));
  }
#endif // ENABLE_BITTORRENT
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());

  auto option = group->getOption();

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int)100_k, group->getMaxDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       option->get(PREF_MAX_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(std::string("307200"),
                       option->get(PREF_BT_REQUEST_PEER_SPEED_LIMIT));

  CPPUNIT_ASSERT_EQUAL(std::string("100"), option->get(PREF_BT_MAX_PEERS));
  CPPUNIT_ASSERT_EQUAL(
      100, e_->getBtRegistry()->get(group->getGID())->btRuntime->getMaxPeers());

  CPPUNIT_ASSERT_EQUAL((int)50_k, group->getMaxUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       option->get(PREF_MAX_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void RpcMethodTest::testChangeOption_withBadOption()
{
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  auto req = createReq(ChangeOptionRpcMethod::getMethodName());
  req.params->append(GroupId::toHex(group->getGID()));
  auto opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT->k, "badvalue");
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeOption_withNotAllowedOption()
{
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionRpcMethod m;
  auto req = createReq(ChangeOptionRpcMethod::getMethodName());
  req.params->append(GroupId::toHex(group->getGID()));
  auto opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "100K");
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());
  // The unacceptable options are just ignored.
  CPPUNIT_ASSERT_EQUAL(0, res.code);
}

void RpcMethodTest::testChangeOption_withoutGid()
{
  ChangeOptionRpcMethod m;
  auto res =
      m.execute(createReq(ChangeOptionRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeGlobalOption()
{
  ChangeGlobalOptionRpcMethod m;
  auto req = createReq(ChangeGlobalOptionRpcMethod::getMethodName());
  auto opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_MAX_OVERALL_UPLOAD_LIMIT->k, "50K");
#endif // ENABLE_BITTORRENT
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(
      (int)100_k, e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       e_->getOption()->get(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(
      (int)50_k, e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       e_->getOption()->get(PREF_MAX_OVERALL_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void RpcMethodTest::testChangeGlobalOption_withBadOption()
{
  ChangeGlobalOptionRpcMethod m;
  auto req = createReq(ChangeGlobalOptionRpcMethod::getMethodName());
  auto opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT->k, "badvalue");
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testChangeGlobalOption_withNotAllowedOption()
{
  ChangeGlobalOptionRpcMethod m;
  auto req = createReq(ChangeGlobalOptionRpcMethod::getMethodName());
  auto opt = Dict::g();
  opt->put(PREF_ENABLE_RPC->k, "100K");
  req.params->append(std::move(opt));
  auto res = m.execute(std::move(req), e_.get());
  // The unacceptable options are just ignored.
  CPPUNIT_ASSERT_EQUAL(0, res.code);
}

void RpcMethodTest::testNoSuchMethod()
{
  NoSuchMethodRpcMethod m;
  auto res = m.execute(createReq("make.hamburger"), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
  CPPUNIT_ASSERT_EQUAL(std::string("No such method: make.hamburger"),
                       getString(downcast<Dict>(res.param), "faultString"));
}

void RpcMethodTest::testTellStatus_withoutGid()
{
  TellStatusRpcMethod m;
  auto res =
      m.execute(createReq(TellStatusRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

namespace {
void addUri(const std::string& uri, const std::shared_ptr<DownloadEngine>& e)
{
  AddUriRpcMethod m;
  auto req = createReq(AddUriRpcMethod::getMethodName());
  auto urisParam = List::g();
  urisParam->append(uri);
  req.params->append(std::move(urisParam));
  CPPUNIT_ASSERT_EQUAL(0, m.execute(std::move(req), e.get()).code);
}
} // namespace

#ifdef ENABLE_BITTORRENT
namespace {
void addTorrent(const std::string& torrentFile,
                const std::shared_ptr<DownloadEngine>& e)
{
  AddTorrentRpcMethod m;
  auto req = createReq(AddTorrentRpcMethod::getMethodName());
  req.params->append(readFile(torrentFile));
  auto res = m.execute(std::move(req), e.get());
}
} // namespace
#endif // ENABLE_BITTORRENT

void RpcMethodTest::testTellWaiting()
{
  addUri("http://1/", e_);
  addUri("http://2/", e_);
  addUri("http://3/", e_);
#ifdef ENABLE_BITTORRENT
  addTorrent(A2_TEST_DIR "/single.torrent", e_);
#else  // !ENABLE_BITTORRENT
  addUri("http://4/", e_);
#endif // !ENABLE_BITTORRENT
  auto& rgman = e_->getRequestGroupMan();
  TellWaitingRpcMethod m;
  auto req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(2));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 1)->getGID()),
      getString(downcast<Dict>(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 2)->getGID()),
      getString(downcast<Dict>(resParams->get(1)), "gid"));
  // waiting.size() == offset+num
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(3));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // waiting.size() < offset+num
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(4));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());

  // offset = INT32_MAX
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(INT32_MAX));
  req.params->append(Integer::g(1));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // num = INT32_MAX
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(INT32_MAX));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // offset=INT32_MAX and num = INT32_MAX
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(INT32_MAX));
  req.params->append(Integer::g(INT32_MAX));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // offset=INT32_MIN and num = INT32_MAX
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(INT32_MIN));
  req.params->append(Integer::g(INT32_MAX));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());

  // negative offset
  req = createReq(TellWaitingRpcMethod::getMethodName());
  req.params->append(Integer::g(-1));
  req.params->append(Integer::g(2));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 3)->getGID()),
      getString(downcast<Dict>(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 2)->getGID()),
      getString(downcast<Dict>(resParams->get(1)), "gid"));
  // negative offset and size < num
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(-1));
  req.params->append(Integer::g(100));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)4, resParams->size());
  // nagative offset and normalized offset < 0
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(-5));
  req.params->append(Integer::g(100));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // nagative offset and normalized offset == 0
  req = RpcRequest(TellWaitingRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(-4));
  req.params->append(Integer::g(100));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)1, resParams->size());
}

void RpcMethodTest::testTellWaiting_fail()
{
  TellWaitingRpcMethod m;
  auto res =
      m.execute(createReq(TellWaitingRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testGetVersion()
{
  GetVersionRpcMethod m;
  auto res =
      m.execute(createReq(GetVersionRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const Dict* resParams = downcast<Dict>(res.param);
  CPPUNIT_ASSERT_EQUAL(std::string(PACKAGE_VERSION),
                       getString(resParams, "version"));
  const List* featureList = downcast<List>(resParams->get("enabledFeatures"));
  std::string features;
  for (auto i = featureList->begin(); i != featureList->end(); ++i) {
    const String* s = downcast<String>(*i);
    features += s->s();
    features += ", ";
  }
  CPPUNIT_ASSERT_EQUAL(featureSummary() + ", ", features);
}

void RpcMethodTest::testGatherStoppedDownload()
{
  std::vector<std::shared_ptr<FileEntry>> fileEntries;
  std::vector<a2_gid_t> followedBy;
  followedBy.push_back(3);
  followedBy.push_back(4);
  auto d = std::make_shared<DownloadResult>();
  d->gid = GroupId::create();
  d->fileEntries = fileEntries;
  d->inMemoryDownload = false;
  d->sessionDownloadLength = UINT64_MAX;
  d->sessionTime = 1_s;
  d->result = error_code::FINISHED;
  d->followedBy = followedBy;
  d->following = 1;
  d->belongsTo = 2;
  auto entry = Dict::g();
  std::vector<std::string> keys;
  gatherStoppedDownload(entry.get(), d, keys);

  const List* followedByRes = downcast<List>(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(3),
                       downcast<String>(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(4),
                       downcast<String>(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(1),
                       downcast<String>(entry->get("following"))->s());
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(2),
                       downcast<String>(entry->get("belongsTo"))->s());

  keys.push_back("gid");

  entry = Dict::g();
  gatherStoppedDownload(entry.get(), d, keys);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
}

#ifdef ENABLE_BITTORRENT
void RpcMethodTest::testGatherStoppedDownload_bt()
{
  auto d = std::make_shared<DownloadResult>();
  d->gid = GroupId::create();
  d->infoHash = "2089b05ecca3d829cee5497d2703803b52216d19";
  d->attrs = std::vector<std::shared_ptr<ContextAttribute>>(MAX_CTX_ATTR);

  auto torrentAttr = std::make_shared<TorrentAttribute>();
  torrentAttr->creationDate = 1000000007;
  d->attrs[CTX_ATTR_BT] = torrentAttr;

  auto entry = Dict::g();
  gatherStoppedDownload(entry.get(), d, {});

  auto btDict = downcast<Dict>(entry->get("bittorrent"));
  CPPUNIT_ASSERT(btDict);

  CPPUNIT_ASSERT_EQUAL((int64_t)1000000007,
                       downcast<Integer>(btDict->get("creationDate"))->i());
}
#endif // ENABLE_BITTORRENT

void RpcMethodTest::testGatherProgressCommon()
{
  auto dctx = std::make_shared<DownloadContext>(0, 0, "aria2.tar.bz2");
  std::string uris[] = {"http://localhost/aria2.tar.bz2"};
  dctx->getFirstFileEntry()->addUris(std::begin(uris), std::end(uris));
  auto group =
      std::make_shared<RequestGroup>(GroupId::create(), util::copy(option_));
  group->setDownloadContext(dctx);
  std::vector<std::shared_ptr<RequestGroup>> followedBy;
  for (int i = 0; i < 2; ++i) {
    followedBy.push_back(
        std::make_shared<RequestGroup>(GroupId::create(), util::copy(option_)));
  }

  group->followedBy(followedBy.begin(), followedBy.end());
  auto leader = GroupId::create();
  group->following(leader->getNumericId());
  auto parent = GroupId::create();
  group->belongsTo(parent->getNumericId());

  auto entry = Dict::g();
  std::vector<std::string> keys;
  gatherProgressCommon(entry.get(), group, keys);

  const List* followedByRes = downcast<List>(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(followedBy[0]->getGID()),
                       downcast<String>(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(GroupId::toHex(followedBy[1]->getGID()),
                       downcast<String>(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(leader->toHex(),
                       downcast<String>(entry->get("following"))->s());
  CPPUNIT_ASSERT_EQUAL(parent->toHex(),
                       downcast<String>(entry->get("belongsTo"))->s());
  const List* files = downcast<List>(entry->get("files"));
  CPPUNIT_ASSERT_EQUAL((size_t)1, files->size());
  const Dict* file = downcast<Dict>(files->get(0));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       downcast<String>(file->get("path"))->s());
  CPPUNIT_ASSERT_EQUAL(
      uris[0],
      downcast<String>(
          downcast<Dict>(downcast<List>(file->get("uris"))->get(0))->get("uri"))
          ->s());
  CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR),
                       downcast<String>(entry->get("dir"))->s());

  keys.push_back("gid");
  entry = Dict::g();
  gatherProgressCommon(entry.get(), group, keys);

  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
}

#ifdef ENABLE_BITTORRENT
void RpcMethodTest::testGatherBitTorrentMetadata()
{
  auto option = std::make_shared<Option>();
  option->put(PREF_DIR, ".");
  auto dctx = std::make_shared<DownloadContext>();
  bittorrent::load(A2_TEST_DIR "/test.torrent", dctx, option);
  auto btDict = Dict::g();
  gatherBitTorrentMetadata(btDict.get(), bittorrent::getTorrentAttrs(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
                       downcast<String>(btDict->get("comment"))->s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1123456789,
                       downcast<Integer>(btDict->get("creationDate"))->i());
  CPPUNIT_ASSERT_EQUAL(std::string("multi"),
                       downcast<String>(btDict->get("mode"))->s());
  CPPUNIT_ASSERT_EQUAL(
      std::string("aria2-test"),
      downcast<String>(downcast<Dict>(btDict->get("info"))->get("name"))->s());
  const List* announceList = downcast<List>(btDict->get("announceList"));
  CPPUNIT_ASSERT_EQUAL((size_t)3, announceList->size());
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://tracker1"),
      downcast<String>(downcast<List>(announceList->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://tracker2"),
      downcast<String>(downcast<List>(announceList->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(
      std::string("http://tracker3"),
      downcast<String>(downcast<List>(announceList->get(2))->get(0))->s());
  // Remove some keys
  auto modBtAttrs = bittorrent::getTorrentAttrs(dctx);
  modBtAttrs->comment.clear();
  modBtAttrs->creationDate = 0;
  modBtAttrs->mode = BT_FILE_MODE_NONE;
  modBtAttrs->metadata.clear();
  btDict = Dict::g();
  gatherBitTorrentMetadata(btDict.get(), modBtAttrs);
  CPPUNIT_ASSERT(!btDict->containsKey("comment"));
  CPPUNIT_ASSERT(!btDict->containsKey("creationDate"));
  CPPUNIT_ASSERT(!btDict->containsKey("mode"));
  CPPUNIT_ASSERT(!btDict->containsKey("info"));
  CPPUNIT_ASSERT(btDict->containsKey("announceList"));
}
#endif // ENABLE_BITTORRENT

void RpcMethodTest::testChangePosition()
{
  e_->getRequestGroupMan()->addReservedGroup(
      std::make_shared<RequestGroup>(GroupId::create(), util::copy(option_)));
  e_->getRequestGroupMan()->addReservedGroup(
      std::make_shared<RequestGroup>(GroupId::create(), util::copy(option_)));

  a2_gid_t gid = getReservedGroup(e_->getRequestGroupMan().get(), 0)->getGID();
  ChangePositionRpcMethod m;
  auto req = createReq(ChangePositionRpcMethod::getMethodName());
  req.params->append(GroupId::toHex(gid));
  req.params->append(Integer::g(1));
  req.params->append("POS_SET");
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)1, downcast<Integer>(res.param)->i());
  CPPUNIT_ASSERT_EQUAL(
      gid, getReservedGroup(e_->getRequestGroupMan().get(), 1)->getGID());
}

void RpcMethodTest::testChangePosition_fail()
{
  ChangePositionRpcMethod m;
  auto res =
      m.execute(createReq(ChangePositionRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  auto req = createReq(ChangePositionRpcMethod::getMethodName());
  req.params->append("1");
  req.params->append(Integer::g(2));
  req.params->append("bad keyword");
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

namespace {
RpcRequest createChangeUriReq(a2_gid_t gid, size_t fileIndex)
{
  auto req = createReq(ChangeUriRpcMethod::getMethodName());

  req.params->append(GroupId::toHex(gid));   // GID
  req.params->append(Integer::g(fileIndex)); // index of FileEntry
  auto removeuris = List::g();
  removeuris->append("http://example.org/mustremove1");
  removeuris->append("http://example.org/mustremove2");
  removeuris->append("http://example.org/notexist");
  req.params->append(std::move(removeuris));
  return req;
}
} // namespace

void RpcMethodTest::testChangeUri()
{
  std::shared_ptr<FileEntry> files[3];
  for (int i = 0; i < 3; ++i) {
    files[i].reset(new FileEntry());
  }
  files[1]->addUri("http://example.org/aria2.tar.bz2");
  files[1]->addUri("http://example.org/mustremove1");
  files[1]->addUri("http://example.org/mustremove2");
  auto dctx = std::make_shared<DownloadContext>();
  dctx->setFileEntries(&files[0], &files[3]);
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  group->setDownloadContext(dctx);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriRpcMethod m;
  auto req = createChangeUriReq(group->getGID(), 2);
  auto adduris = List::g();
  adduris->append("http://example.org/added1");
  adduris->append("http://example.org/added2");
  adduris->append("baduri");
  adduris->append("http://example.org/added3");
  req.params->append(std::move(adduris));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)3, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[0]->getRemainingUris().size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, files[2]->getRemainingUris().size());
  std::deque<std::string> uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)4, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/aria2.tar.bz2"),
                       uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1"), uris[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added2"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added3"), uris[3]);

  req = createChangeUriReq(group->getGID(), 2);
  // Change adduris
  adduris = List::g();
  adduris->append("http://example.org/added1-1");
  adduris->append("http://example.org/added1-2");
  req.params->append(std::move(adduris));
  // Set position parameter
  req.params->append(Integer::g(2));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)0, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  uris = files[1]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)6, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[2]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[3]);

  // Change index of FileEntry
  req = createChangeUriReq(group->getGID(), 1);
  adduris = List::g();
  adduris->append("http://example.org/added1-1");
  adduris->append("http://example.org/added1-2");
  req.params->append(std::move(adduris));
  // Set position far beyond the size of uris in FileEntry.
  req.params->append(Integer::g(1000));
  res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)0, downcast<Integer>(downcast<List>(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)2, downcast<Integer>(downcast<List>(res.param)->get(1))->i());
  uris = files[0]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[1]);
}

namespace {
RpcRequest createChangeUriEmptyReq(a2_gid_t gid, size_t fileIndex)
{
  auto req = createReq(ChangeUriRpcMethod::getMethodName());

  req.params->append(GroupId::toHex(gid));   // GID
  req.params->append(Integer::g(fileIndex)); // index of FileEntry
  req.params->append(List::g());             // remove uris
  req.params->append(List::g());             // append uris
  return req;
}
} // namespace

void RpcMethodTest::testChangeUri_fail()
{
  std::shared_ptr<FileEntry> files[3];
  for (int i = 0; i < 3; ++i) {
    files[i] = std::make_shared<FileEntry>();
  }
  auto dctx = std::make_shared<DownloadContext>();
  dctx->setFileEntries(&files[0], &files[3]);
  auto group = std::make_shared<RequestGroup>(GroupId::create(), option_);
  group->setDownloadContext(dctx);
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeUriRpcMethod m;
  auto req = createChangeUriEmptyReq(group->getGID(), 1);
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);

  req = createChangeUriEmptyReq(group->getGID(), 0);
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because 2nd argument is less than 1.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req = createChangeUriEmptyReq(GroupId::create()->getNumericId(), 1);
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because the given GID does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req = createChangeUriEmptyReq(group->getGID(), 4);
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because FileEntry#3 does not exist.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req = createChangeUriEmptyReq(group->getGID(), 1);
  req.params->set(1, String::g("0"));
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because index of FileEntry is string.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req = createChangeUriEmptyReq(group->getGID(), 1);
  req.params->set(2, String::g("http://url"));
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because 3rd param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req = createChangeUriEmptyReq(group->getGID(), 1);
  req.params->set(2, List::g());
  req.params->set(3, String::g("http://url"));
  res = m.execute(std::move(req), e_.get());
  // RPC request fails because 4th param is not list.
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testGetSessionInfo()
{
  GetSessionInfoRpcMethod m;
  auto res =
      m.execute(createReq(GetSessionInfoRpcMethod::getMethodName()), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(util::toHex(e_->getSessionId()),
                       getString(downcast<Dict>(res.param), "sessionId"));
}

void RpcMethodTest::testPause()
{
  std::vector<std::string> uris{
      "http://url1",
      "http://url2",
      "http://url3",
  };
  option_->put(PREF_FORCE_SEQUENTIAL, A2_V_TRUE);
  std::vector<std::shared_ptr<RequestGroup>> groups;
  createRequestGroupForUri(groups, option_, uris);
  CPPUNIT_ASSERT_EQUAL((size_t)3, groups.size());
  e_->getRequestGroupMan()->addReservedGroup(groups);
  {
    PauseRpcMethod m;
    auto req = createReq(PauseRpcMethod::getMethodName());
    req.params->append(GroupId::toHex(groups[0]->getGID()));
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(groups[0]->isPauseRequested());
  {
    UnpauseRpcMethod m;
    auto req = createReq(UnpauseRpcMethod::getMethodName());
    req.params->append(GroupId::toHex(groups[0]->getGID()));
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(!groups[0]->isPauseRequested());
  {
    PauseAllRpcMethod m;
    auto req = createReq(PauseAllRpcMethod::getMethodName());
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for (size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
  {
    UnpauseAllRpcMethod m;
    auto req = createReq(UnpauseAllRpcMethod::getMethodName());
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for (size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(!groups[i]->isPauseRequested());
  }
  {
    ForcePauseAllRpcMethod m;
    auto req = createReq(ForcePauseAllRpcMethod::getMethodName());
    auto res = m.execute(std::move(req), e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for (size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
}

void RpcMethodTest::testSystemMulticall()
{
  SystemMulticallRpcMethod m;
  auto req = createReq("system.multicall");
  auto reqparams = List::g();
  for (int i = 0; i < 2; ++i) {
    auto dict = Dict::g();
    dict->put("methodName", AddUriRpcMethod::getMethodName());
    auto params = List::g();
    auto urisParam = List::g();
    urisParam->append("http://localhost/" + util::itos(i));
    params->append(std::move(urisParam));
    dict->put("params", std::move(params));
    reqparams->append(std::move(dict));
  }
  {
    auto dict = Dict::g();
    dict->put("methodName", "not exists");
    dict->put("params", List::g());
    reqparams->append(std::move(dict));
  }
  {
    reqparams->append("not struct");
  }
  {
    auto dict = Dict::g();
    dict->put("methodName", "system.multicall");
    dict->put("params", List::g());
    reqparams->append(std::move(dict));
  }
  {
    // missing params
    auto dict = Dict::g();
    dict->put("methodName", GetVersionRpcMethod::getMethodName());
    reqparams->append(std::move(dict));
  }
  {
    auto dict = Dict::g();
    dict->put("methodName", GetVersionRpcMethod::getMethodName());
    dict->put("params", List::g());
    reqparams->append(std::move(dict));
  }
  req.params->append(std::move(reqparams));
  auto res = m.execute(std::move(req), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = downcast<List>(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)7, resParams->size());
  auto& rgman = e_->getRequestGroupMan();
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 0)->getGID()),
      downcast<String>(downcast<List>(resParams->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(
      GroupId::toHex(getReservedGroup(rgman.get(), 1)->getGID()),
      downcast<String>(downcast<List>(resParams->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)1,
      downcast<Integer>(downcast<Dict>(resParams->get(2))->get("faultCode"))
          ->i());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)1,
      downcast<Integer>(downcast<Dict>(resParams->get(3))->get("faultCode"))
          ->i());
  CPPUNIT_ASSERT_EQUAL(
      (int64_t)1,
      downcast<Integer>(downcast<Dict>(resParams->get(4))->get("faultCode"))
          ->i());
  CPPUNIT_ASSERT(downcast<List>(resParams->get(5)));
  CPPUNIT_ASSERT(downcast<List>(resParams->get(6)));
}

void RpcMethodTest::testSystemMulticall_fail()
{
  SystemMulticallRpcMethod m;
  auto res = m.execute(createReq("system.multicall"), e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void RpcMethodTest::testSystemListMethods()
{
  SystemListMethodsRpcMethod m;
  auto res = m.execute(createReq("system.listMethods"), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);

  const auto resParams = downcast<List>(res.param);
  auto& allNames = allMethodNames();

  CPPUNIT_ASSERT_EQUAL(allNames.size(), resParams->size());

  for (size_t i = 0; i < allNames.size(); ++i) {
    const auto s = downcast<String>(resParams->get(i));
    CPPUNIT_ASSERT(s);
    CPPUNIT_ASSERT_EQUAL(allNames[i], s->s());
  }
}

void RpcMethodTest::testSystemListNotifications()
{
  SystemListNotificationsRpcMethod m;
  auto res = m.execute(createReq("system.listNotifications"), e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);

  const auto resParams = downcast<List>(res.param);
  auto& allNames = allNotificationsNames();

  CPPUNIT_ASSERT_EQUAL(allNames.size(), resParams->size());

  for (size_t i = 0; i < allNames.size(); ++i) {
    const auto s = downcast<String>(resParams->get(i));
    CPPUNIT_ASSERT(s);
    CPPUNIT_ASSERT_EQUAL(allNames[i], s->s());
  }
}

} // namespace rpc

} // namespace aria2
