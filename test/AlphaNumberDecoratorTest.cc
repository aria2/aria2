#include "AlphaNumberDecorator.h"
#include <cppunit/extensions/HelperMacros.h>

class AlphaNumberDecoratorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AlphaNumberDecoratorTest);
  CPPUNIT_TEST(testDecorate);
  CPPUNIT_TEST(testDecorate_uppercase);
  CPPUNIT_TEST(testDecorate_minus);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testDecorate();
  void testDecorate_uppercase();
  void testDecorate_minus();
};


CPPUNIT_TEST_SUITE_REGISTRATION( AlphaNumberDecoratorTest );

void AlphaNumberDecoratorTest::testDecorate()
{
  CPPUNIT_ASSERT_EQUAL(string("a"), AlphaNumberDecorator(1).decorate(0));
  CPPUNIT_ASSERT_EQUAL(string("z"), AlphaNumberDecorator(1).decorate(25));
  CPPUNIT_ASSERT_EQUAL(string("zz"), AlphaNumberDecorator(1).decorate(675)); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(string("aab"), AlphaNumberDecorator(3).decorate(1));
}

void AlphaNumberDecoratorTest::testDecorate_uppercase()
{
  CPPUNIT_ASSERT_EQUAL(string("A"), AlphaNumberDecorator(1, true).decorate(0));
  CPPUNIT_ASSERT_EQUAL(string("Z"), AlphaNumberDecorator(1, true).decorate(25));
  CPPUNIT_ASSERT_EQUAL(string("ZZ"), AlphaNumberDecorator(1, true).decorate(675)); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(string("AAB"), AlphaNumberDecorator(3, true).decorate(1));
}

void AlphaNumberDecoratorTest::testDecorate_minus()
{
  try {
    AlphaNumberDecorator(1, true).decorate(-1);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(FatalException* e) {
    cerr << e->getMsg() << endl;
    delete e;
  } catch(...) {
    CPPUNIT_FAIL("FatalException must be thrown.");
  }
}
