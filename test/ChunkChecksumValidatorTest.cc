#include "ChunkChecksumValidator.h"
#include "DefaultDiskWriter.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class ChunkChecksumValidatorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ChunkChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate2);
  CPPUNIT_TEST(testValidate3);
  CPPUNIT_TEST(testValidate4);
  CPPUNIT_TEST_SUITE_END();
private:

  static const char* csArray[];// = { "29b0e7878271645fffb7eec7db4a7473a1c00bc1",
  //  "4df75a661cb7eb2733d9cdaa7f772eae3a4e2976",
  //			   "0a4ea2f7dd7c52ddf2099a444ab2184b4d341bdb" };
public:
  void setUp() {
  }

  void testValidate();
  void testValidate2();
  void testValidate3();
  void testValidate4();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ChunkChecksumValidatorTest );

const char* ChunkChecksumValidatorTest::csArray[] = { "29b0e7878271645fffb7eec7db4a7473a1c00bc1",
						      "4df75a661cb7eb2733d9cdaa7f772eae3a4e2976",
						      "0a4ea2f7dd7c52ddf2099a444ab2184b4d341bdb" };

void ChunkChecksumValidatorTest::testValidate() {
  BitfieldMan bitfieldMan(100, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  ChunkChecksumValidator validator;
  validator.setDiskWriter(diskWriter);

  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";

  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(2));
}

void ChunkChecksumValidatorTest::testValidate2() {
  BitfieldMan bitfieldMan(50, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  ChunkChecksumValidator validator;
  validator.setDiskWriter(diskWriter);

  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";
  validator.validate(&bitfieldMan, checksums, 100);

  CPPUNIT_ASSERT(bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(2));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(3));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(4));
}

void ChunkChecksumValidatorTest::testValidate3() {
  BitfieldMan bitfieldMan(50, 250);
  bitfieldMan.setAllBit();
  Strings checksums;
  checksums.push_back("898a81b8e0181280ae2ee1b81e269196d91e869a");

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  ChunkChecksumValidator validator;
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

void ChunkChecksumValidatorTest::testValidate4() {
  BitfieldMan bitfieldMan(70, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  DefaultDiskWriterHandle diskWriter = new DefaultDiskWriter();
  diskWriter->openExistingFile("chunkChecksumTestFile250.txt");

  ChunkChecksumValidator validator;
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
