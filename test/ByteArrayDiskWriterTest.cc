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

  string msg1 = "Hello world!";
  bw.writeData(msg1.c_str(), msg1.size(), 0);
  
  char buf[100];
  int32_t c = bw.readData(buf, sizeof(buf), 0);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(msg1, string(buf));

  // second call
  memset(buf, '\0', sizeof(buf));

  c = bw.readData(buf, sizeof(buf), 0);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(msg1, string(buf));
}

void ByteArrayDiskWriterTest::testWriteAndRead2() {
  ByteArrayDiskWriter bw;

  string msg1 = "Hello world!";
  bw.writeData(msg1.c_str(), msg1.size(), 16);
  
  char buf[100];
  int32_t c = bw.readData(buf, sizeof(buf), 16);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(msg1, string(buf));

  // second call
  memset(buf, '\0', sizeof(buf));

  c = bw.readData(buf, sizeof(buf), 16);
  buf[c] = '\0';

  CPPUNIT_ASSERT_EQUAL(msg1, string(buf));
}
