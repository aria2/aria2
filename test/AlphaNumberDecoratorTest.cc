#include "AlphaNumberDecorator.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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
  CPPUNIT_ASSERT_EQUAL(std::string("a"), AlphaNumberDecorator(1).decorate(0));
  CPPUNIT_ASSERT_EQUAL(std::string("z"), AlphaNumberDecorator(1).decorate(25));
  CPPUNIT_ASSERT_EQUAL(std::string("zz"), AlphaNumberDecorator(1).decorate(675)); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(std::string("aab"), AlphaNumberDecorator(3).decorate(1));
}

void AlphaNumberDecoratorTest::testDecorate_uppercase()
{
  CPPUNIT_ASSERT_EQUAL(std::string("A"), AlphaNumberDecorator(1, true).decorate(0));
  CPPUNIT_ASSERT_EQUAL(std::string("Z"), AlphaNumberDecorator(1, true).decorate(25));
  CPPUNIT_ASSERT_EQUAL(std::string("ZZ"), AlphaNumberDecorator(1, true).decorate(675)); // 25*26+25
  CPPUNIT_ASSERT_EQUAL(std::string("AAB"), AlphaNumberDecorator(3, true).decorate(1));
}

void AlphaNumberDecoratorTest::testDecorate_minus()
{
  CPPUNIT_ASSERT_EQUAL(std::string("NXMRLXV"), AlphaNumberDecorator(1, true).decorate(-1));
}

} // namespace aria2
