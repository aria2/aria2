#include "IteratableChecksumValidator.h"
#include "SingleFileDownloadContext.h"
#include "DefaultPieceStorage.h"
#include "Option.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class IteratableChecksumValidatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IteratableChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate_fail);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testValidate();
  void testValidate_fail();
};


CPPUNIT_TEST_SUITE_REGISTRATION( IteratableChecksumValidatorTest );

void IteratableChecksumValidatorTest::testValidate() {
  Option option;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(100, 250, "chunkChecksumTestFile250.txt"));
  dctx->setChecksum("898a81b8e0181280ae2ee1b81e269196d91e869a");
  dctx->setChecksumHashAlgo("sha1");
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &option));
  ps->initStorage();
  ps->getDiskAdaptor()->openFile();

  IteratableChecksumValidator validator(dctx, ps);
  validator.init();
  while(!validator.finished()) {
    validator.validateChunk();
  }

  CPPUNIT_ASSERT(ps->downloadFinished());
}

void IteratableChecksumValidatorTest::testValidate_fail() {
  Option option;
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(100, 250, "chunkChecksumTestFile250.txt"));
  dctx->setChecksum(std::string(40, '0')); // set wrong checksum
  dctx->setChecksumHashAlgo("sha1");
  SharedHandle<DefaultPieceStorage> ps(new DefaultPieceStorage(dctx, &option));
  ps->initStorage();
  ps->getDiskAdaptor()->openFile();

  IteratableChecksumValidator validator(dctx, ps);
  validator.init();
  
  while(!validator.finished()) {
    validator.validateChunk();
  }

  CPPUNIT_ASSERT(!ps->downloadFinished());
}

} // namespace aria2
