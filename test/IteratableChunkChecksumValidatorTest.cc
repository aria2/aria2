#include "IteratableChunkChecksumValidator.h"

#include <cppunit/extensions/HelperMacros.h>

#include "TestUtil.h"
#include "DownloadContext.h"
#include "DefaultPieceStorage.h"
#include "Option.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include "PieceSelector.h"

namespace aria2 {

class IteratableChunkChecksumValidatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IteratableChunkChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate_readError);
  CPPUNIT_TEST_SUITE_END();
private:

  static const std::string csArray[];
public:
  void setUp() {
  }

  void testValidate();
  void testValidate_readError();
};


CPPUNIT_TEST_SUITE_REGISTRATION( IteratableChunkChecksumValidatorTest );

const std::string IteratableChunkChecksumValidatorTest::csArray[] =
  { fromHex("29b0e7878271645fffb7eec7db4a7473a1c00bc1"),
    fromHex("4df75a661cb7eb2733d9cdaa7f772eae3a4e2976"),
    fromHex("0a4ea2f7dd7c52ddf2099a444ab2184b4d341bdb") };

void IteratableChunkChecksumValidatorTest::testValidate() {
  Option option;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(100, 250, A2_TEST_DIR"/chunkChecksumTestFile250.txt"));
  dctx->setPieceHashes("sha-1", &csArray[0], &csArray[3]);
  SharedHandle<DefaultPieceStorage> ps
    (new DefaultPieceStorage(dctx, &option));
  ps->initStorage();
  ps->getDiskAdaptor()->enableReadOnly();
  ps->getDiskAdaptor()->openFile();

  IteratableChunkChecksumValidator validator(dctx, ps);
  validator.init();

  validator.validateChunk();
  CPPUNIT_ASSERT(!validator.finished());
  validator.validateChunk();
  CPPUNIT_ASSERT(!validator.finished());
  validator.validateChunk();
  CPPUNIT_ASSERT(validator.finished());
  CPPUNIT_ASSERT(ps->downloadFinished());

  // make the test fail
  std::deque<std::string> badHashes(&csArray[0], &csArray[3]);
  badHashes[1] = fromHex("ffffffffffffffffffffffffffffffffffffffff");
  dctx->setPieceHashes("sha-1", badHashes.begin(), badHashes.end());

  validator.init();

  while(!validator.finished()) {
    validator.validateChunk();
  }
  CPPUNIT_ASSERT(ps->hasPiece(0));
  CPPUNIT_ASSERT(!ps->hasPiece(1));
  CPPUNIT_ASSERT(ps->hasPiece(2));
}

void IteratableChunkChecksumValidatorTest::testValidate_readError() {
  Option option;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(100, 500, A2_TEST_DIR"/chunkChecksumTestFile250.txt"));
  std::deque<std::string> hashes(&csArray[0], &csArray[3]);
  hashes.push_back(fromHex("ffffffffffffffffffffffffffffffffffffffff"));
  hashes.push_back(fromHex("ffffffffffffffffffffffffffffffffffffffff"));
  dctx->setPieceHashes("sha-1", hashes.begin(), hashes.end());
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &option));
  ps->initStorage();
  ps->getDiskAdaptor()->enableReadOnly();
  ps->getDiskAdaptor()->openFile();

  IteratableChunkChecksumValidator validator(dctx, ps);
  validator.init();

  while(!validator.finished()) {
    validator.validateChunk();
  }

  CPPUNIT_ASSERT(ps->hasPiece(0));
  CPPUNIT_ASSERT(ps->hasPiece(1));
  CPPUNIT_ASSERT(!ps->hasPiece(2)); // #2 piece is not valid because
                                    // #program expects its size is
                                    // #100, but it reads only 50
                                    // #bytes and raises error.
  CPPUNIT_ASSERT(!ps->hasPiece(3));
  CPPUNIT_ASSERT(!ps->hasPiece(4));
}

} // namespace aria2
