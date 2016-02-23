#include "CoreSuite.h"
#include "TestTest.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestTest,
                                       coreSuiteName() );


TestTest::TestTest() : 
    CPPUNIT_NS::TestFixture()
{
}


TestTest::~TestTest()
{
}


void 
TestTest::setUp()
{
  m_suite = new CPPUNIT_NS::TestSuite( "suite" );
  m_test1 = new MockTestCase( "test1" );
  m_test2 = new MockTestCase( "test2" );
  m_suite->addTest( m_test1 );
  m_suite->addTest( m_test2 );
  
  m_path = new CPPUNIT_NS::TestPath();
}


void 
TestTest::tearDown()
{
  delete m_suite;
  delete m_path;
}


void 
TestTest::testFindTestPathPointerThis()
{
  CPPUNIT_ASSERT( m_test1->findTestPath( m_test1, *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test1 == m_path->getChildTest() );

  m_path->removeTests();

  CPPUNIT_ASSERT( m_suite->findTestPath( m_suite, *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_suite == m_path->getChildTest() );
}


void 
TestTest::testFindTestPathPointer()
{
  CPPUNIT_ASSERT( m_suite->findTestPath( m_test1, *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 2, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_suite == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test1 == m_path->getTestAt(1) );
}


void 
TestTest::testFindTestPathPointerFail()
{
  MockTestCase test( "test" );
  CPPUNIT_ASSERT( !m_suite->findTestPath( &test, *m_path ) );
  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestTest::testFindTestPathNameThis()
{
  CPPUNIT_ASSERT( m_test1->findTestPath( "test1", *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_test1 == m_path->getChildTest() );

  m_path->removeTests();

  CPPUNIT_ASSERT( m_suite->findTestPath( "suite", *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 1, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_suite == m_path->getChildTest() );
}


void 
TestTest::testFindTestPathName()
{
  CPPUNIT_ASSERT( m_suite->findTestPath( "test2", *m_path ) );
  CPPUNIT_ASSERT_EQUAL( 2, m_path->getTestCount() );
  CPPUNIT_ASSERT( m_suite == m_path->getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == m_path->getTestAt(1) );
}


void 
TestTest::testFindTestPathNameFail()
{
  CPPUNIT_ASSERT( !m_suite->findTestPath( "bad-test", *m_path ) );
  CPPUNIT_ASSERT( !m_path->isValid() );
}


void 
TestTest::testFindTest()
{
  CPPUNIT_ASSERT( m_test1 == m_suite->findTest( "test1" ) );
}


void 
TestTest::testFindTestThrow()
{
  m_suite->findTest( "no-name" );
}


void 
TestTest::testResolveTestPath()
{
  CPPUNIT_NS::TestPath path1 = m_suite->resolveTestPath( "suite");
  CPPUNIT_ASSERT_EQUAL( 1, path1.getTestCount() );
  CPPUNIT_ASSERT( m_suite == path1.getTestAt(0) );

  CPPUNIT_NS::TestPath path2 = m_suite->resolveTestPath( "suite/test2");
  CPPUNIT_ASSERT_EQUAL( 2, path2.getTestCount() );
  CPPUNIT_ASSERT( m_suite == path2.getTestAt(0) );
  CPPUNIT_ASSERT( m_test2 == path2.getTestAt(1) );
}
