#include "IteratableChunkChecksumValidator.h"
#include "SingleFileDownloadContext.h"
#include "DefaultPieceStorage.h"
#include "Option.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class IteratableChunkChecksumValidatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IteratableChunkChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST_SUITE_END();
private:

  static const char* csArray[];
public:
  void setUp() {
  }

  void testValidate();
};


CPPUNIT_TEST_SUITE_REGISTRATION( IteratableChunkChecksumValidatorTest );

const char* IteratableChunkChecksumValidatorTest::csArray[] = { "29b0e7878271645fffb7eec7db4a7473a1c00bc1",
						      "4df75a661cb7eb2733d9cdaa7f772eae3a4e2976",
						      "0a4ea2f7dd7c52ddf2099a444ab2184b4d341bdb" };

void IteratableChunkChecksumValidatorTest::testValidate() {
  Option option;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(100, 250, "chunkChecksumTestFile250.txt"));
  dctx->setPieceHashes(std::deque<std::string>(&csArray[0], &csArray[3]));
  dctx->setPieceHashAlgo("sha1");
  SharedHandle<DefaultPieceStorage> ps
    (new DefaultPieceStorage(dctx, &option));
  ps->initStorage();
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
  badHashes[1] = "ffffffffffffffffffffffffffffffffffffffff";
  dctx->setPieceHashes(badHashes);

  validator.init();

  while(!validator.finished()) {
    validator.validateChunk();
  }
  CPPUNIT_ASSERT(ps->hasPiece(0));
  CPPUNIT_ASSERT(!ps->hasPiece(1));
  CPPUNIT_ASSERT(ps->hasPiece(2));
}

} // namespace aria2
