#include "MetaFileUtil.h"
#include "Data.h"
#include "Dictionary.h"
#include "List.h"
#include "DlAbortEx.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class MetaFileUtilTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MetaFileUtilTest);
  CPPUNIT_TEST(testParseMetaFile);
  CPPUNIT_TEST(testBdecoding);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testParseMetaFile();
  void testBdecoding();
};


CPPUNIT_TEST_SUITE_REGISTRATION( MetaFileUtilTest );

void MetaFileUtilTest::testParseMetaFile() {
  MetaEntry* entry = MetaFileUtil::parseMetaFile("test.torrent");
  Dictionary* d = dynamic_cast<Dictionary*>(entry);
  CPPUNIT_ASSERT(d != NULL);
}

void MetaFileUtilTest::testBdecoding() {
  try {
    const char* str = "5:abcd";
    MetaFileUtil::bdecoding(str, strlen(str));
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  } catch(DlAbortEx* ex) {
    delete ex;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  }

  try {
    const char* str = "i1234";
    MetaFileUtil::bdecoding(str, strlen(str));
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  } catch(DlAbortEx* ex) {
    delete ex;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  }

  try {
    const char* str = "5abcd";
    MetaFileUtil::bdecoding(str, strlen(str));
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  } catch(DlAbortEx* ex) {
    delete ex;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  }

  try {
    const char* str = "d";
    MetaFileUtil::bdecoding(str, strlen(str));
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  } catch(DlAbortEx* ex) {
    delete ex;
  } catch(...) {
    CPPUNIT_FAIL("DlAbortEx exception must be thrown.");
  }
}
    
