#ifndef TESTSUITETEST_H
#define TESTSUITETEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <stdexcept>


class TestSuiteTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestSuiteTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testCountTestCasesWithNoTest );
  CPPUNIT_TEST( testCountTestCasesWithTwoTests );
  CPPUNIT_TEST( testCountTestCasesWithSubSuite );
  CPPUNIT_TEST( testRunWithOneTest );
  CPPUNIT_TEST( testRunWithOneTestAndSubSuite );
  CPPUNIT_TEST( testGetTests );
  CPPUNIT_TEST( testDeleteContents );
  CPPUNIT_TEST( testGetChildTestCount );
  CPPUNIT_TEST( testGetChildTestAt );
  CPPUNIT_TEST_EXCEPTION( testGetChildTestAtThrow1, std::out_of_range );
  CPPUNIT_TEST_EXCEPTION( testGetChildTestAtThrow2, std::out_of_range );
  CPPUNIT_TEST_SUITE_END();

public:
  TestSuiteTest();
  virtual ~TestSuiteTest();

  virtual void setUp();
  virtual void tearDown();

  void testConstructor();

  void testCountTestCasesWithNoTest();
  void testCountTestCasesWithTwoTests();
  void testCountTestCasesWithSubSuite();

  void testRunWithOneTest();
  void testRunWithOneTestAndSubSuite();

  void testGetTests();

  void testDeleteContents();

  void testGetChildTestCount();
  void testGetChildTestAt();
  void testGetChildTestAtThrow1();
  void testGetChildTestAtThrow2();

private:
  TestSuiteTest( const TestSuiteTest &copy );
  void operator =( const TestSuiteTest &copy );

private:
  CPPUNIT_NS::TestSuite *m_suite;
};



#endif  // TESTSUITETEST_H
