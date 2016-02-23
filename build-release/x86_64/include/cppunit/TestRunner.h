#ifndef CPPUNIT_TESTRUNNER_H
#define CPPUNIT_TESTRUNNER_H

#include <cppunit/TestSuite.h>
#include <string>

CPPUNIT_NS_BEGIN


class Test;
class TestResult;


/*! \brief Generic test runner.
 * \ingroup ExecutingTest
 *
 * The TestRunner assumes ownership of all added tests: you can not add test
 * or suite that are local variable since they can't be deleted.
 *
 * Example of usage:
 * \code
 * #include <cppunit/extensions/TestFactoryRegistry.h>
 * #include <cppunit/CompilerOutputter.h>
 * #include <cppunit/TestResult.h>
 * #include <cppunit/TestResultCollector.h>
 * #include <cppunit/TestRunner.h>
 * #include <cppunit/TextTestProgressListener.h>
 * 
 * 
 * int 
 * main( int argc, char* argv[] )
 * {
 *   std::string testPath = (argc > 1) ? std::string(argv[1]) : "";
 * 
 *   // Create the event manager and test controller
 *   CppUnit::TestResult controller;
 * 
 *   // Add a listener that colllects test result
 *   CppUnit::TestResultCollector result;
 *   controller.addListener( &result );        
 * 
 *   // Add a listener that print dots as test run.
 *   CppUnit::TextTestProgressListener progress;
 *   controller.addListener( &progress );      
 * 
 *   // Add the top suite to the test runner
 *   CppUnit::TestRunner runner;
 *   runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );   
 *   try
 *   {
 *     std::cout << "Running "  <<  testPath;
 *     runner.run( controller, testPath );
 * 
 *     std::cerr << std::endl;
 * 
 *     // Print test in a compiler compatible format.
 *     CppUnit::CompilerOutputter outputter( &result, std::cerr );
 *     outputter.write();                      
 *   }
 *   catch ( std::invalid_argument &e )  // Test path not resolved
 *   {
 *     std::cerr  <<  std::endl  
 *                <<  "ERROR: "  <<  e.what()
 *                << std::endl;
 *     return 0;
 *   }
 * 
 *   return result.wasSuccessful() ? 0 : 1;
 * }
 * \endcode
 */
class CPPUNIT_API TestRunner
{
public:
  /*! \brief Constructs a TestRunner object.
   */
  TestRunner(  );

  /// Destructor.
  virtual ~TestRunner();

  /*! \brief Adds the specified test.
   * \param test Test to add. The TestRunner takes ownership of the test.
   */
  virtual void addTest( Test *test );

  /*! \brief Runs a test using the specified controller.
   * \param controller Event manager and controller used for testing
   * \param testPath Test path string. See Test::resolveTestPath() for detail.
   * \exception std::invalid_argument if no test matching \a testPath is found.
   *                                  see TestPath::TestPath( Test*, const std::string &)
   *                                  for detail.
   */
  virtual void run( TestResult &controller,
                    const std::string &testPath = "" );

protected:
  /*! \brief (INTERNAL) Mutating test suite.
   */
  class CPPUNIT_API WrappingSuite : public TestSuite
  {
  public:
    WrappingSuite( const std::string &name = "All Tests" );

    int getChildTestCount() const;

    std::string getName() const;

    void run( TestResult *result );

  protected:
    Test *doGetChildTestAt( int index ) const;

    bool hasOnlyOneTest() const;

    Test *getUniqueChildTest() const;
  };

protected:
  WrappingSuite *m_suite;

private:
  /// Prevents the use of the copy constructor.
  TestRunner( const TestRunner &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestRunner &copy );

private:
};


CPPUNIT_NS_END

#endif  // CPPUNIT_TESTRUNNER_H
