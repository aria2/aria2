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


  ChunkChecksumValidatorHandle
  createChunkChecksumValidator(const string& filename, BitfieldMan* bitfield, const ChunkChecksumHandle& chunkChecksum)
  {
    IteratableChunkChecksumValidatorHandle iv = new IteratableChunkChecksumValidator();
    DefaultDiskWriterHandle dw = new DefaultDiskWriter();
    dw->openExistingFile(filename);
    iv->setDiskWriter(dw);
    iv->setBitfield(bitfield);
    iv->setChunkChecksum(chunkChecksum);

    return new ChunkChecksumValidator(iv);
  } 

  void testValidate();
  void testValidate2();
  void testValidate3();
  void testValidate4();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ChunkChecksumValidatorTest );

const char* ChunkChecksumValidatorTest::csArray[] =
  { "29b0e7878271645fffb7eec7db4a7473a1c00bc1",
    "4df75a661cb7eb2733d9cdaa7f772eae3a4e2976",
    "0a4ea2f7dd7c52ddf2099a444ab2184b4d341bdb" };

void ChunkChecksumValidatorTest::testValidate() {
  BitfieldMan bitfieldMan(100, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  ChunkChecksumValidatorHandle validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));
  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";

  validator = 
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));

  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(2));
}

void ChunkChecksumValidatorTest::testValidate2() {
  BitfieldMan bitfieldMan(50, 250);
  bitfieldMan.setAllBit();
  Strings checksums(&csArray[0], &csArray[3]);

  ChunkChecksumValidatorHandle validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));
  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";
  validator = 
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));
  validator->validate();

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

  ChunkChecksumValidatorHandle validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 250));
  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[0] = "ffffffffffffffffffffffffffffffffffffffff";

  validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 250));
  validator->validate();

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

  ChunkChecksumValidatorHandle validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));
  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isAllBitSet());

  checksums[1] = "ffffffffffffffffffffffffffffffffffffffff";

  validator =
    createChunkChecksumValidator("chunkChecksumTestFile250.txt",
				 &bitfieldMan, new ChunkChecksum("sha1",
								 checksums,
								 100));

  validator->validate();

  CPPUNIT_ASSERT(bitfieldMan.isBitSet(0));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(1));
  CPPUNIT_ASSERT(!bitfieldMan.isBitSet(2));
  CPPUNIT_ASSERT(bitfieldMan.isBitSet(3));
}

