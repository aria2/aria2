#include "CoreSuite.h"
#include "TestSuiteTest.h"
#include <cppunit/TestResult.h>
#include "MockTestCase.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestSuiteTest,
                                       coreSuiteName() );


TestSuiteTest::TestSuiteTest()
{
}


TestSuiteTest::~TestSuiteTest()
{
}


void 
TestSuiteTest::setUp()
{
  m_suite = new CPPUNIT_NS::TestSuite();
}


void 
TestSuiteTest::tearDown()
{
  delete m_suite;
}


void 
TestSuiteTest::testConstructor()
{
  std::string name( "MySuite" );
  CPPUNIT_NS::TestSuite suite( name );
  CPPUNIT_ASSERT_EQUAL( name, suite.getName() );
}


void 
TestSuiteTest::testCountTestCasesWithNoTest()
{
  CPPUNIT_ASSERT_EQUAL( 0, m_suite->countTestCases() );
}


void 
TestSuiteTest::testCountTestCasesWithTwoTests()
{
  MockTestCase *case1 = new MockTestCase( "test1" );
  case1->setExpectedCountTestCasesCall();
  MockTestCase *case2 = new MockTestCase( "test2" );
  case2->setExpectedCountTestCasesCall();
  m_suite->addTest( case1 );
  m_suite->addTest( case2 );

  CPPUNIT_ASSERT_EQUAL( 2, m_suite->countTestCases() );
  case1->verify();
  case2->verify();
}


void 
TestSuiteTest::testCountTestCasesWithSubSuite()
{
  MockTestCase *case1 = new MockTestCase( "test1" );
  case1->setExpectedCountTestCasesCall();
  MockTestCase *case2 = new MockTestCase( "test2" );
  case2->setExpectedCountTestCasesCall();
  MockTestCase *case3 = new MockTestCase( "test3" );
  case3->setExpectedCountTestCasesCall();
  CPPUNIT_NS::TestSuite *subSuite = new CPPUNIT_NS::TestSuite( "SubSuite");
  subSuite->addTest( case1 );
  subSuite->addTest( case2 );
  m_suite->addTest( case3 );
  m_suite->addTest( subSuite );

  CPPUNIT_ASSERT_EQUAL( 3, m_suite->countTestCases() );
  case1->verify();
  case2->verify();
  case3->verify();
}


void 
TestSuiteTest::testRunWithOneTest()
{
  MockTestCase *case1 = new MockTestCase( "test1" );
  case1->setExpectedRunTestCall();
  m_suite->addTest( case1 );

  CPPUNIT_NS::TestResult result;
  m_suite->run( &result );

  case1->verify();
}


void 
TestSuiteTest::testRunWithOneTestAndSubSuite()
{
  MockTestCase *case1 = new MockTestCase( "test1" );
  case1->setExpectedRunTestCall();
  MockTestCase *case2 = new MockTestCase( "test2" );
  case2->setExpectedRunTestCall();
  MockTestCase *case3 = new MockTestCase( "test3" );
  case3->setExpectedRunTestCall();
  CPPUNIT_NS::TestSuite *subSuite = new CPPUNIT_NS::TestSuite( "SubSuite");
  subSuite->addTest( case1 );
  subSuite->addTest( case2 );
  m_suite->addTest( case3 );
  m_suite->addTest( subSuite);

  CPPUNIT_NS::TestResult result;
  m_suite->run( &result );

  case1->verify();
  case2->verify();
  case3->verify();
}


void 
TestSuiteTest::testGetTests()
{
  m_suite->addTest( new CPPUNIT_NS::TestCase( "test1" ) );
  m_suite->addTest( new CPPUNIT_NS::TestCase( "test2" ) );
  CPPUNIT_ASSERT_EQUAL( 2, int(m_suite->getTests().size()) );
}


void 
TestSuiteTest::testDeleteContents()
{
  m_suite->addTest( new CPPUNIT_NS::TestCase( "test2" ) );
  m_suite->deleteContents();
  CPPUNIT_ASSERT_EQUAL( 0, int(m_suite->getTests().size()) );
}


void 
TestSuiteTest::testGetChildTestCount()
{
  m_suite->addTest( new CPPUNIT_NS::TestCase( "test1" ) );
  m_suite->addTest( new CPPUNIT_NS::TestCase( "test2" ) );

  CPPUNIT_ASSERT_EQUAL( 2, m_suite->getChildTestCount() );
}


void 
TestSuiteTest::testGetChildTestAt()
{
  CPPUNIT_NS::TestCase *test1 = new CPPUNIT_NS::TestCase( "test1" );
  CPPUNIT_NS::TestCase *test2 = new CPPUNIT_NS::TestCase( "test2" );
  m_suite->addTest( test1 );
  m_suite->addTest( test2 );

  CPPUNIT_ASSERT( test1 == m_suite->getChildTestAt(0) );
  CPPUNIT_ASSERT( test2 == m_suite->getChildTestAt(1) );
}


void 
TestSuiteTest::testGetChildTestAtThrow1()
{
  m_suite->getChildTestAt(-1);
}


void 
TestSuiteTest::testGetChildTestAtThrow2()
{
  m_suite->getChildTestAt(0);
}
