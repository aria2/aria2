#ifndef CPPUNIT_EXTENSIONS_EXCEPTIONTESTCASEDECORATOR_H
#define CPPUNIT_EXTENSIONS_EXCEPTIONTESTCASEDECORATOR_H

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/extensions/TestCaseDecorator.h>

CPPUNIT_NS_BEGIN


/*! \brief Expected exception test case decorator.
 *
 * A decorator used to assert that a specific test case should throw an
 * exception of a given type.
 *
 * You should use this class only if you need to check the exception object
 * state (that a specific cause is set for example). If you don't need to
 * do that, you might consider using CPPUNIT_TEST_EXCEPTION() instead.
 *
 * Intended use is to subclass and override checkException(). Example:
 *
 * \code
 *
 * class NetworkErrorTestCaseDecorator : 
 *           public ExceptionTestCaseDecorator<NetworkError>
 * {
 * public:
 *   NetworkErrorTestCaseDecorator( NetworkError::Cause expectedCause )
 *       : m_expectedCause( expectedCause )
 *   {
 *   }
 * private:
 *   void checkException( ExpectedExceptionType &e )
 *   {
 *     CPPUNIT_ASSERT_EQUAL( m_expectedCause, e.getCause() );
 *   }
 *
 *   NetworkError::Cause m_expectedCause;
 * };
 * \endcode
 *
 */ 
template<class ExpectedException>
class ExceptionTestCaseDecorator : public TestCaseDecorator
{
public:
  typedef ExpectedException ExpectedExceptionType;

  /*! \brief Decorates the specified test.
   * \param test TestCase to decorate. Assumes ownership of the test.
   */
  ExceptionTestCaseDecorator( TestCase *test )
      : TestCaseDecorator( test )
  {
  }

  /*! \brief Checks that the expected exception is thrown by the decorated test.
   * is thrown.
   *
   * Calls the decorated test runTest() and checks that an exception of
   * type ExpectedException is thrown. Call checkException() passing the
   * exception that was caught so that some assertions can be made if
   * needed.
   */
  void runTest()
  {
    try
    {
      TestCaseDecorator::runTest();
    }
    catch ( ExpectedExceptionType &e )
    {
      checkException( e );
      return;
    }

    // Moved outside the try{} statement to handle the case where the
    // expected exception type is Exception (expecting assertion failure).
#if CPPUNIT_USE_TYPEINFO_NAME
      throw Exception( Message(
                         "expected exception not thrown",
                         "Expected exception type: " + 
                           TypeInfoHelper::getClassName( 
                               typeid( ExpectedExceptionType ) ) ) );
#else
      throw Exception( Message("expected exception not thrown") );
#endif
  }

private:
  /*! \brief Called when the exception is caught.
   *
   * Should be overriden to check the exception.
   */
  virtual void checkException( ExpectedExceptionType &e )
  {
  }
};


CPPUNIT_NS_END

#endif // CPPUNIT_EXTENSIONS_EXCEPTIONTESTCASEDECORATOR_H

