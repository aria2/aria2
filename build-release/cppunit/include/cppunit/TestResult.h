#ifndef CPPUNIT_TESTRESULT_H
#define CPPUNIT_TESTRESULT_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/SynchronizedObject.h>
#include <cppunit/portability/CppUnitDeque.h>
#include <string>

CPPUNIT_NS_BEGIN


class Exception;
class Functor;
class Protector;
class ProtectorChain;
class Test;
class TestFailure;
class TestListener;


#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<TestListener *>;
#endif

/*! \brief Manages TestListener.
 * \ingroup TrackingTestExecution
 *
 * A single instance of this class is used when running the test. It is usually
 * created by the test runner (TestRunner).
 *
 * This class shouldn't have to be inherited from. Use a TestListener
 * or one of its subclasses to be informed of the ongoing tests.
 * Use a Outputter to receive a test summary once it has finished
 *
 * TestResult supplies a template method 'setSynchronizationObject()'
 * so that subclasses can provide mutual exclusion in the face of multiple
 * threads.  This can be useful when tests execute in one thread and
 * they fill a subclass of TestResult which effects change in another 
 * thread.  To have mutual exclusion, override setSynchronizationObject()
 * and make sure that you create an instance of ExclusiveZone at the 
 * beginning of each method.
 *
 * \see Test, TestListener, TestResultCollector, Outputter.
 */
class CPPUNIT_API TestResult : protected SynchronizedObject
{
public:
  /// Construct a TestResult
  TestResult( SynchronizationObject *syncObject = 0 );

  /// Destroys a test result
  virtual ~TestResult();

  virtual void addListener( TestListener *listener );

  virtual void removeListener( TestListener *listener );

  /// Resets the stop flag.
  virtual void reset();
  
  /// Stop testing
  virtual void stop();

  /// Returns whether testing should be stopped
  virtual bool shouldStop() const;

  /// Informs TestListener that a test will be started.
  virtual void startTest( Test *test );

  /*! \brief Adds an error to the list of errors. 
   *  The passed in exception
   *  caused the error
   */
  virtual void addError( Test *test, Exception *e );

  /*! \brief Adds a failure to the list of failures. The passed in exception
   * caused the failure.
   */
  virtual void addFailure( Test *test, Exception *e );

  /// Informs TestListener that a test was completed.
  virtual void endTest( Test *test );

  /// Informs TestListener that a test suite will be started.
  virtual void startSuite( Test *test );

  /// Informs TestListener that a test suite was completed.
  virtual void endSuite( Test *test );

  /*! \brief Run the specified test.
   * 
   * Calls startTestRun(), test->run(this), and finally endTestRun().
   */
  virtual void runTest( Test *test );

  /*! \brief Protects a call to the specified functor.
   *
   * See Protector to understand how protector works. A default protector is
   * always present. It captures CppUnit::Exception, std::exception and
   * any other exceptions, retrieving as much as possible information about
   * the exception as possible.
   *
   * Additional Protector can be added to the chain to support other exception
   * types using pushProtector() and popProtector().
   *
   * \param functor Functor to call (typically a call to setUp(), runTest() or
   *                tearDown().
   * \param test Test the functor is associated to (used for failure reporting).
   * \param shortDescription Short description override for the failure message.
   */
  virtual bool protect( const Functor &functor,
                        Test *test,
                        const std::string &shortDescription = std::string("") );

  /// Adds the specified protector to the protector chain.
  virtual void pushProtector( Protector *protector );

  /// Removes the last protector from the protector chain.
  virtual void popProtector();

protected:
  /*! \brief Called to add a failure to the list of failures.
   */
  void addFailure( const TestFailure &failure );

  virtual void startTestRun( Test *test );
  virtual void endTestRun( Test *test );
  
protected:
  typedef CppUnitDeque<TestListener *> TestListeners;
  TestListeners m_listeners;
  ProtectorChain *m_protectorChain;
  bool m_stop;

private: 
  TestResult( const TestResult &other );
  TestResult &operator =( const TestResult &other );
};


CPPUNIT_NS_END


#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_TESTRESULT_H


