#include "IteratableChecksumValidator.h"
#include "DefaultDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class IteratableChecksumValidatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IteratableChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate2);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testValidate();
  void testValidate2();
};


CPPUNIT_TEST_SUITE_REGISTRATION( IteratableChecksumValidatorTest );

void IteratableChecksumValidatorTest::testValidate() {
  BitfieldMan bitfieldMan(100, 250);
  bitfieldMan.setAllBit();

  ChecksumHandle checksum = new Checksum("898a81b8e0181280ae2ee1b81e269196d91e869a", DIGEST_ALGO_SHA1);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  IteratableChecksumValidator validator;
  validator.setDiskWriter(diskWriter);
  validator.setBitfield(&bitfieldMan);
  validator.setChecksum(checksum);

  validator.init();
  while(!validator.finished()) {
    validator.validateChunk();
  }
  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());
}

void IteratableChecksumValidatorTest::testValidate2() {
  BitfieldMan bitfieldMan(100, 250);
  bitfieldMan.setAllBit();

  ChecksumHandle checksum = new Checksum("ffffffffffffffffffffffffffffffffffffffff", DIGEST_ALGO_SHA1);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  IteratableChecksumValidator validator;
  validator.setDiskWriter(diskWriter);
  validator.setBitfield(&bitfieldMan);
  validator.setChecksum(checksum);

  validator.init();
  while(!validator.finished()) {
    validator.validateChunk();
  }
  CPPUNIT_ASSERT(!bitfieldMan.isAllBitSet());
}
/*
void IteratableChecksumValidatorTest::testValidate3() {
  BitfieldMan bitfieldMan(50, 250);
  bitfieldMan.setAllBit();
  Strings checksums;
  checksums.push_back("898a81b8e0181280ae2ee1b81e269196d91e869a");

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  IteratableChecksumValidator validator;
  validator.setDiskWriter(diskWriter);

  validator.validate(&bitfieldMan, checksums, 250);

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[0] = "ffffffffffffffffffffffffffffffffffffffff";

  validator.validate(&bitfieldMan, checksums, 250);

  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(2));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(3));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(4));
}

void IteratableChecksumValidatorTest::testValidate4() {
  BitfieldMan bitfieldMan(70, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  IteratableChecksumValidator validator;
  validator.setDiskWriter(diskWriter);

  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";
  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(2));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(3));
}
*/
