#ifndef CPPUNIT_ASSERTER_H
#define CPPUNIT_ASSERTER_H

#include <cppunit/AdditionalMessage.h>
#include <cppunit/SourceLine.h>
#include <string>

CPPUNIT_NS_BEGIN


class Message;


/*! \brief A set of functions to help writing assertion macros.
 * \ingroup CreatingNewAssertions
 *
 * Here is an example of assertion, a simplified version of the
 * actual assertion implemented in examples/cppunittest/XmlUniformiser.h:
 * \code
 * #include <cppunit/SourceLine.h>
 * #include <cppunit/TestAssert.h>
 * 
 * void 
 * checkXmlEqual( std::string expectedXml,
 *                std::string actualXml,
 *                CppUnit::SourceLine sourceLine )
 * {
 *   std::string expected = XmlUniformiser( expectedXml ).stripped();
 *   std::string actual = XmlUniformiser( actualXml ).stripped();
 * 
 *   if ( expected == actual )
 *     return;
 * 
 *   ::CppUnit::Asserter::failNotEqual( expected,
 *                                      actual,
 *                                      sourceLine );
 * }
 * 
 * /// Asserts that two XML strings are equivalent.
 * #define CPPUNITTEST_ASSERT_XML_EQUAL( expected, actual ) \
 *     checkXmlEqual( expected, actual,                     \
 *                    CPPUNIT_SOURCELINE() )
 * \endcode
 */
struct Asserter
{
  /*! \brief Throws a Exception with the specified message and location.
   */
  static void CPPUNIT_API fail( const Message &message, 
                                const SourceLine &sourceLine = SourceLine() );

  /*! \brief Throws a Exception with the specified message and location.
   * \deprecated Use fail( Message, SourceLine ) instead.
   */
  static void CPPUNIT_API fail( std::string message, 
                                const SourceLine &sourceLine = SourceLine() );

  /*! \brief Throws a Exception with the specified message and location.
   * \param shouldFail if \c true then the exception is thrown. Otherwise
   *                   nothing happen.
   * \param message Message explaining the assertion failiure.
   * \param sourceLine Location of the assertion.
   */
  static void CPPUNIT_API failIf( bool shouldFail, 
                                  const Message &message, 
                                  const SourceLine &sourceLine = SourceLine() );

  /*! \brief Throws a Exception with the specified message and location.
   * \deprecated Use failIf( bool, Message, SourceLine ) instead.
   * \param shouldFail if \c true then the exception is thrown. Otherwise
   *                   nothing happen.
   * \param message Message explaining the assertion failiure.
   * \param sourceLine Location of the assertion.
   */
  static void CPPUNIT_API failIf( bool shouldFail, 
                                  std::string message, 
                                  const SourceLine &sourceLine = SourceLine() );

  /*! \brief Returns a expected value string for a message.
   * Typically used to create 'not equal' message, or to check that a message
   * contains the expected content when writing unit tests for your custom 
   * assertions.
   *
   * \param expectedValue String that represents the expected value.
   * \return \a expectedValue prefixed with "Expected: ".
   * \see makeActual().
   */
  static std::string CPPUNIT_API makeExpected( const std::string &expectedValue );

  /*! \brief Returns an actual value string for a message.
   * Typically used to create 'not equal' message, or to check that a message
   * contains the expected content when writing unit tests for your custom 
   * assertions.
   *
   * \param actualValue String that represents the actual value.
   * \return \a actualValue prefixed with "Actual  : ".
   * \see makeExpected().
   */
  static std::string CPPUNIT_API makeActual( const std::string &actualValue );

  static Message CPPUNIT_API makeNotEqualMessage( const std::string &expectedValue,
                                                  const std::string &actualValue,
                                                  const AdditionalMessage &additionalMessage = AdditionalMessage(),
                                                  const std::string &shortDescription = "equality assertion failed");

  /*! \brief Throws an Exception with the specified message and location.
   * \param expected Text describing the expected value.
   * \param actual Text describing the actual value.
   * \param sourceLine Location of the assertion.
   * \param additionalMessage Additional message. Usually used to report
   *                          what are the differences between the expected and actual value.
   * \param shortDescription Short description for the failure message.
   */
  static void CPPUNIT_API failNotEqual( std::string expected, 
                                        std::string actual, 
                                        const SourceLine &sourceLine,
                                        const AdditionalMessage &additionalMessage = AdditionalMessage(),
                                        std::string shortDescription = "equality assertion failed" );

  /*! \brief Throws an Exception with the specified message and location.
   * \param shouldFail if \c true then the exception is thrown. Otherwise
   *                   nothing happen.
   * \param expected Text describing the expected value.
   * \param actual Text describing the actual value.
   * \param sourceLine Location of the assertion.
   * \param additionalMessage Additional message. Usually used to report
   *                          where the "difference" is located.
   * \param shortDescription Short description for the failure message.
   */
  static void CPPUNIT_API failNotEqualIf( bool shouldFail,
                                          std::string expected, 
                                          std::string actual, 
                                          const SourceLine &sourceLine,
                                          const AdditionalMessage &additionalMessage = AdditionalMessage(),
                                          std::string shortDescription = "equality assertion failed" );

};


CPPUNIT_NS_END


#endif  // CPPUNIT_ASSERTER_H
