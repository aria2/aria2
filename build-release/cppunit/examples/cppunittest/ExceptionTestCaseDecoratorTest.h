// //////////////////////////////////////////////////////////////////////////
// Header file ExceptionTestCaseDecoratorTest.h for class ExceptionTestCaseDecoratorTest
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/08/03
// //////////////////////////////////////////////////////////////////////////
#ifndef EXCEPTIONTESTCASEDECORATORTEST_H
#define EXCEPTIONTESTCASEDECORATORTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestResult.h>
#include "FailureException.h"
#include "MockTestCase.h"
#include "MockTestListener.h"


/// Unit tests for ExceptionTestCaseDecoratorTest
class ExceptionTestCaseDecoratorTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ExceptionTestCaseDecoratorTest );
  CPPUNIT_TEST( testNoExceptionThrownFailed );
  CPPUNIT_TEST( testExceptionThrownPass );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a ExceptionTestCaseDecoratorTest object.
   */
  ExceptionTestCaseDecoratorTest();

  /// Destructor.
  virtual ~ExceptionTestCaseDecoratorTest();

  void setUp();
  void tearDown();

  void testNoExceptionThrownFailed();
  void testExceptionThrownPass();

private:
  /// Prevents the use of the copy constructor.
  ExceptionTestCaseDecoratorTest( const ExceptionTestCaseDecoratorTest &other );

  /// Prevents the use of the copy operator.
  void operator =( const ExceptionTestCaseDecoratorTest &other );

private:
  typedef CPPUNIT_NS::ExceptionTestCaseDecorator<FailureException> FailureExceptionTestCase;

  CPPUNIT_NS::TestResult *m_result;
  MockTestListener *m_testListener;
  MockTestCase *m_test;
  FailureExceptionTestCase *m_decorator;
};



// Inlines methods for ExceptionTestCaseDecoratorTest:
// ---------------------------------------------------



#endif  // EXCEPTIONTESTCASEDECORATORTEST_H
