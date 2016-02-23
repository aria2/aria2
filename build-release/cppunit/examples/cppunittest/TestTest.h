#ifndef TESTTEST_H
#define TESTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestPath.h>
#include "MockTestCase.h"
#include <stdexcept>


/*! \class TestTest
 * \brief Unit test for class Test.
 */
class TestTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestTest );
  CPPUNIT_TEST( testFindTestPathPointerThis );
  CPPUNIT_TEST( testFindTestPathPointer );
  CPPUNIT_TEST( testFindTestPathPointerFail );
  CPPUNIT_TEST( testFindTestPathNameThis );
  CPPUNIT_TEST( testFindTestPathName );
  CPPUNIT_TEST( testFindTestPathNameFail );
  CPPUNIT_TEST( testFindTest );
  CPPUNIT_TEST_EXCEPTION( testFindTestThrow, std::invalid_argument );
  CPPUNIT_TEST( testResolveTestPath );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a TestTest object.
   */
  TestTest();

  /// Destructor.
  virtual ~TestTest();

  void setUp();
  void tearDown();

  void testFindTestPathPointerThis();
  void testFindTestPathPointer();
  void testFindTestPathPointerFail();

  void testFindTestPathNameThis();
  void testFindTestPathName();
  void testFindTestPathNameFail();

  void testFindTest();
  void testFindTestThrow();

  void testResolveTestPath();

private:
  /// Prevents the use of the copy constructor.
  TestTest( const TestTest &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestTest &copy );

private:
  CPPUNIT_NS::TestSuite *m_suite;
  MockTestCase *m_test1;
  MockTestCase *m_test2;
  CPPUNIT_NS::TestPath *m_path;
};


#endif  // TESTTEST_H
