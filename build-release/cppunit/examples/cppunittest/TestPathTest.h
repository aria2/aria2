#ifndef TESTPATHTEST_H
#define TESTPATHTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestCase.h>
#include <stdexcept>


/*! \class TestPathTest
 * \brief Unit tests for class TestPath.
 */
class TestPathTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestPathTest );
  CPPUNIT_TEST( testDefaultConstructor );
  CPPUNIT_TEST( testAddTest );
  CPPUNIT_TEST_EXCEPTION( testGetTestAtThrow1, std::out_of_range );
  CPPUNIT_TEST_EXCEPTION( testGetTestAtThrow2, std::out_of_range );
  CPPUNIT_TEST( testGetChildTest );
  CPPUNIT_TEST( testGetChildTestManyTests );
  CPPUNIT_TEST_EXCEPTION( testGetChildTestThrowIfNotValid, std::out_of_range );
  CPPUNIT_TEST( testAddPath );
  CPPUNIT_TEST( testAddInvalidPath );
  CPPUNIT_TEST( testRemoveTests );
  CPPUNIT_TEST( testRemoveTest );
  CPPUNIT_TEST_EXCEPTION( testRemoveTestThrow1, std::out_of_range );
  CPPUNIT_TEST_EXCEPTION( testRemoveTestThrow2, std::out_of_range );
  CPPUNIT_TEST( testUp );
  CPPUNIT_TEST_EXCEPTION( testUpThrow, std::out_of_range );
  CPPUNIT_TEST( testInsert );
  CPPUNIT_TEST( testInsertAtEnd );
  CPPUNIT_TEST_EXCEPTION( testInsertThrow1, std::out_of_range );
  CPPUNIT_TEST_EXCEPTION( testInsertThrow2, std::out_of_range );
  CPPUNIT_TEST( testInsertPath );
  CPPUNIT_TEST_EXCEPTION( testInsertPathThrow, std::out_of_range );
  CPPUNIT_TEST( testInsertPathDontThrowIfInvalid );
  CPPUNIT_TEST( testRootConstructor );
  CPPUNIT_TEST( testPathSliceConstructorCopyUntilEnd );
  CPPUNIT_TEST( testPathSliceConstructorCopySpecifiedCount );
  CPPUNIT_TEST( testPathSliceConstructorCopyNone );
  CPPUNIT_TEST( testPathSliceConstructorNegativeIndex );
  CPPUNIT_TEST( testPathSliceConstructorAfterEndIndex );
  CPPUNIT_TEST( testPathSliceConstructorNegativeIndexUntilEnd );
  CPPUNIT_TEST( testPathSliceConstructorNegativeIndexNone );
  CPPUNIT_TEST( testToStringNoTest );
  CPPUNIT_TEST( testToStringOneTest );
  CPPUNIT_TEST( testToStringHierarchy );
  CPPUNIT_TEST( testPathStringConstructorRoot );
  CPPUNIT_TEST( testPathStringConstructorEmptyIsRoot );
  CPPUNIT_TEST( testPathStringConstructorHierarchy );
  CPPUNIT_TEST_EXCEPTION( testPathStringConstructorBadRootThrow, std::invalid_argument );
  CPPUNIT_TEST( testPathStringConstructorRelativeRoot );
  CPPUNIT_TEST( testPathStringConstructorRelativeRoot2 );
  CPPUNIT_TEST_EXCEPTION( testPathStringConstructorThrow1, std::invalid_argument );
  CPPUNIT_TEST( testPathStringConstructorRelativeHierarchy );
  CPPUNIT_TEST_EXCEPTION( testPathStringConstructorBadRelativeHierarchyThrow, std::invalid_argument );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a TestPathTest object.
   */
  TestPathTest();

  /// Destructor.
  virtual ~TestPathTest();

  void setUp();
  void tearDown();

  void testDefaultConstructor();
  void testAddTest();
  void testGetTestAtThrow1();
  void testGetTestAtThrow2();
  void testGetChildTest();
  void testGetChildTestManyTests();
  void testGetChildTestThrowIfNotValid();

  void testAddPath();
  void testAddInvalidPath();

  void testRemoveTests();
  void testRemoveTest();
  void testRemoveTestThrow1();
  void testRemoveTestThrow2();
  void testUp();
  void testUpThrow();

  void testInsert();
  void testInsertAtEnd();
  void testInsertThrow1();
  void testInsertThrow2();

  void testInsertPath();
  void testInsertPathThrow();
  void testInsertPathDontThrowIfInvalid();

  void testRootConstructor();
  void testPathSliceConstructorCopyUntilEnd();
  void testPathSliceConstructorCopySpecifiedCount();
  void testPathSliceConstructorCopyNone();
  void testPathSliceConstructorNegativeIndex();
  void testPathSliceConstructorAfterEndIndex();
  void testPathSliceConstructorNegativeIndexUntilEnd();
  void testPathSliceConstructorNegativeIndexNone();

  void testToStringNoTest();
  void testToStringOneTest();
  void testToStringHierarchy();

  void testPathStringConstructorRoot();
  void testPathStringConstructorEmptyIsRoot();
  void testPathStringConstructorHierarchy();
  void testPathStringConstructorBadRootThrow();
  void testPathStringConstructorRelativeRoot();
  void testPathStringConstructorRelativeRoot2();
  void testPathStringConstructorThrow1();
  void testPathStringConstructorRelativeHierarchy();
  void testPathStringConstructorBadRelativeHierarchyThrow();

private:
  /// Prevents the use of the copy constructor.
  TestPathTest( const TestPathTest &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestPathTest &copy );

private:
  CPPUNIT_NS::TestPath *m_path;
  CPPUNIT_NS::TestCase *m_test1;
  CPPUNIT_NS::TestCase *m_test2;
  CPPUNIT_NS::TestCase *m_test3;
  CPPUNIT_NS::TestCase *m_test4;
  CPPUNIT_NS::TestSuite *m_suite1;
  CPPUNIT_NS::TestSuite *m_suite2;
  CPPUNIT_NS::TestCase *m_testSuite2a;
  CPPUNIT_NS::TestCase *m_testSuite2b;
};



#endif  // TESTPATHTEST_H
