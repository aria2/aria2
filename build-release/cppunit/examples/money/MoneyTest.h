// MoneyTest.h
#ifndef MONEYTEST_H
#define MONEYTEST_H

#include <cppunit/extensions/HelperMacros.h>

class MoneyTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( MoneyTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testEqual );
  CPPUNIT_TEST( testAdd );
  CPPUNIT_TEST_EXCEPTION( testAddThrow, IncompatibleMoneyError );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testConstructor();
  void testEqual();
  void testAdd();
  void testAddThrow();
};

#endif  // MONEYTEST_H
