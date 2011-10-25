#include "BtDependency.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "DefaultPieceStorage.h"
#include "DownloadContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "Exception.h"
#include "SegmentMan.h"
#include "Segment.h"
#include "FileEntry.h"
#include "PieceSelector.h"
#include "bittorrent_helper.h"
#include "DirectDiskAdaptor.h"
#include "ByteArrayDiskWriter.h"
#include "MockPieceStorage.h"
#include "prefs.h"
#include "util.h"

namespace aria2 {

class BtDependencyTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtDependencyTest);
  CPPUNIT_TEST(testResolve);
  CPPUNIT_TEST(testResolve_nullDependee);
  CPPUNIT_TEST(testResolve_originalNameNoMatch);
  CPPUNIT_TEST(testResolve_singleFileWithoutOriginalName);
  CPPUNIT_TEST(testResolve_multiFile);
  CPPUNIT_TEST(testResolve_metadata);
  CPPUNIT_TEST(testResolve_loadError);
  CPPUNIT_TEST(testResolve_dependeeFailure);
  CPPUNIT_TEST(testResolve_dependeeInProgress);
  CPPUNIT_TEST_SUITE_END();

  SharedHandle<RequestGroup> createDependant(const SharedHandle<Option>& option)
  {
    SharedHandle<RequestGroup> dependant(new RequestGroup(util::copy(option)));
    SharedHandle<DownloadContext> dctx
      (new DownloadContext(0, 0, "/tmp/outfile.path"));
    std::vector<std::string> uris;
    uris.push_back("http://localhost/outfile.path");
    SharedHandle<FileEntry> fileEntry = dctx->getFirstFileEntry();
    fileEntry->setUris(uris);
    fileEntry->setOriginalName("aria2-0.8.2.tar.bz2");
    dependant->setDownloadContext(dctx);
    return dependant;
  }

  SharedHandle<RequestGroup>
  createDependee
  (const SharedHandle<Option>& option,
   const std::string& torrentFile,
   int64_t length)
  {
    SharedHandle<RequestGroup> dependee(new RequestGroup(util::copy(option)));
    SharedHandle<DownloadContext> dctx
      (new DownloadContext(1024*1024, length, torrentFile));
    dependee->setDownloadContext(dctx);
    dependee->initPieceStorage();
    return dependee;
  }

  SharedHandle<Option> option_;
public:
  void setUp()
  {
    option_.reset(new Option());
    option_->put(PREF_DIR, "/tmp");
  }

  void testResolve();
  void testResolve_nullDependee();
  void testResolve_originalNameNoMatch();
  void testResolve_singleFileWithoutOriginalName();
  void testResolve_multiFile();
  void testResolve_metadata();
  void testResolve_loadError();
  void testResolve_dependeeFailure();
  void testResolve_dependeeInProgress();
};


CPPUNIT_TEST_SUITE_REGISTRATION( BtDependencyTest );

void BtDependencyTest::testResolve()
{
  std::string filename = A2_TEST_DIR"/single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, filename, File(filename).size());
  dependee->getPieceStorage()->getDiskAdaptor()->enableReadOnly();
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());

  CPPUNIT_ASSERT_EQUAL
    (std::string("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf"),
     bittorrent::getInfoHashString(dependant->getDownloadContext()));
  const SharedHandle<FileEntry>& firstFileEntry =
    dependant->getDownloadContext()->getFirstFileEntry();
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
                       firstFileEntry->getPath());
  CPPUNIT_ASSERT_EQUAL((size_t)1, firstFileEntry->getRemainingUris().size());
  CPPUNIT_ASSERT(firstFileEntry->isRequested());
}

void BtDependencyTest::testResolve_nullDependee()
{
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  BtDependency dep(dependant.get(), SharedHandle<RequestGroup>());
  CPPUNIT_ASSERT(dep.resolve());
}

void BtDependencyTest::testResolve_originalNameNoMatch()
{
  std::string filename = A2_TEST_DIR"/single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  dependant->getDownloadContext()->getFirstFileEntry()->setOriginalName
    ("aria2-1.1.0.tar.bz2");
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, filename, File(filename).size());
  dependee->getPieceStorage()->getDiskAdaptor()->enableReadOnly();
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());

  CPPUNIT_ASSERT(!dependant->getDownloadContext()->hasAttribute
                 (bittorrent::BITTORRENT));
}

void BtDependencyTest::testResolve_singleFileWithoutOriginalName()
{
  std::string filename = A2_TEST_DIR"/single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  dependant->getDownloadContext()->getFirstFileEntry()->setOriginalName("");
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, filename, File(filename).size());
  dependee->getPieceStorage()->getDiskAdaptor()->enableReadOnly();
  dependee->getPieceStorage()->markAllPiecesDone();
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());
  CPPUNIT_ASSERT(dependant->getDownloadContext()->hasAttribute
                 (bittorrent::BITTORRENT));
}

void BtDependencyTest::testResolve_multiFile()
{
  std::string filename = A2_TEST_DIR"/test.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  dependant->getDownloadContext()->getFirstFileEntry()->setOriginalName
    ("aria2-test/aria2/src/aria2c");
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, filename, File(filename).size());
  dependee->getPieceStorage()->getDiskAdaptor()->enableReadOnly();
  dependee->getPieceStorage()->markAllPiecesDone();
  
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());

  CPPUNIT_ASSERT(dependant->getDownloadContext()->hasAttribute
                 (bittorrent::BITTORRENT));

  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    dependant->getDownloadContext()->getFileEntries();
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
                       fileEntries[0]->getPath());
  CPPUNIT_ASSERT(fileEntries[0]->isRequested());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-test/aria2-0.2.2.tar.bz2"),
                       fileEntries[1]->getPath());
  CPPUNIT_ASSERT(!fileEntries[1]->isRequested());
}

void BtDependencyTest::testResolve_metadata()
{
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, "metadata", 0);

  SharedHandle<DirectDiskAdaptor> diskAdaptor(new DirectDiskAdaptor());
  SharedHandle<ByteArrayDiskWriter> diskWriter(new ByteArrayDiskWriter());
  diskAdaptor->setDiskWriter(diskWriter);
  diskWriter->setString
    ("d4:name19:aria2-0.8.2.tar.bz26:lengthi384e12:piece lengthi128e"
     "6:pieces60:AAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCC"
     "e");
  SharedHandle<MockPieceStorage> pieceStorage(new MockPieceStorage());
  pieceStorage->setDiskAdaptor(diskAdaptor);
  pieceStorage->setDownloadFinished(true);
  dependee->setPieceStorage(pieceStorage);
  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  dependee->getDownloadContext()->setAttribute(bittorrent::BITTORRENT, attrs);

  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());

  CPPUNIT_ASSERT_EQUAL
    (std::string("cd41c7fdddfd034a15a04d7ff881216e01c4ceaf"),
     bittorrent::getInfoHashString(dependant->getDownloadContext()));
  CPPUNIT_ASSERT
    (dependant->getDownloadContext()->getFirstFileEntry()->isRequested());
}

void BtDependencyTest::testResolve_loadError()
{
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, "notExist", 40);
  dependee->getPieceStorage()->markAllPiecesDone();
    
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());
    
  CPPUNIT_ASSERT
    (!dependant->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
                       dependant->getFirstFilePath());
}

void BtDependencyTest::testResolve_dependeeFailure()
{
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  SharedHandle<RequestGroup> dependee = createDependee(option_, "notExist", 40);
    
  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(dep.resolve());
  
  CPPUNIT_ASSERT
    (!dependant->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT));
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
                       dependant->getFirstFilePath());
}

void BtDependencyTest::testResolve_dependeeInProgress()
{
  std::string filename = A2_TEST_DIR"/single.torrent";
  SharedHandle<RequestGroup> dependant = createDependant(option_);
  SharedHandle<RequestGroup> dependee =
    createDependee(option_, filename, File(filename).size());
  dependee->increaseNumCommand();

  BtDependency dep(dependant.get(), dependee);
  CPPUNIT_ASSERT(!dep.resolve());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/outfile.path"),
                       dependant->getFirstFilePath());
}

} // namespace aria2
