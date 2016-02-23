#ifndef CPPUNIT_TESTFAILURE_H    // -*- C++ -*-
#define CPPUNIT_TESTFAILURE_H

#include <cppunit/Portability.h>
#include <string>

CPPUNIT_NS_BEGIN


class Exception;
class SourceLine;
class Test;


/*! \brief Record of a failed Test execution.
 * \ingroup BrowsingCollectedTestResult
 *
 * A TestFailure collects a failed test together with
 * the caught exception.
 *
 * TestFailure assumes lifetime control for any exception
 * passed to it.
 */
class CPPUNIT_API TestFailure 
{
public:
  TestFailure( Test *failedTest,
               Exception *thrownException,
               bool isError );

  virtual ~TestFailure ();

  virtual Test *failedTest() const;

  virtual Exception *thrownException() const;

  virtual SourceLine sourceLine() const;

  virtual bool isError() const;

  virtual std::string failedTestName() const;

  virtual TestFailure *clone() const;

protected:
  Test *m_failedTest;
  Exception *m_thrownException;
  bool m_isError;

private: 
  TestFailure( const TestFailure &other ); 
  TestFailure &operator =( const TestFailure& other ); 
};


CPPUNIT_NS_END

#endif // CPPUNIT_TESTFAILURE_H
