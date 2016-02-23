#ifndef CPPUNIT_PORTABILITY_H
#define CPPUNIT_PORTABILITY_H

#if defined(_WIN32) && !defined(WIN32)
# define WIN32 1
#endif

/* include platform specific config */
#if defined(__BORLANDC__)
#  include <cppunit/config/config-bcb5.h>
#elif defined (_MSC_VER)
#  if _MSC_VER == 1200 && defined(_WIN32_WCE) //evc4
#    include <cppunit/config/config-evc4.h>
#  else
#    include <cppunit/config/config-msvc6.h>
#  endif
#else
#    include <cppunit/config-auto.h>
#endif

// Version number of package
#ifndef CPPUNIT_VERSION 
#define CPPUNIT_VERSION  "1.12.0"
#endif
 
#include <cppunit/config/CppUnitApi.h>    // define CPPUNIT_API & CPPUNIT_NEED_DLL_DECL
#include <cppunit/config/SelectDllLoader.h>


/* Options that the library user may switch on or off.
 * If the user has not done so, we chose default values.
 */


/* Define to 1 if you wish to have the old-style macros
   assert(), assertEqual(), assertDoublesEqual(), and assertLongsEqual() */
#if !defined(CPPUNIT_ENABLE_NAKED_ASSERT)
# define CPPUNIT_ENABLE_NAKED_ASSERT          0
#endif

/* Define to 1 if you wish to have the old-style CU_TEST family
   of macros. */
#if !defined(CPPUNIT_ENABLE_CU_TEST_MACROS)
# define CPPUNIT_ENABLE_CU_TEST_MACROS        0
#endif

/* Define to 1 if the preprocessor expands (#foo) to "foo" (quotes incl.) 
   I don't think there is any C preprocess that does NOT support this! */
#if !defined(CPPUNIT_HAVE_CPP_SOURCE_ANNOTATION)
# define CPPUNIT_HAVE_CPP_SOURCE_ANNOTATION   1
#endif

/* Assumes that STL and CppUnit are in global space if the compiler does not
   support namespace. */
#if !defined(CPPUNIT_HAVE_NAMESPACES)
# if !defined(CPPUNIT_NO_NAMESPACE)
#  define CPPUNIT_NO_NAMESPACE 1
# endif // !defined(CPPUNIT_NO_NAMESPACE)
# if !defined(CPPUNIT_NO_STD_NAMESPACE)
#  define CPPUNIT_NO_STD_NAMESPACE 1
# endif // !defined(CPPUNIT_NO_STD_NAMESPACE)
#endif // !defined(CPPUNIT_HAVE_NAMESPACES)

/* Define CPPUNIT_STD_NEED_ALLOCATOR to 1 if you need to specify
 * the allocator you used when instantiating STL container. Typically
 * used for compilers that do not support template default parameter.
 * CPPUNIT_STD_ALLOCATOR will be used as the allocator. Default is
 * std::allocator. On some compilers, you may need to change this to
 * std::allocator<T>.
 */
#if CPPUNIT_STD_NEED_ALLOCATOR
# if !defined(CPPUNIT_STD_ALLOCATOR)
#  define CPPUNIT_STD_ALLOCATOR std::allocator
# endif // !defined(CPPUNIT_STD_ALLOCATOR)
#endif // defined(CPPUNIT_STD_NEED_ALLOCATOR)


// Compiler error location format for CompilerOutputter
// If not define, assumes that it's gcc
// See class CompilerOutputter for format.
#if !defined(CPPUNIT_COMPILER_LOCATION_FORMAT)
#if defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) )
// gcc/Xcode integration on Mac OS X
# define CPPUNIT_COMPILER_LOCATION_FORMAT "%p:%l: " 
#else
# define CPPUNIT_COMPILER_LOCATION_FORMAT "%f:%l:"
#endif
#endif

// If CPPUNIT_HAVE_CPP_CAST is defined, then c++ style cast will be used,
// otherwise, C style cast are used.
#if defined( CPPUNIT_HAVE_CPP_CAST )
# define CPPUNIT_CONST_CAST( TargetType, pointer ) \
    const_cast<TargetType>( pointer )

# define CPPUNIT_STATIC_CAST( TargetType, pointer ) \
    static_cast<TargetType>( pointer )
#else // defined( CPPUNIT_HAVE_CPP_CAST )
# define CPPUNIT_CONST_CAST( TargetType, pointer ) \
    ((TargetType)( pointer ))
# define CPPUNIT_STATIC_CAST( TargetType, pointer ) \
    ((TargetType)( pointer ))
#endif // defined( CPPUNIT_HAVE_CPP_CAST )

// If CPPUNIT_NO_STD_NAMESPACE is defined then STL are in the global space.
// => Define macro 'std' to nothing
#if defined(CPPUNIT_NO_STD_NAMESPACE)
# undef std
# define std
#endif  // defined(CPPUNIT_NO_STD_NAMESPACE)

// If CPPUNIT_NO_NAMESPACE is defined, then put CppUnit classes in the
// global namespace: the compiler does not support namespace.
#if defined(CPPUNIT_NO_NAMESPACE)
# define CPPUNIT_NS_BEGIN
# define CPPUNIT_NS_END
# define CPPUNIT_NS
#else   // defined(CPPUNIT_NO_NAMESPACE)
# define CPPUNIT_NS_BEGIN namespace CppUnit {
# define CPPUNIT_NS_END }
# define CPPUNIT_NS CppUnit
#endif  // defined(CPPUNIT_NO_NAMESPACE)

/*! Stringize a symbol.
 * 
 * Use this macro to convert a preprocessor symbol to a string.
 *
 * Example of usage:
 * \code
 * #define CPPUNIT_PLUGIN_EXPORTED_NAME cppunitTestPlugIn
 * const char *name = CPPUNIT_STRINGIZE( CPPUNIT_PLUGIN_EXPORTED_NAME );
 * \endcode
 */
#define CPPUNIT_STRINGIZE( symbol ) _CPPUNIT_DO_STRINGIZE( symbol )

/// \internal
#define _CPPUNIT_DO_STRINGIZE( symbol ) #symbol

/*! Joins to symbol after expanding them into string.
 *
 * Use this macro to join two symbols. Example of usage:
 *
 * \code
 * #define MAKE_UNIQUE_NAME(prefix) CPPUNIT_JOIN( prefix, __LINE__ )
 * \endcode
 *
 * The macro defined in the example concatenate a given prefix with the line number
 * to obtain a 'unique' identifier.
 *
 * \internal From boost documentation:
 * The following piece of macro magic joins the two 
 * arguments together, even when one of the arguments is
 * itself a macro (see 16.3.1 in C++ standard).  The key
 * is that macro expansion of macro arguments does not
 * occur in CPPUNIT_JOIN2 but does in CPPUNIT_JOIN.
 */
#define CPPUNIT_JOIN( symbol1, symbol2 ) _CPPUNIT_DO_JOIN( symbol1, symbol2 )

/// \internal
#define _CPPUNIT_DO_JOIN( symbol1, symbol2 ) _CPPUNIT_DO_JOIN2( symbol1, symbol2 )

/// \internal
#define _CPPUNIT_DO_JOIN2( symbol1, symbol2 ) symbol1##symbol2

/*! Adds the line number to the specified string to create a unique identifier.
 * \param prefix Prefix added to the line number to create a unique identifier.
 * \see CPPUNIT_TEST_SUITE_REGISTRATION for an example of usage.
 */
#define CPPUNIT_MAKE_UNIQUE_NAME( prefix ) CPPUNIT_JOIN( prefix, __LINE__ )

/*! Defines wrap colunm for %CppUnit. Used by CompilerOuputter.
 */
#if !defined(CPPUNIT_WRAP_COLUMN)
# define CPPUNIT_WRAP_COLUMN 79
#endif

#endif // CPPUNIT_PORTABILITY_H
