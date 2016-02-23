#ifndef CPPUNIT_TESTASSERT_H
#define CPPUNIT_TESTASSERT_H

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>
#include <cppunit/portability/Stream.h>
#include <stdio.h>
#include <float.h> // For struct assertion_traits<double>


CPPUNIT_NS_BEGIN


/*! \brief Traits used by CPPUNIT_ASSERT_EQUAL().
 *
 * Here is an example of specialising these traits: 
 *
 * \code
 * template<>
 * struct assertion_traits<std::string>   // specialization for the std::string type
 * {
 *   static bool equal( const std::string& x, const std::string& y )
 *   {
 *     return x == y;
 *   }
 * 
 *   static std::string toString( const std::string& x )
 *   {
 *     std::string text = '"' + x + '"';    // adds quote around the string to see whitespace
 *     OStringStream ost;
 *     ost << text;
 *     return ost.str();
 *   }
 * };
 * \endcode
 */
template <class T>
struct assertion_traits 
{  
    static bool equal( const T& x, const T& y )
    {
        return x == y;
    }

    static std::string toString( const T& x )
    {
        OStringStream ost;
        ost << x;
        return ost.str();
    }
};


/*! \brief Traits used by CPPUNIT_ASSERT_DOUBLES_EQUAL(). 
 * 
 * This specialisation from @c struct @c assertion_traits<> ensures that 
 * doubles are converted in full, instead of being rounded to the default 
 * 6 digits of precision. Use the system defined ISO C99 macro DBL_DIG 
 * within float.h is available to define the maximum precision, otherwise
 * use the hard-coded maximum precision of 15.
 */
template <>
struct assertion_traits<double>
{  
    static bool equal( double x, double y )
    {
        return x == y;
    }

    static std::string toString( double x )
    {
#ifdef DBL_DIG
       const int precision = DBL_DIG;
#else
       const int precision = 15;
#endif  // #ifdef DBL_DIG
       char buffer[128];
#ifdef __STDC_SECURE_LIB__ // Use secure version with visual studio 2005 to avoid warning.
       sprintf_s(buffer, sizeof(buffer), "%.*g", precision, x); 
#else	
       sprintf(buffer, "%.*g", precision, x); 
#endif
       return buffer;
    }
};


/*! \brief (Implementation) Asserts that two objects of the same type are equals.
 * Use CPPUNIT_ASSERT_EQUAL instead of this function.
 * \sa assertion_traits, Asserter::failNotEqual().
 */
template <class T>
void assertEquals( const T& expected,
                   const T& actual,
                   SourceLine sourceLine,
                   const std::string &message )
{
  if ( !assertion_traits<T>::equal(expected,actual) ) // lazy toString conversion...
  {
    Asserter::failNotEqual( assertion_traits<T>::toString(expected),
                            assertion_traits<T>::toString(actual),
                            sourceLine,
                            message );
  }
}


/*! \brief (Implementation) Asserts that two double are equals given a tolerance.
 * Use CPPUNIT_ASSERT_DOUBLES_EQUAL instead of this function.
 * \sa Asserter::failNotEqual().
 * \sa CPPUNIT_ASSERT_DOUBLES_EQUAL for detailed semantic of the assertion.
 */
void CPPUNIT_API assertDoubleEquals( double expected,
                                     double actual,
                                     double delta,
                                     SourceLine sourceLine, 
                                     const std::string &message );


/* A set of macros which allow us to get the line number
 * and file name at the point of an error.
 * Just goes to show that preprocessors do have some
 * redeeming qualities.
 */
#if CPPUNIT_HAVE_CPP_SOURCE_ANNOTATION
/** Assertions that a condition is \c true.
 * \ingroup Assertions
 */
#define CPPUNIT_ASSERT(condition)                                                 \
  ( CPPUNIT_NS::Asserter::failIf( !(condition),                                   \
                                 CPPUNIT_NS::Message( "assertion failed",         \
                                                      "Expression: " #condition), \
                                 CPPUNIT_SOURCELINE() ) )
#else
#define CPPUNIT_ASSERT(condition)                                            \
  ( CPPUNIT_NS::Asserter::failIf( !(condition),                              \
                                  CPPUNIT_NS::Message( "assertion failed" ), \
                                  CPPUNIT_SOURCELINE() ) )
#endif

/** Assertion with a user specified message.
 * \ingroup Assertions
 * \param message Message reported in diagnostic if \a condition evaluates
 *                to \c false.
 * \param condition If this condition evaluates to \c false then the
 *                  test failed.
 */
#define CPPUNIT_ASSERT_MESSAGE(message,condition)                          \
  ( CPPUNIT_NS::Asserter::failIf( !(condition),                            \
                                  CPPUNIT_NS::Message( "assertion failed", \
                                                       "Expression: "      \
                                                       #condition,         \
                                                       message ),          \
                                  CPPUNIT_SOURCELINE() ) )

/** Fails with the specified message.
 * \ingroup Assertions
 * \param message Message reported in diagnostic.
 */
#define CPPUNIT_FAIL( message )                                         \
  ( CPPUNIT_NS::Asserter::fail( CPPUNIT_NS::Message( "forced failure",  \
                                                     message ),         \
                                CPPUNIT_SOURCELINE() ) )

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
/// Generalized macro for primitive value comparisons
#define CPPUNIT_ASSERT_EQUAL(expected,actual)                     \
  ( CPPUNIT_NS::assertEquals( (expected),             \
                              (actual),               \
                              __LINE__, __FILE__ ) )
#else
/** Asserts that two values are equals.
 * \ingroup Assertions
 *
 * Equality and string representation can be defined with
 * an appropriate CppUnit::assertion_traits class.
 *
 * A diagnostic is printed if actual and expected values disagree.
 *
 * Requirement for \a expected and \a actual parameters:
 * - They are exactly of the same type
 * - They are serializable into a std::strstream using operator <<.
 * - They can be compared using operator ==. 
 *
 * The last two requirements (serialization and comparison) can be
 * removed by specializing the CppUnit::assertion_traits.
 */
#define CPPUNIT_ASSERT_EQUAL(expected,actual)          \
  ( CPPUNIT_NS::assertEquals( (expected),              \
                              (actual),                \
                              CPPUNIT_SOURCELINE(),    \
                              "" ) )

/** Asserts that two values are equals, provides additional message on failure.
 * \ingroup Assertions
 *
 * Equality and string representation can be defined with
 * an appropriate assertion_traits class.
 *
 * A diagnostic is printed if actual and expected values disagree.
 * The message is printed in addition to the expected and actual value
 * to provide additional information.
 *
 * Requirement for \a expected and \a actual parameters:
 * - They are exactly of the same type
 * - They are serializable into a std::strstream using operator <<.
 * - They can be compared using operator ==. 
 *
 * The last two requirements (serialization and comparison) can be
 * removed by specializing the CppUnit::assertion_traits.
 */
#define CPPUNIT_ASSERT_EQUAL_MESSAGE(message,expected,actual)      \
  ( CPPUNIT_NS::assertEquals( (expected),              \
                              (actual),                \
                              CPPUNIT_SOURCELINE(),    \
                              (message) ) )
#endif

/*! \brief Macro for primitive double value comparisons. 
 * \ingroup Assertions
 *
 * The assertion pass if both expected and actual are finite and
 * \c fabs( \c expected - \c actual ) <= \c delta.
 * If either \c expected or actual are infinite (+/- inf), the 
 * assertion pass if \c expected == \c actual.
 * If either \c expected or \c actual is a NaN (not a number), then
 * the assertion fails.
 */
#define CPPUNIT_ASSERT_DOUBLES_EQUAL(expected,actual,delta)        \
  ( CPPUNIT_NS::assertDoubleEquals( (expected),            \
                                    (actual),              \
                                    (delta),               \
                                    CPPUNIT_SOURCELINE(),  \
                                    "" ) )


/*! \brief Macro for primitive double value comparisons, setting a 
 * user-supplied message in case of failure. 
 * \ingroup Assertions
 * \sa CPPUNIT_ASSERT_DOUBLES_EQUAL for detailed semantic of the assertion.
 */
#define CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(message,expected,actual,delta)  \
  ( CPPUNIT_NS::assertDoubleEquals( (expected),            \
                                    (actual),              \
                                    (delta),               \
                                    CPPUNIT_SOURCELINE(),  \
                                    (message) ) )


/** Asserts that the given expression throws an exception of the specified type. 
 * \ingroup Assertions
 * Example of usage:
 * \code
 *   std::vector<int> v;
 *  CPPUNIT_ASSERT_THROW( v.at( 50 ), std::out_of_range );
 * \endcode
 */
# define CPPUNIT_ASSERT_THROW( expression, ExceptionType )              \
   CPPUNIT_ASSERT_THROW_MESSAGE( CPPUNIT_NS::AdditionalMessage(),       \
                                 expression,                            \
                                 ExceptionType )


// implementation detail
#if CPPUNIT_USE_TYPEINFO_NAME
#define CPPUNIT_EXTRACT_EXCEPTION_TYPE_( exception, no_rtti_message ) \
   CPPUNIT_NS::TypeInfoHelper::getClassName( typeid(exception) )
#else
#define CPPUNIT_EXTRACT_EXCEPTION_TYPE_( exception, no_rtti_message ) \
   std::string( no_rtti_message )
#endif // CPPUNIT_USE_TYPEINFO_NAME

// implementation detail
#define CPPUNIT_GET_PARAMETER_STRING( parameter ) #parameter

/** Asserts that the given expression throws an exception of the specified type, 
 * setting a user supplied message in case of failure. 
 * \ingroup Assertions
 * Example of usage:
 * \code
 *   std::vector<int> v;
 *  CPPUNIT_ASSERT_THROW_MESSAGE( "- std::vector<int> v;", v.at( 50 ), std::out_of_range );
 * \endcode
 */
# define CPPUNIT_ASSERT_THROW_MESSAGE( message, expression, ExceptionType )   \
   do {                                                                       \
      bool cpputCorrectExceptionThrown_ = false;                              \
      CPPUNIT_NS::Message cpputMsg_( "expected exception not thrown" );       \
      cpputMsg_.addDetail( message );                                         \
      cpputMsg_.addDetail( "Expected: "                                       \
                           CPPUNIT_GET_PARAMETER_STRING( ExceptionType ) );   \
                                                                              \
      try {                                                                   \
         expression;                                                          \
      } catch ( const ExceptionType & ) {                                     \
         cpputCorrectExceptionThrown_ = true;                                 \
      } catch ( const std::exception &e) {                                    \
         cpputMsg_.addDetail( "Actual  : " +                                  \
                              CPPUNIT_EXTRACT_EXCEPTION_TYPE_( e,             \
                                          "std::exception or derived") );     \
         cpputMsg_.addDetail( std::string("What()  : ") + e.what() );         \
      } catch ( ... ) {                                                       \
         cpputMsg_.addDetail( "Actual  : unknown.");                          \
      }                                                                       \
                                                                              \
      if ( cpputCorrectExceptionThrown_ )                                     \
         break;                                                               \
                                                                              \
      CPPUNIT_NS::Asserter::fail( cpputMsg_,                                  \
                                  CPPUNIT_SOURCELINE() );                     \
   } while ( false )


/** Asserts that the given expression does not throw any exceptions.
 * \ingroup Assertions
 * Example of usage:
 * \code
 *   std::vector<int> v;
 *   v.push_back( 10 );
 *  CPPUNIT_ASSERT_NO_THROW( v.at( 0 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_NO_THROW( expression )                             \
   CPPUNIT_ASSERT_NO_THROW_MESSAGE( CPPUNIT_NS::AdditionalMessage(),       \
                                    expression )


/** Asserts that the given expression does not throw any exceptions, 
 * setting a user supplied message in case of failure. 
 * \ingroup Assertions
 * Example of usage:
 * \code
 *   std::vector<int> v;
 *   v.push_back( 10 );
 *  CPPUNIT_ASSERT_NO_THROW( "std::vector<int> v;", v.at( 0 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_NO_THROW_MESSAGE( message, expression )               \
   do {                                                                       \
      CPPUNIT_NS::Message cpputMsg_( "unexpected exception caught" );         \
      cpputMsg_.addDetail( message );                                         \
                                                                              \
      try {                                                                   \
         expression;                                                          \
      } catch ( const std::exception &e ) {                                   \
         cpputMsg_.addDetail( "Caught: " +                                    \
                              CPPUNIT_EXTRACT_EXCEPTION_TYPE_( e,             \
                                          "std::exception or derived" ) );    \
         cpputMsg_.addDetail( std::string("What(): ") + e.what() );           \
         CPPUNIT_NS::Asserter::fail( cpputMsg_,                               \
                                     CPPUNIT_SOURCELINE() );                  \
      } catch ( ... ) {                                                       \
         cpputMsg_.addDetail( "Caught: unknown." );                           \
         CPPUNIT_NS::Asserter::fail( cpputMsg_,                               \
                                     CPPUNIT_SOURCELINE() );                  \
      }                                                                       \
   } while ( false )


/** Asserts that an assertion fail.
 * \ingroup Assertions
 * Use to test assertions.
 * Example of usage:
 * \code
 *   CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( 1 == 2 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_ASSERTION_FAIL( assertion )                 \
   CPPUNIT_ASSERT_THROW( assertion, CPPUNIT_NS::Exception )


/** Asserts that an assertion fail, with a user-supplied message in 
 * case of error.
 * \ingroup Assertions
 * Use to test assertions.
 * Example of usage:
 * \code
 *   CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( "1 == 2", CPPUNIT_ASSERT( 1 == 2 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( message, assertion )    \
   CPPUNIT_ASSERT_THROW_MESSAGE( message, assertion, CPPUNIT_NS::Exception )


/** Asserts that an assertion pass.
 * \ingroup Assertions
 * Use to test assertions.
 * Example of usage:
 * \code
 *   CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( 1 == 1 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_ASSERTION_PASS( assertion )                 \
   CPPUNIT_ASSERT_NO_THROW( assertion )


/** Asserts that an assertion pass, with a user-supplied message in 
 * case of failure. 
 * \ingroup Assertions
 * Use to test assertions.
 * Example of usage:
 * \code
 *   CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "1 != 1", CPPUNIT_ASSERT( 1 == 1 ) );
 * \endcode
 */
# define CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( message, assertion )    \
   CPPUNIT_ASSERT_NO_THROW_MESSAGE( message, assertion )




// Backwards compatibility

#if CPPUNIT_ENABLE_NAKED_ASSERT

#undef assert
#define assert(c)                 CPPUNIT_ASSERT(c)
#define assertEqual(e,a)          CPPUNIT_ASSERT_EQUAL(e,a)
#define assertDoublesEqual(e,a,d) CPPUNIT_ASSERT_DOUBLES_EQUAL(e,a,d)
#define assertLongsEqual(e,a)     CPPUNIT_ASSERT_EQUAL(e,a)

#endif


CPPUNIT_NS_END

#endif  // CPPUNIT_TESTASSERT_H
