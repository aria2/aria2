#ifndef CPPUNIT_TESTRESULTCOLLECTOR_H
#define CPPUNIT_TESTRESULTCOLLECTOR_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 4660 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/TestSuccessListener.h>
#include <cppunit/portability/CppUnitDeque.h>


CPPUNIT_NS_BEGIN

#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<TestFailure *>;
//  template class CPPUNIT_API std::deque<Test *>;
#endif


/*! \brief Collects test result.
 * \ingroup WritingTestResult
 * \ingroup BrowsingCollectedTestResult
 * 
 * A TestResultCollector is a TestListener which collects the results of executing 
 * a test case. It is an instance of the Collecting Parameter pattern.
 *
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 * \see TestListener, TestFailure.
 */
class CPPUNIT_API TestResultCollector : public TestSuccessListener
{
public:
  typedef CppUnitDeque<TestFailure *> TestFailures;
  typedef CppUnitDeque<Test *> Tests;


  /*! Constructs a TestResultCollector object.
   */
  TestResultCollector( SynchronizationObject *syncObject = 0 );

  /// Destructor.
  virtual ~TestResultCollector();

  void startTest( Test *test );
  void addFailure( const TestFailure &failure );

  virtual void reset();

  virtual int runTests() const;
  virtual int testErrors() const;
  virtual int testFailures() const;
  virtual int testFailuresTotal() const;

  virtual const TestFailures& failures() const;
  virtual const Tests &tests() const;

protected:
  void freeFailures();

  Tests m_tests;
  TestFailures m_failures;
  int m_testErrors;

private:
  /// Prevents the use of the copy constructor.
  TestResultCollector( const TestResultCollector &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestResultCollector &copy );
};



CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


#endif  // CPPUNIT_TESTRESULTCOLLECTOR_H
