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

std::stringhexHash(unsigned char* md, int len) {
  char* temp = new char[len*2+1];
  for(int i = 0; i < len; i++) {
    sprintf(temp+i*2, "%02x", md[i]);
  }
  temp[len*2] = '\0';
  std::stringh(temp);
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
  std::stringhashHex = hexHash(md, len);
  CPPUNIT_ASSERT_EQUAL(std::string("20482dadd856f5ac908848f731d9235d2891c41e"),
		       hashHex);
}

void ShaVisitorTest::testVisitCompound() {
  ShaVisitor v;
  std::string data = "d4:name5:aria24:listli123eee";
  MetaEntry* e = MetaFileUtil::bdecoding(data.c_str(), data.size());
  e->accept(&v);
  unsigned char md[20];
  int len = 0;
  v.getHash(md, len);
  std::stringhashHex = hexHash(md, len);
  CPPUNIT_ASSERT_EQUAL(std::string("75538fbac9a074bb98c6a19b6bca3bc87ef9bf8e"),
		       hashHex);
}
