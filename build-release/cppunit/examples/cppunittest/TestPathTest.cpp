#include "CoreSuite.h"
#include "TestPathTest.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestPathTest,
                                       coreSuiteName() );


TestPathTest::TestPathTest()
{
}


TestPathTest::~TestPathTest()
{
}


void 
TestPathTest::setUp()
{
  m_path = new CPPUNIT_NS::TestPath();
  m_test1 = new CPPUNIT_NS::TestCase( "test1" );
  m_test2 = new CPPUNIT_NS::TestCase( "test2" );
  m_test3 = new CPPUNIT_NS::TestCase( "test3" );
  m_test4 = new CPPUNIT_NS::TestCase( "test4" );

  m_suite1 = new CPPUNIT_NS::TestSuite( "All Tests" );
  m_suite2 = new CPPUNIT_NS::TestSuite( "Custom" );
  m_testSuite2a =  new CPPUNIT_NS::TestCase( "MyTest::testDefaultConstructor" );
  m_testSuite2b =  new CPPUNIT_NS::TestCase( "MyTest::testConstructor" );
  m_suite2->addTest( m_testSuite2a );
  m_suite2->addTest( m_testSuite2b );
  m_suite1->addTest( m_suite2 );
}


void 
TestPathTest::tearDown()
{
  delete m_suite1;
  delete m_path;
  delete m_test4;
  delete m_test3;
  delete m_test2;
  delete m_test1;
}


void 
TestPathTest::testDefaultConstructor()
{
  CPPUNIT_ASSERT_EQUAL( 0, m_path->getTestCount() );
  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestPathTest::testAddTest()
{
  m_path->add( m_test1 );
  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_path->isValid() );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(0) );
}


void 
TestPathTest::testGetTestAtThrow1()
{
  m_path->getTestAt( 0 );
}


void 
TestPathTest::testGetTestAtThrow2()
{
  m_path->add( m_test1 );
  m_path->getTestAt( 1 );
}


void 
TestPathTest::testGetChildTest()
{
  m_path->add( m_test1 );
  CPPUNIT_ASSERT( m_test1 == m_path->getChildTest() );
}


void 
TestPathTest::testGetChildTestManyTests()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );
  m_path->add( m_test3 );
  CPPUNIT_ASSERT( m_test3 == m_path->getChildTest() );
}


void 
TestPathTest::testGetChildTestThrowIfNotValid()
{
  m_path->getChildTest();
}


void 
TestPathTest::testAddPath()
{
  CPPUNIT_NS::TestPath path;
  path.add( m_test2 );
  path.add( m_test3 );

  m_path->add( m_test1 );
  m_path->add( path );

  CPPUNIT_ASSERT_EQUAL( 3, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(1) );
  CPPUNIT_ASSERT( m_test3 == m_path->getTestAt(2) );
}


void 
TestPathTest::testAddInvalidPath()
{
  CPPUNIT_NS::TestPath path;
  m_path->add( path );

  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestPathTest::testRemoveTests()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  m_path->removeTests();

  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestPathTest::testRemoveTest()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  m_path->removeTest( 0 );

  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(0) );
}


void 
TestPathTest::testRemoveTestThrow1()
{
  m_path->removeTest( -1 );
}


void 
TestPathTest::testRemoveTestThrow2()
{
  m_path->add( m_test1 );

  m_path->removeTest( 1 );
}


void 
TestPathTest::testUp()
{
  m_path->add( m_test1 );

  m_path->up();

  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestPathTest::testUpThrow()
{
  m_path->up();
}


void 
TestPathTest::testInsert()
{
  m_path->add( m_test1 );

  m_path->insert( m_test2, 0 );

  CPPUNIT_ASSERT_EQUAL( 2, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(1) );
}


void 
TestPathTest::testInsertAtEnd()
{
  m_path->add( m_test1 );

  m_path->insert( m_test2, 1 );

  CPPUNIT_ASSERT_EQUAL( 2, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(1) );
}


void 
TestPathTest::testInsertThrow1()
{
  m_path->insert( m_test1, -1 );
}


void 
TestPathTest::testInsertThrow2()
{
  m_path->add( m_test1 );

  m_path->insert( m_test1, 2 );
}


void 
TestPathTest::testInsertPath()
{
  CPPUNIT_NS::TestPath path;
  path.add( m_test2 );
  path.add( m_test3 );

  m_path->add( m_test1 );
  m_path->add( m_test4 );
  m_path->insert( path, 1 );

  CPPUNIT_ASSERT_EQUAL( 4, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(1) );
  CPPUNIT_ASSERT( m_test3 == m_path->getTestAt(2) );
  CPPUNIT_ASSERT( m_test4 == m_path->getTestAt(3) );
}


void 
TestPathTest::testInsertPathThrow()
{
  CPPUNIT_NS::TestPath path;
  path.add( m_test2 );

  m_path->insert( path, 1 );
}


void 
TestPathTest::testInsertPathDontThrowIfInvalid()
{
  CPPUNIT_NS::TestPath path;
  m_path->insert( path, 1 );
}


void 
TestPathTest::testRootConstructor()
{
  CPPUNIT_NS::TestPath path( m_test1 );
  CPPUNIT_ASSERT( path.isValid() );
  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_test1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathSliceConstructorCopyUntilEnd()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );
  m_path->add( m_test3 );
  
  CPPUNIT_NS::TestPath path( *m_path, 1 );

  CPPUNIT_ASSERT_EQUAL( 2, path.getTestCount() );
  CPPUNIT_ASSERT( m_test2 == path.getTestAt(0) );
  CPPUNIT_ASSERT( m_test3 == path.getTestAt(1) );
}


void 
TestPathTest::testPathSliceConstructorCopySpecifiedCount()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );
  m_path->add( m_test3 );
  
  CPPUNIT_NS::TestPath path( *m_path, 0, 1 );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_test1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathSliceConstructorCopyNone()
{
  m_path->add( m_test1 );
  
  CPPUNIT_NS::TestPath path( *m_path, 0, 0 );
  CPPUNIT_ASSERT_EQUAL( 0, path.getTestCount() );
}


void 
TestPathTest::testPathSliceConstructorNegativeIndex()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  CPPUNIT_NS::TestPath path( *m_path, -1, 2 );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_test1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathSliceConstructorAfterEndIndex()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  CPPUNIT_NS::TestPath path( *m_path, 2, 5 );

  CPPUNIT_ASSERT_EQUAL( 0, path.getTestCount() );
}


void 
TestPathTest::testPathSliceConstructorNegativeIndexUntilEnd()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  CPPUNIT_NS::TestPath path( *m_path, -1 );

  CPPUNIT_ASSERT_EQUAL( 2, path.getTestCount() );
  CPPUNIT_ASSERT( m_test1 == path.getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == path.getTestAt(1) );
}


void 
TestPathTest::testPathSliceConstructorNegativeIndexNone()
{
  m_path->add( m_test1 );
  m_path->add( m_test2 );

  CPPUNIT_NS::TestPath path( *m_path, -2, 1 );

  CPPUNIT_ASSERT_EQUAL( 0, path.getTestCount() );
}


void 
TestPathTest::testToStringNoTest()
{
  std::string expected = "/";
  std::string actual = m_path->toString();

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}


void 
TestPathTest::testToStringOneTest()
{
  m_path->add( m_test1 );

  std::string expected = "/test1";
  std::string actual = m_path->toString();

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}


void 
TestPathTest::testToStringHierarchy()
{
  m_path->add( m_suite1 );
  m_path->add( m_suite2 );
  m_path->add( m_suite2->getChildTestAt(0) );

  std::string expected = "/All Tests/Custom/MyTest::testDefaultConstructor";
  std::string actual = m_path->toString();

  CPPUNIT_ASSERT_EQUAL( expected, actual );
}


void 
TestPathTest::testPathStringConstructorRoot()
{
  CPPUNIT_NS::TestPath path( m_suite1, "/All Tests" );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathStringConstructorEmptyIsRoot()
{
  CPPUNIT_NS::TestPath path( m_suite1, "" );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathStringConstructorHierarchy()
{
  CPPUNIT_NS::TestPath path( m_suite1, "/All Tests/Custom/MyTest::testDefaultConstructor" );

  CPPUNIT_ASSERT_EQUAL( 3, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite1 == path.getTestAt(0) );
  CPPUNIT_ASSERT( m_suite2 == path.getTestAt(1) );
  CPPUNIT_ASSERT( m_testSuite2a == path.getTestAt(2) );
}


void 
TestPathTest::testPathStringConstructorBadRootThrow()
{
  CPPUNIT_NS::TestPath path( m_suite1, "/Custom" );
}


void 
TestPathTest::testPathStringConstructorRelativeRoot()
{
  CPPUNIT_NS::TestPath path( m_suite1, "All Tests" );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite1 == path.getTestAt(0) );
}


void 
TestPathTest::testPathStringConstructorRelativeRoot2()
{
  CPPUNIT_NS::TestPath path( m_suite1, "Custom" );

  CPPUNIT_ASSERT_EQUAL( 1, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite2 == path.getTestAt(0) );
}


void 
TestPathTest::testPathStringConstructorThrow1()
{
  CPPUNIT_NS::TestPath path( m_suite1, "/" );
}


void 
TestPathTest::testPathStringConstructorRelativeHierarchy()
{
  CPPUNIT_NS::TestPath path( m_suite1, "Custom/MyTest::testConstructor" );

  CPPUNIT_ASSERT_EQUAL( 2, path.getTestCount() );
  CPPUNIT_ASSERT( m_suite2 == path.getTestAt(0) );
  CPPUNIT_ASSERT( m_testSuite2b == path.getTestAt(1) );
}


void 
TestPathTest::testPathStringConstructorBadRelativeHierarchyThrow()
{
  CPPUNIT_NS::TestPath path( m_suite1, "Custom/MyBadTest::testConstructor" );
}
