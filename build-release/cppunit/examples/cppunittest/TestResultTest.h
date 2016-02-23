#ifndef TESTRESULTTEST_H
#define TESTRESULTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestResult.h>
#include "MockTestListener.h"


class TestResultTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestResultTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testStop );
  CPPUNIT_TEST( testAddError );
  CPPUNIT_TEST( testAddFailure );
  CPPUNIT_TEST( testStartTest );
  CPPUNIT_TEST( testEndTest );
  CPPUNIT_TEST( testStartSuite );
  CPPUNIT_TEST( testEndSuite );
  CPPUNIT_TEST( testRunTest );
  CPPUNIT_TEST( testTwoListener );
  CPPUNIT_TEST( testDefaultProtectSucceed );
  CPPUNIT_TEST( testDefaultProtectFail );
  CPPUNIT_TEST( testDefaultProtectFailIfThrow );
  CPPUNIT_TEST( testProtectChainPushOneTrap );
  CPPUNIT_TEST( testProtectChainPushOnePassThrough );
  CPPUNIT_TEST( testProtectChainPushTwoTrap );
  CPPUNIT_TEST_SUITE_END();

public:
  TestResultTest();
  virtual ~TestResultTest();

  virtual void setUp();
  virtual void tearDown();

  void testConstructor();
  void testStop();

  void testAddError();
  void testAddFailure();
  void testStartTest();
  void testEndTest();
  void testStartSuite();
  void testEndSuite();
  void testRunTest();

  void testTwoListener();

  void testDefaultProtectSucceed();
  void testDefaultProtectFail();
  void testDefaultProtectFailIfThrow();

  void testProtectChainPushOneTrap();
  void testProtectChainPushOnePassThrough();

  void testProtectChainPushTwoTrap();

private:
  TestResultTest( const TestResultTest &copy );
  void operator =( const TestResultTest &copy );

private:
  CPPUNIT_NS::TestResult *m_result;
  MockTestListener *m_listener1;
  MockTestListener *m_listener2;
  CPPUNIT_NS::Test *m_dummyTest;
};



#endif  // TESTRESULTTEST_H
