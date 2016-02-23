#ifndef CPPUNIT_TESTLISTENER_H    // -*- C++ -*-
#define CPPUNIT_TESTLISTENER_H

#include <cppunit/Portability.h>


CPPUNIT_NS_BEGIN


class Exception;
class Test;
class TestFailure;
class TestResult;


/*! \brief Listener for test progress and result.
 * \ingroup TrackingTestExecution
 *
 * Implementing the Observer pattern a TestListener may be registered
 * to a TestResult to obtain information on the testing progress. Use
 * specialized sub classes of TestListener for text output
 * (TextTestProgressListener). Do not use the Listener for the test
 * result output, use a subclass of Outputter instead.
 *
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 *
 * Here is an example to track test time:
 *
 *
 * \code
 * #include <cppunit/TestListener.h>
 * #include <cppunit/Test.h>
 * #include <time.h>    // for clock()
 *
 * class TimingListener : public CppUnit::TestListener
 * {
 * public:
 *   void startTest( CppUnit::Test *test )
 *   {
 *     _chronometer.start();
 *   }
 *  
 *   void endTest( CppUnit::Test *test )
 *   {
 *     _chronometer.end();
 *     addTest( test, _chronometer.elapsedTime() );
 *   }
 *
 *   // ... (interface to add/read test timing result)
 *
 * private:
 *   Clock _chronometer;
 * };
 * \endcode
 *   
 * And another example that track failure/success at test suite level and captures
 * the TestPath of each suite:
 * \code
 * class SuiteTracker : public CppUnit::TestListener
 * {
 * public:
 *   void startSuite( CppUnit::Test *suite )
 *   {
 *     m_currentPath.add( suite );
 *   }
 *   
 *   void addFailure( const TestFailure &failure )
 *   {
 *     m_suiteFailure.top() = false;
 *   }
 * 
 *   void endSuite( CppUnit::Test *suite )
 *   {
 *     m_suiteStatus.insert( std::make_pair( suite, m_suiteFailure.top() ) );
 *     m_suitePaths.insert( std::make_pair( suite, m_currentPath ) );
 *
 *     m_currentPath.up();
 *     m_suiteFailure.pop();
 *   }
 *
 * private:
 *   std::stack<bool> m_suiteFailure;
 *   CppUnit::TestPath m_currentPath;
 *   std::map<CppUnit::Test *, bool> m_suiteStatus;
 *   std::map<CppUnit::Test *, CppUnit::TestPath> m_suitePaths;
 * };
 * \endcode
 *
 * \see TestResult
 */
class CPPUNIT_API TestListener
{
public:
  virtual ~TestListener() {}
  
  /// Called when just before a TestCase is run.
  virtual void startTest( Test * /*test*/ ) {}

  /*! \brief Called when a failure occurs while running a test.
   * \see TestFailure.
   * \warning \a failure is a temporary object that is destroyed after the 
   *          method call. Use TestFailure::clone() to create a duplicate.
   */
  virtual void addFailure( const TestFailure & /*failure*/ ) {}

  /// Called just after a TestCase was run (even if a failure occured).
  virtual void endTest( Test * /*test*/ ) {}

  /*! \brief Called by a TestComposite just before running its child tests.
   */
  virtual void startSuite( Test * /*suite*/ ) {}

  /*! \brief Called by a TestComposite after running its child tests.
   */
  virtual void endSuite( Test * /*suite*/ ) {}

  /*! \brief Called by a TestRunner before running the test.
   * 
   * You can use this to do some global initialisation. A listener
   * could also use to output a 'prolog' to the test run.
   *
   * \param test Test that is going to be run.
   * \param eventManager Event manager used for the test run.
   */
  virtual void startTestRun( Test * /*test*/, 
                             TestResult * /*eventManager*/ ) {}

  /*! \brief Called by a TestRunner after running the test.
   *
   * TextTestProgressListener use this to emit a line break. You can also use this
   * to do some global uninitialisation.
   *
   * \param test Test that was run.
   * \param eventManager Event manager used for the test run.
   */
  virtual void endTestRun( Test * /*test*/, 
                           TestResult * /*eventManager*/ ) {}
};


CPPUNIT_NS_END

#endif // CPPUNIT_TESTLISTENER_H


