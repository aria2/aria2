#ifndef TESTFAILURETEST_H
#define TESTFAILURETEST_H

#include <cppunit/extensions/HelperMacros.h>


class TestFailureTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestFailureTest );
  CPPUNIT_TEST( testConstructorAndGetters );
  CPPUNIT_TEST( testConstructorAndGettersForError );
  CPPUNIT_TEST_SUITE_END();

public:
  TestFailureTest();
  virtual ~TestFailureTest();

  virtual void setUp();
  virtual void tearDown();

  void testConstructorAndGetters();
  void testConstructorAndGettersForError();

  void exceptionDestroyed();

private:
  class ObservedException : public CPPUNIT_NS::Exception
  {
  public:
    ObservedException( TestFailureTest *listener ) : 
        CPPUNIT_NS::Exception( CPPUNIT_NS::Message("ObservedException" ) ),
        m_listener( listener )
    {
    }

    virtual ~ObservedException() throw()
    {
      m_listener->exceptionDestroyed();
    }
  private:
    TestFailureTest *m_listener;
  };


  TestFailureTest( const TestFailureTest &copy );
  void operator =( const TestFailureTest &copy );
  void checkTestFailure( CPPUNIT_NS::Test *test, 
                         CPPUNIT_NS::Exception *error,
                         bool isError );

private:
  bool m_exceptionDestroyed;
};



#endif  // TESTFAILURETEST_H
