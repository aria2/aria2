#ifndef _INCLUDE_CPPUNIT_CONFIG_EVC4_H
#define _INCLUDE_CPPUNIT_CONFIG_EVC4_H 1

#if _MSC_VER > 1000     // VC++
#pragma warning( disable : 4786 )   // disable warning debug symbol > 255...
#endif // _MSC_VER > 1000

#define HAVE_CMATH 1
 
/* include/cppunit/config-msvc6.h. Manually adapted from 
   include/cppunit/config-auto.h */

/* define to 1 if the compiler implements namespaces */
#ifndef CPPUNIT_HAVE_NAMESPACES 
#define CPPUNIT_HAVE_NAMESPACES  1 
#endif

/* define if library uses std::string::compare(string,pos,n) */
#ifdef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST 
#undef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST
#endif

/* Define if you have the <dlfcn.h> header file. */
#ifdef CPPUNIT_HAVE_DLFCN_H 
#undef CPPUNIT_HAVE_DLFCN_H 
#endif

/* define to 1 if the compiler implements namespaces */
#ifndef CPPUNIT_HAVE_NAMESPACES 
#define CPPUNIT_HAVE_NAMESPACES  1 
#endif

/* define if the compiler supports Run-Time Type Identification */
#ifndef CPPUNIT_HAVE_RTTI 
#define CPPUNIT_HAVE_RTTI  0
#endif

/* Define to 1 to use type_info::name() for class names */
#ifndef CPPUNIT_USE_TYPEINFO_NAME 
#define CPPUNIT_USE_TYPEINFO_NAME  CPPUNIT_HAVE_RTTI 
#endif

#define CPPUNIT_NO_STREAM 1
#define CPPUNIT_NO_ASSERT 1

#define CPPUNIT_HAVE_SSTREAM 0

/* Name of package */
#ifndef CPPUNIT_PACKAGE 
#define CPPUNIT_PACKAGE  "cppunit" 
#endif


// Compiler error location format for CompilerOutputter
// See class CompilerOutputter for format.
#undef CPPUNIT_COMPILER_LOCATION_FORMAT
#if _MSC_VER >= 1300    // VS 7.0
# define CPPUNIT_COMPILER_LOCATION_FORMAT "%p(%l) : error : "
#else
# define CPPUNIT_COMPILER_LOCATION_FORMAT "%p(%l):"
#endif

/* define to 1 if the compiler has _finite() */
#ifndef CPPUNIT_HAVE__FINITE
#define CPPUNIT_HAVE__FINITE 1 
#endif

// Uncomment to turn on STL wrapping => use this to test compilation. 
// This will make CppUnit subclass std::vector & co to provide default
// parameter.
/*#define CPPUNIT_STD_NEED_ALLOCATOR 1
#define CPPUNIT_STD_ALLOCATOR std::allocator<T>
//#define CPPUNIT_NO_NAMESPACE 1
*/


/* _INCLUDE_CPPUNIT_CONFIG_EVC4_H */
#endif
