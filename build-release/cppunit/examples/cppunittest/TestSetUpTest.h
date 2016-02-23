#ifndef TESTSETUPTEST_H
#define TESTSETUPTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestSetUp.h>


class TestSetUpTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestSetUpTest );
  CPPUNIT_TEST( testRun );
  CPPUNIT_TEST_SUITE_END();

public:
  TestSetUpTest();
  virtual ~TestSetUpTest();

  void setUp();
  void tearDown();

  void testRun();

private:
  class MockSetUp : public CPPUNIT_NS::TestSetUp
  {
  public:
    MockSetUp( CPPUNIT_NS::Test *test )
        : CPPUNIT_NS::TestSetUp( test )
        , m_setUpCalled( false )
        , m_tearDownCalled( false )
    {
    }

    void setUp() 
    {
      m_setUpCalled = true;
    }

    void tearDown()
    {
      m_tearDownCalled = true;
    }

    void verify()
    {
      CPPUNIT_ASSERT( m_setUpCalled );
      CPPUNIT_ASSERT( m_tearDownCalled );
    }

  private:
    bool m_setUpCalled;
    bool m_tearDownCalled;
  };

  TestSetUpTest( const TestSetUpTest &copy );
  void operator =( const TestSetUpTest &copy );

private:
};



#endif  // TESTSETUPTEST_H
