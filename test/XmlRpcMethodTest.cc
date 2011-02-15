#include "XmlRpcMethod.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadEngine.h"
#include "SelectEventPoll.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "XmlRpcMethodImpl.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "XmlRpcRequest.h"
#include "XmlRpcResponse.h"
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

namespace xmlrpc {

class XmlRpcMethodTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcMethodTest);
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
    option_->put(PREF_DIR, A2_TEST_OUT_DIR"/aria2_XmlRpcMethodTest");
    option_->put(PREF_SEGMENT_SIZE, "1048576");
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


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcMethodTest);

namespace {
std::string getString(const Dict* dict, const std::string& key)
{
  return asString(dict->get(key))->s();
}
} // namespace

void XmlRpcMethodTest::testAddUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost/");
  req.params->append(urisParam);
  {
    XmlRpcResponse res = m.execute(req, e_.get());
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
  opt->put(PREF_DIR, "/sink");
  req.params->append(opt);
  {
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(std::string("/sink"),
                         e_->getRequestGroupMan()->findReservedGroup(2)->
                         getOption()->get(PREF_DIR));
  }
}

void XmlRpcMethodTest::testAddUri_withoutUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddUri_notUri()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("not uri");
  req.params->append(urisParam);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddUri_withBadOption()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost");
  req.params->append(urisParam);
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_FILE_ALLOCATION, "badvalue");
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddUri_withPosition()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req1(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam1 = List::g();
  urisParam1->append("http://uri1");
  req1.params->append(urisParam1);
  XmlRpcResponse res1 = m.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);
  
  XmlRpcRequest req2(AddUriXmlRpcMethod::getMethodName(), List::g());
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

void XmlRpcMethodTest::testAddUri_withBadPosition()
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam = List::g();
  urisParam->append("http://localhost/");
  req.params->append(urisParam);
  req.params->append(Dict::g());
  req.params->append(Integer::g(-1));
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

#ifdef ENABLE_BITTORRENT
void XmlRpcMethodTest::testAddTorrent()
{
  File(e_->getOption()->get(PREF_DIR)+
       "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").remove();
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(A2_TEST_DIR"/single.torrent"));
  SharedHandle<List> uris = List::g();
  uris->append("http://localhost/aria2-0.8.2.tar.bz2");
  req.params->append(uris);
  {
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT
      (File(e_->getOption()->get(PREF_DIR)+
            "/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").exists());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), asString(res.param)->s());

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
  std::string dir = A2_TEST_OUT_DIR"/aria2_XmlRpcMethodTest_testAddTorrent";
  File(dir).mkdirs();
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_DIR, dir);
  File(dir+"/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").remove();
  req.params->append(opt);
  {
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL
      (dir+"/aria2-0.8.2.tar.bz2",
       e_->getRequestGroupMan()->findReservedGroup(2)->getFirstFilePath());
    CPPUNIT_ASSERT
      (File(dir+"/0a3893293e27ac0490424c06de4d09242215f0a6.torrent").exists());
  }
}

void XmlRpcMethodTest::testAddTorrent_withoutTorrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddTorrent_notBase64Torrent()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), List::g());
  req.params->append("not torrent");
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddTorrent_withPosition()
{
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req1(AddTorrentXmlRpcMethod::getMethodName(), List::g());
  req1.params->append(readFile(A2_TEST_DIR"/test.torrent"));
  req1.params->append(List::g());
  req1.params->append(Dict::g());
  XmlRpcResponse res1 = m.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  XmlRpcRequest req2(AddTorrentXmlRpcMethod::getMethodName(), List::g());
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
void XmlRpcMethodTest::testAddMetalink()
{
  File(e_->getOption()->get(PREF_DIR)+
       "/c908634fbc257fd56f0114912c2772aeeb4064f4.metalink").remove();
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(A2_TEST_DIR"/2files.metalink"));
  {
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    const List* resParams = asList(res.param);
    CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
    CPPUNIT_ASSERT_EQUAL(std::string("1"), asString(resParams->get(0))->s());
    CPPUNIT_ASSERT_EQUAL(std::string("2"), asString(resParams->get(1))->s());
    CPPUNIT_ASSERT
      (File(e_->getOption()->get(PREF_DIR)+
            "/c908634fbc257fd56f0114912c2772aeeb4064f4.metalink").exists());

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
  std::string dir = A2_TEST_OUT_DIR"/aria2_XmlRpcMethodTest_testAddMetalink";
  File(dir).mkdirs();
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_DIR, dir);
  File(dir+"/c908634fbc257fd56f0114912c2772aeeb4064f4.metalink").remove();
  req.params->append(opt);
  {
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
    CPPUNIT_ASSERT_EQUAL(dir+"/aria2-5.0.0.tar.bz2",
                         e_->getRequestGroupMan()->findReservedGroup(3)->
                         getFirstFilePath());
    CPPUNIT_ASSERT
      (File(dir+"/c908634fbc257fd56f0114912c2772aeeb4064f4.metalink").exists());
  }
}

void XmlRpcMethodTest::testAddMetalink_withoutMetalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddMetalink_notBase64Metalink()
{
  AddMetalinkXmlRpcMethod m;
  XmlRpcRequest req(AddMetalinkXmlRpcMethod::getMethodName(), List::g());
  req.params->append("not metalink");
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testAddMetalink_withPosition()
{
  AddUriXmlRpcMethod m1;
  XmlRpcRequest req1(AddUriXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<List> urisParam1 = List::g();
  urisParam1->append("http://uri");
  req1.params->append(urisParam1);
  XmlRpcResponse res1 = m1.execute(req1, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res1.code);

  AddMetalinkXmlRpcMethod m2;
  XmlRpcRequest req2("ari2.addMetalink", List::g());
  req2.params->append(readFile(A2_TEST_DIR"/2files.metalink"));
  req2.params->append(Dict::g());
  req2.params->append(Integer::g(0));
  XmlRpcResponse res2 = m2.execute(req2, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res2.code);

  CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR)+"/aria2-5.0.0.tar.bz2",
                       e_->getRequestGroupMan()->getReservedGroups()[0]->
                       getFirstFilePath());
}

#endif // ENABLE_METALINK

void XmlRpcMethodTest::testChangeOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_BT_MAX_PEERS, "100");
  opt->put(PREF_BT_REQUEST_PEER_SPEED_LIMIT, "300K");
  opt->put(PREF_MAX_UPLOAD_LIMIT, "50K");

  BtObject btObject;
  btObject.btRuntime_ = SharedHandle<BtRuntime>(new BtRuntime());
  e_->getBtRegistry()->put(group->getGID(), btObject);
#endif // ENABLE_BITTORRENT
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());

  SharedHandle<Option> option = group->getOption();

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100*1024,
                       group->getMaxDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       option->get(PREF_MAX_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL(std::string("307200"),
                       option->get(PREF_BT_REQUEST_PEER_SPEED_LIMIT));

  CPPUNIT_ASSERT_EQUAL(std::string("100"), option->get(PREF_BT_MAX_PEERS));
  CPPUNIT_ASSERT_EQUAL((unsigned int)100, btObject.btRuntime_->getMaxPeers());

  CPPUNIT_ASSERT_EQUAL((unsigned int)50*1024,
                       group->getMaxUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       option->get(PREF_MAX_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeOption_withBadOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT, "badvalue");
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testChangeOption_withNotAllowedOption()
{
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  e_->getRequestGroupMan()->addReservedGroup(group);

  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT, "100K");
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testChangeOption_withoutGid()
{
  ChangeOptionXmlRpcMethod m;
  XmlRpcRequest req(ChangeOptionXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testChangeGlobalOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT, "100K");
#ifdef ENABLE_BITTORRENT
  opt->put(PREF_MAX_OVERALL_UPLOAD_LIMIT, "50K");
#endif // ENABLE_BITTORRENT
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());

  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL
    ((unsigned int)100*1024,
     e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("102400"),
                       e_->getOption()->get(PREF_MAX_OVERALL_DOWNLOAD_LIMIT));
#ifdef ENABLE_BITTORRENT
  CPPUNIT_ASSERT_EQUAL
    ((unsigned int)50*1024,
     e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
  CPPUNIT_ASSERT_EQUAL(std::string("51200"),
                       e_->getOption()->get(PREF_MAX_OVERALL_UPLOAD_LIMIT));
#endif // ENABLE_BITTORRENT
}

void XmlRpcMethodTest::testChangeGlobalOption_withBadOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_OVERALL_DOWNLOAD_LIMIT, "badvalue");
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testChangeGlobalOption_withNotAllowedOption()
{
  ChangeGlobalOptionXmlRpcMethod m;
  XmlRpcRequest req
    (ChangeGlobalOptionXmlRpcMethod::getMethodName(), List::g());
  SharedHandle<Dict> opt = Dict::g();
  opt->put(PREF_MAX_DOWNLOAD_LIMIT, "100K");
  req.params->append(opt);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testNoSuchMethod()
{
  NoSuchMethodXmlRpcMethod m;
  XmlRpcRequest req("make.hamburger", List::g());
  XmlRpcResponse res = m.execute(req, 0);
  CPPUNIT_ASSERT_EQUAL(1, res.code);
  CPPUNIT_ASSERT_EQUAL(std::string("No such method: make.hamburger"),
                       getString(asDict(res.param), "faultString"));
  CPPUNIT_ASSERT_EQUAL
    (std::string("<?xml version=\"1.0\"?>"
                 "<methodResponse>"
                 "<fault>"
                 "<value>"
                 "<struct>"
                 "<member>"
                 "<name>faultCode</name><value><int>1</int></value>"
                 "</member>"
                 "<member>"
                 "<name>faultString</name>"
                 "<value>"
                 "<string>No such method: make.hamburger</string>"
                 "</value>"
                 "</member>"
                 "</struct>"
                 "</value>"
                 "</fault>"
                 "</methodResponse>"),
     res.toXml());
}

void XmlRpcMethodTest::testTellStatus_withoutGid()
{
  TellStatusXmlRpcMethod m;
  XmlRpcRequest req(TellStatusXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

namespace {
void addUri(const std::string& uri, const SharedHandle<DownloadEngine>& e)
{
  AddUriXmlRpcMethod m;
  XmlRpcRequest req(AddUriXmlRpcMethod::getMethodName(), List::g());
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
  AddTorrentXmlRpcMethod m;
  XmlRpcRequest req(AddTorrentXmlRpcMethod::getMethodName(), List::g());
  req.params->append(readFile(torrentFile));
  XmlRpcResponse res = m.execute(req, e.get());
}
} // namespace
#endif // ENABLE_BITTORRENT

void XmlRpcMethodTest::testTellWaiting()
{
  addUri("http://1/", e_);
  addUri("http://2/", e_);
  addUri("http://3/", e_);
#ifdef ENABLE_BITTORRENT
  addTorrent(A2_TEST_DIR"/single.torrent", e_);
#else // !ENABLE_BITTORRENT
  addUri("http://4/", e_);
#endif // !ENABLE_BITTORRENT

  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req(TellWaitingXmlRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(2));
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       getString(asDict(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"),
                       getString(asDict(resParams->get(1)), "gid"));
  // waiting.size() == offset+num 
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(3));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // waiting.size() < offset+num 
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(1));
  req.params->append(Integer::g(4));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)3, resParams->size());
  // negative offset
  req = XmlRpcRequest(TellWaitingXmlRpcMethod::getMethodName(), List::g());
  req.params->append(Integer::g(-1));
  req.params->append(Integer::g(2));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)2, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("4"),
                       getString(asDict(resParams->get(0)), "gid"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"),
                       getString(asDict(resParams->get(1)), "gid"));
  // negative offset and size < num
  req.params->set(1, Integer::g(100));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)4, resParams->size());
  // nagative offset and normalized offset < 0
  req.params->set(0, Integer::g(-5));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)0, resParams->size());
  // nagative offset and normalized offset == 0
  req.params->set(0, Integer::g(-4));
  res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)1, resParams->size());
}

void XmlRpcMethodTest::testTellWaiting_fail()
{
  TellWaitingXmlRpcMethod m;
  XmlRpcRequest req(TellWaitingXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testGetVersion()
{
  GetVersionXmlRpcMethod m;
  XmlRpcRequest req(GetVersionXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const Dict* resParams = asDict(res.param);
  CPPUNIT_ASSERT_EQUAL(std::string(PACKAGE_VERSION),
                       getString(resParams, "version"));
  const List* featureList = asList(resParams->get("enabledFeatures"));
  std::string features;
  for(List::ValueType::const_iterator i = featureList->begin();
      i != featureList->end(); ++i) {
    const String* s = asString(*i);
    features += s->s();
    features += ", ";
  }
  
  CPPUNIT_ASSERT_EQUAL(FeatureConfig::getInstance()->featureSummary()+", ",
                       features);
}

void XmlRpcMethodTest::testGatherStoppedDownload()
{
  std::vector<SharedHandle<FileEntry> > fileEntries;
  std::vector<gid_t> followedBy;
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

  const List* followedByRes = asList(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(std::string("3"), asString(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("4"), asString(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       asString(entry->get("belongsTo"))->s());

  keys.push_back("gid");

  entry = Dict::g();
  gatherStoppedDownload(entry, d, keys);
  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
}

void XmlRpcMethodTest::testGatherProgressCommon()
{
  SharedHandle<DownloadContext> dctx(new DownloadContext(0, 0,"aria2.tar.bz2"));
  std::string uris[] = { "http://localhost/aria2.tar.bz2" };
  dctx->getFirstFileEntry()->addUris(vbegin(uris), vend(uris));
  SharedHandle<RequestGroup> group(new RequestGroup(option_));
  group->setDownloadContext(dctx);
  std::vector<SharedHandle<RequestGroup> > followedBy;
  for(int i = 0; i < 2; ++i) {
    followedBy.push_back(SharedHandle<RequestGroup>(new RequestGroup(option_)));
  }

  group->followedBy(followedBy.begin(), followedBy.end());
  group->belongsTo(2);

  SharedHandle<Dict> entry = Dict::g();
  std::vector<std::string> keys;
  gatherProgressCommon(entry, group, keys);
  
  const List* followedByRes = asList(entry->get("followedBy"));
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[0]->getGID()),
                       asString(followedByRes->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(util::itos(followedBy[1]->getGID()),
                       asString(followedByRes->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       asString(entry->get("belongsTo"))->s());
  const List* files = asList(entry->get("files"));
  CPPUNIT_ASSERT_EQUAL((size_t)1, files->size());
  const Dict* file = asDict(files->get(0));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2.tar.bz2"),
                       asString(file->get("path"))->s());
  CPPUNIT_ASSERT_EQUAL(uris[0],
                       asString
                       (asDict
                        (asList(file->get("uris"))->get(0))
                        ->get("uri"))
                       ->s());
  CPPUNIT_ASSERT_EQUAL(e_->getOption()->get(PREF_DIR),
                       asString(entry->get("dir"))->s());

  keys.push_back("gid");
  entry = Dict::g();
  gatherProgressCommon(entry, group, keys);

  CPPUNIT_ASSERT_EQUAL((size_t)1, entry->size());
  CPPUNIT_ASSERT(entry->containsKey("gid"));
  
}

#ifdef ENABLE_BITTORRENT
void XmlRpcMethodTest::testGatherBitTorrentMetadata()
{
  SharedHandle<Option> option(new Option());
  option->put(PREF_DIR, ".");
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  bittorrent::load(A2_TEST_DIR"/test.torrent", dctx, option);
  SharedHandle<Dict> btDict = Dict::g();
  gatherBitTorrentMetadata(btDict, bittorrent::getTorrentAttrs(dctx));
  CPPUNIT_ASSERT_EQUAL(std::string("REDNOAH.COM RULES"),
                       asString(btDict->get("comment"))->s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1123456789,
                       asInteger(btDict->get("creationDate"))->i());
  CPPUNIT_ASSERT_EQUAL(std::string("multi"),
                       asString(btDict->get("mode"))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2-test"),
                       asString
                       (asDict
                        (btDict->get("info"))
                        ->get("name"))
                       ->s());
  const List* announceList = asList(btDict->get("announceList"));
  CPPUNIT_ASSERT_EQUAL((size_t)3, announceList->size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"),
                       asString(asList(announceList->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"),
                       asString(asList(announceList->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker3"),
                       asString(asList(announceList->get(2))->get(0))->s());
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

void XmlRpcMethodTest::testChangePosition()
{
  e_->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(option_)));
  e_->getRequestGroupMan()->addReservedGroup
    (SharedHandle<RequestGroup>(new RequestGroup(option_)));

  ChangePositionXmlRpcMethod m;
  XmlRpcRequest req(ChangePositionXmlRpcMethod::getMethodName(), List::g());
  req.params->append("1");
  req.params->append(Integer::g(1));
  req.params->append("POS_SET");
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)1, asInteger(res.param)->i());
  CPPUNIT_ASSERT_EQUAL
    ((gid_t)1, e_->getRequestGroupMan()->getReservedGroups()[1]->getGID());
}

void XmlRpcMethodTest::testChangePosition_fail()
{
  ChangePositionXmlRpcMethod m;
  XmlRpcRequest req(ChangePositionXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);

  req.params->append("1");
  req.params->append(Integer::g(2));
  req.params->append("bad keyword");
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

void XmlRpcMethodTest::testChangeUri()
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

  ChangeUriXmlRpcMethod m;
  XmlRpcRequest req(ChangeUriXmlRpcMethod::getMethodName(), List::g());
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
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL((int64_t)2, asInteger(asList(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)3, asInteger(asList(res.param)->get(1))->i());
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
  CPPUNIT_ASSERT_EQUAL((int64_t)0, asInteger(asList(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, asInteger(asList(res.param)->get(1))->i());
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
  CPPUNIT_ASSERT_EQUAL((int64_t)0, asInteger(asList(res.param)->get(0))->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)2, asInteger(asList(res.param)->get(1))->i());
  uris = files[0]->getRemainingUris();
  CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-1"), uris[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://example.org/added1-2"), uris[1]);
}

void XmlRpcMethodTest::testChangeUri_fail()
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

  ChangeUriXmlRpcMethod m;
  XmlRpcRequest req(ChangeUriXmlRpcMethod::getMethodName(), List::g());
  req.params->append("1"); // GID
  req.params->append(Integer::g(1)); // index of FileEntry
  SharedHandle<List> removeuris = List::g();
  req.params->append(removeuris);
  SharedHandle<List> adduris = List::g();
  req.params->append(adduris);
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);

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

void XmlRpcMethodTest::testGetSessionInfo()
{
  GetSessionInfoXmlRpcMethod m;
  XmlRpcRequest req(GetSessionInfoXmlRpcMethod::getMethodName(), List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  CPPUNIT_ASSERT_EQUAL(util::toHex(e_->getSessionId()),
                       getString(asDict(res.param), "sessionId"));
}

void XmlRpcMethodTest::testPause()
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
    PauseXmlRpcMethod m;
    XmlRpcRequest req(PauseXmlRpcMethod::getMethodName(), List::g());
    req.params->append("1");
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(groups[0]->isPauseRequested());
  {
    UnpauseXmlRpcMethod m;
    XmlRpcRequest req(UnpauseXmlRpcMethod::getMethodName(), List::g());
    req.params->append("1");
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  CPPUNIT_ASSERT(!groups[0]->isPauseRequested());
  {
    PauseAllXmlRpcMethod m;
    XmlRpcRequest req(PauseAllXmlRpcMethod::getMethodName(), List::g());
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
  {
    UnpauseAllXmlRpcMethod m;
    XmlRpcRequest req(UnpauseAllXmlRpcMethod::getMethodName(), List::g());
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(!groups[i]->isPauseRequested());
  }
  {
    ForcePauseAllXmlRpcMethod m;
    XmlRpcRequest req(ForcePauseAllXmlRpcMethod::getMethodName(), List::g());
    XmlRpcResponse res = m.execute(req, e_.get());
    CPPUNIT_ASSERT_EQUAL(0, res.code);
  }
  for(size_t i = 0; i < groups.size(); ++i) {
    CPPUNIT_ASSERT(groups[i]->isPauseRequested());
  }
}

void XmlRpcMethodTest::testSystemMulticall()
{
  SystemMulticallXmlRpcMethod m;
  XmlRpcRequest req("system.multicall", List::g());
  SharedHandle<List> reqparams = List::g();
  req.params->append(reqparams);
  for(int i = 0; i < 2; ++i) {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", AddUriXmlRpcMethod::getMethodName());
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
    dict->put("methodName", GetVersionXmlRpcMethod::getMethodName());
    reqparams->append(dict);
  }
  {
    SharedHandle<Dict> dict = Dict::g();
    dict->put("methodName", GetVersionXmlRpcMethod::getMethodName());
    dict->put("params", List::g());
    reqparams->append(dict);
  }
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(0, res.code);
  const List* resParams = asList(res.param);
  CPPUNIT_ASSERT_EQUAL((size_t)7, resParams->size());
  CPPUNIT_ASSERT_EQUAL(std::string("1"),
                       asString(asList(resParams->get(0))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("2"),
                       asString(asList(resParams->get(1))->get(0))->s());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       asInteger
                       (asDict(resParams->get(2))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       asInteger
                       (asDict(resParams->get(3))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       asInteger
                       (asDict(resParams->get(4))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT_EQUAL((int64_t)1,
                       asInteger
                       (asDict(resParams->get(5))->get("faultCode"))
                       ->i());
  CPPUNIT_ASSERT(asList(resParams->get(6)));
}

void XmlRpcMethodTest::testSystemMulticall_fail()
{
  SystemMulticallXmlRpcMethod m;
  XmlRpcRequest req("system.multicall", List::g());
  XmlRpcResponse res = m.execute(req, e_.get());
  CPPUNIT_ASSERT_EQUAL(1, res.code);
}

} // namespace xmlrpc

} // namespace aria2
