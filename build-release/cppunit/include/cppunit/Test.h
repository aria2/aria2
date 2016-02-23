#ifndef CPPUNIT_TEST_H
#define CPPUNIT_TEST_H

#include <cppunit/Portability.h>
#include <string>

CPPUNIT_NS_BEGIN


class TestResult;
class TestPath;

/*! \brief Base class for all test objects.
 * \ingroup BrowsingCollectedTestResult
 *
 * All test objects should be a subclass of Test.  Some test objects,
 * TestCase for example, represent one individual test.  Other test
 * objects, such as TestSuite, are comprised of several tests.  
 *
 * When a Test is run, the result is collected by a TestResult object.
 *
 * \see TestCase
 * \see TestSuite
 */
class CPPUNIT_API Test
{
public:
  virtual ~Test() {};

  /*! \brief Run the test, collecting results.
   */
  virtual void run( TestResult *result ) =0;

  /*! \brief Return the number of test cases invoked by run().
   *
   * The base unit of testing is the class TestCase.  This
   * method returns the number of TestCase objects invoked by
   * the run() method.
   */
  virtual int countTestCases () const =0;

  /*! \brief Returns the number of direct child of the test.
   */
  virtual int getChildTestCount() const =0;

  /*! \brief Returns the child test of the specified index.
   *
   * This method test if the index is valid, then call doGetChildTestAt() if 
   * the index is valid. Otherwise std::out_of_range exception is thrown.
   *
   * You should override doGetChildTestAt() method.
   * 
   * \param index Zero based index of the child test to return.
   * \return Pointer on the test. Never \c NULL.
   * \exception std::out_of_range is \a index is < 0 or >= getChildTestCount().
   */
  virtual Test *getChildTestAt( int index ) const;

  /*! \brief Returns the test name.
   * 
   * Each test has a name.  This name may be used to find the
   * test in a suite or registry of tests.
   */
  virtual std::string getName () const =0;

  /*! \brief Finds the test with the specified name and its parents test.
   * \param testName Name of the test to find.
   * \param testPath If the test is found, then all the tests traversed to access
   *                 \a test are added to \a testPath, including \c this and \a test.
   * \return \c true if a test with the specified name is found, \c false otherwise.
   */
  virtual bool findTestPath( const std::string &testName,
                             TestPath &testPath ) const;

  /*! \brief Finds the specified test and its parents test.
   * \param test Test to find.
   * \param testPath If the test is found, then all the tests traversed to access
   *                 \a test are added to \a testPath, including \c this and \a test.
   * \return \c true if the specified test is found, \c false otherwise.
   */
  virtual bool findTestPath( const Test *test,
                             TestPath &testPath ) const;

  /*! \brief Finds the test with the specified name in the hierarchy.
   * \param testName Name of the test to find.
   * \return Pointer on the first test found that is named \a testName. Never \c NULL.
   * \exception std::invalid_argument if no test named \a testName is found.
   */
  virtual Test *findTest( const std::string &testName ) const;

  /*! \brief Resolved the specified test path with this test acting as 'root'.
   * \param testPath Test path string to resolve.
   * \return Resolved TestPath. 
   * \exception std::invalid_argument if \a testPath could not be resolved.
   * \see TestPath.
   */
  virtual TestPath resolveTestPath( const std::string &testPath ) const;

protected:
  /*! Throws an exception if the specified index is invalid.
   * \param index Zero base index of a child test.
   * \exception std::out_of_range is \a index is < 0 or >= getChildTestCount().
   */
  virtual void checkIsValidIndex( int index ) const;

  /*! \brief Returns the child test of the specified valid index.
   * \param index Zero based valid index of the child test to return.
   * \return Pointer on the test. Never \c NULL.
   */
  virtual Test *doGetChildTestAt( int index ) const =0;
};


CPPUNIT_NS_END

#endif // CPPUNIT_TEST_H

