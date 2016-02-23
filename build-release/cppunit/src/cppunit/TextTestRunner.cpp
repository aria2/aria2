// ==> Implementation of cppunit/ui/text/TestRunner.h

#include <cppunit/config/SourcePrefix.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TextTestResult.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/portability/Stream.h>
#include <stdexcept>


CPPUNIT_NS_BEGIN


/*! Constructs a new text runner.
 * \param outputter used to print text result. Owned by the runner.
 */
TextTestRunner::TextTestRunner( Outputter *outputter ) 
    : m_result( new TestResultCollector() )
    , m_eventManager( new TestResult() )
    , m_outputter( outputter )
{
  if ( !m_outputter )
    m_outputter = new TextOutputter( m_result, stdCOut() );
  m_eventManager->addListener( m_result );
}


TextTestRunner::~TextTestRunner()
{
  delete m_eventManager;
  delete m_outputter;
  delete m_result;
}


/*! Runs the named test case.
 *
 * \param testName Name of the test case to run. If an empty is given, then
 *                 all added tests are run. The name can be the name of any
 *                 test in the hierarchy.
 * \param doWait if \c true then the user must press the RETURN key 
 *               before the run() method exit.
 * \param doPrintResult if \c true (default) then the test result are printed
 *                      on the standard output.
 * \param doPrintProgress if \c true (default) then TextTestProgressListener is
 *                        used to show the progress.
 * \return \c true is the test was successful, \c false if the test
 *         failed or was not found.
 */
bool
TextTestRunner::run( std::string testName,
                       bool doWait,
                       bool doPrintResult,
                       bool doPrintProgress )
{
  TextTestProgressListener progress;
  if ( doPrintProgress )
    m_eventManager->addListener( &progress );

  TestRunner *pThis = this;
  pThis->run( *m_eventManager, testName );

  if ( doPrintProgress )
    m_eventManager->removeListener( &progress );

  printResult( doPrintResult );
  wait( doWait );

  return m_result->wasSuccessful();
}


void 
TextTestRunner::wait( bool doWait )
{
#if !defined( CPPUNIT_NO_STREAM )
  if ( doWait ) 
  {
    stdCOut() << "<RETURN> to continue\n";
    stdCOut().flush();
    std::cin.get ();
  }
#endif
}


void 
TextTestRunner::printResult( bool doPrintResult )
{
  stdCOut() << "\n";
  if ( doPrintResult )
    m_outputter->write();
}


/*! Returns the result of the test run.
 * Use this after calling run() to access the result of the test run.
 */
TestResultCollector &
TextTestRunner::result() const
{
  return *m_result;
}


/*! Returns the event manager.
 * The instance of TestResult results returned is the one that is used to run the
 * test. Use this to register additional TestListener before running the tests.
 */
TestResult &
TextTestRunner::eventManager() const
{
  return *m_eventManager;
}


/*! Specifies an alternate outputter.
 *
 * Notes that the outputter will be use after the test run only if \a printResult was
 * \c true.
 * \param outputter New outputter to use. The previous outputter is destroyed. 
 *                  The TextTestRunner assumes ownership of the outputter.
 * \see CompilerOutputter, XmlOutputter, TextOutputter.
 */
void 
TextTestRunner::setOutputter( Outputter *outputter )
{
  delete m_outputter;
  m_outputter = outputter;
}


void 
TextTestRunner::run( TestResult &controller,
                     const std::string &testPath )
{
  TestRunner::run( controller, testPath );
}


CPPUNIT_NS_END
