#include "ByteArrayDiskWriter.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class ByteArrayDiskWriterTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ByteArrayDiskWriterTest);
  CPPUNIT_TEST(testWriteAndRead);
  CPPUNIT_TEST(testWriteAndRead2);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testWriteAndRead();
  void testWriteAndRead2();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ByteArrayDiskWriterTest );

void ByteArrayDiskWriterTest::testWriteAndRead() {
  ByteArrayDiskWriter bw;

  string msg1 = "Hello";
  bw.writeData((const unsigned char*)msg1.c_str(), msg1.size(), 0);
  // write at the end of stream
  string msg2 = " World";
  bw.writeData((const unsigned char*)msg2.c_str(), msg2.size(), 5);
  // write at the end of stream +1
  string msg3 = "!!";
  bw.writeData((const unsigned char*)msg3.c_str(), msg3.size(), 12);
  // write space at the 'hole'
  string msg4 = " ";
  bw.writeData((const unsigned char*)msg4.c_str(), msg4.size(), 11);

  char buf[100];
  int32_t c = bw.readData((unsigned char*)buf, sizeof(buf), 1);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(string("ello World !!"), string(buf));
}

void ByteArrayDiskWriterTest::testWriteAndRead2() {
  ByteArrayDiskWriter bw;

  string msg1 = "Hello World";
  bw.writeData((const unsigned char*)msg1.c_str(), msg1.size(), 0);
  string msg2 = "From Mars";
  bw.writeData((const unsigned char*)msg2.c_str(), msg2.size(), 6);

  char buf[100];
  int32_t c = bw.readData((unsigned char*)buf, sizeof(buf), 0);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(string("Hello From Mars"), string(buf));
}
