#ifndef TESTASSERTTEST_H
#define TESTASSERTTEST_H

#include <cppunit/extensions/HelperMacros.h>


class TestAssertTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestAssertTest );
  CPPUNIT_TEST( testAssertThrow );
  CPPUNIT_TEST( testAssertNoThrow );
  CPPUNIT_TEST( testAssertAssertionFail );
  CPPUNIT_TEST( testAssertAssertionPass );
  CPPUNIT_TEST( testAssert );
  CPPUNIT_TEST( testAssertEqual );
  CPPUNIT_TEST( testAssertMessageTrue );
  CPPUNIT_TEST( testAssertMessageFalse );
  CPPUNIT_TEST( testAssertDoubleEquals );
  CPPUNIT_TEST( testAssertDoubleEqualsPrecision );
  CPPUNIT_TEST( testAssertDoubleNonFinite );
  CPPUNIT_TEST( testFail );
  CPPUNIT_TEST_SUITE_END();

public:
  TestAssertTest();

  virtual ~TestAssertTest();

  virtual void setUp();
  virtual void tearDown();

  void testAssertThrow();
  void testAssertNoThrow();
  void testAssertAssertionFail();
  void testAssertAssertionPass();

  void testBasicAssertions();

  void testAssert();
  
  void testAssertEqual();

  void testAssertMessageTrue();
  void testAssertMessageFalse();

  void testAssertDoubleEquals();
  void testAssertDoubleEqualsPrecision();
  void testAssertDoubleNonFinite();

  void testAssertLongEquals();
  void testAssertLongNotEquals();

  void testFail();

private:
  TestAssertTest( const TestAssertTest &copy );
  void operator =( const TestAssertTest &copy );

  void checkDoubleNotEquals( double expected, 
                             double actual, 
                             double delta );

  void checkMessageContains( CPPUNIT_NS::Exception *e,
                             std::string expectedMessage );

private:
};

#endif  // TESTASSERTTEST_H
