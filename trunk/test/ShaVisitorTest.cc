#include "ShaVisitor.h"
#include "MetaFileUtil.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class ShaVisitorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ShaVisitorTest);
  CPPUNIT_TEST(testVisit);
  CPPUNIT_TEST(testVisitCompound);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testVisit();
  void testVisitCompound();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ShaVisitorTest );

string hexHash(unsigned char* md, int len) {
  char* temp = new char[len*2+1];
  for(int i = 0; i < len; i++) {
    sprintf(temp+i*2, "%02x", md[i]);
  }
  temp[len*2] = '\0';
  string h(temp);
  delete [] temp;
  return h;
}

void ShaVisitorTest::testVisit() {
  ShaVisitor v;
  Data d("test", 4);
  d.accept(&v);
  unsigned char md[20];
  int len = 0;
  v.getHash(md, len);
  string hashHex = hexHash(md, len);
  CPPUNIT_ASSERT_EQUAL(string("20482dadd856f5ac908848f731d9235d2891c41e"),
		       hashHex);
}

void ShaVisitorTest::testVisitCompound() {
  ShaVisitor v;
  MetaEntry* e = MetaFileUtil::parseMetaFile("test.torrent");
  e->accept(&v);
  unsigned char md[20];
  int len = 0;
  v.getHash(md, len);
  string hashHex = hexHash(md, len);
  CPPUNIT_ASSERT_EQUAL(string("9d33ba293924df85f6067a81c65b484de04e8efd"),
		       hashHex);
}
