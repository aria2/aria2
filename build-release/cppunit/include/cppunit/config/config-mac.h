#ifndef _INCLUDE_CPPUNIT_CONFIG_MAC_H
#define _INCLUDE_CPPUNIT_CONFIG_MAC_H 1

/* MacOS X should be installed using the configure script.
   This file is for other macs.

   It is not integrated into <cppunit/Portability.h> because we don't
   know a suitable preprocessor symbol that will distinguish MacOS X
   from other MacOS versions.  Email us if you know the answer.
*/
 
/* define if library uses std::string::compare(string,pos,n) */
#ifdef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST 
#undef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST
#endif

/* define if the library defines strstream */
#ifndef CPPUNIT_HAVE_CLASS_STRSTREAM 
#define CPPUNIT_HAVE_CLASS_STRSTREAM  1 
#endif

/* Define if you have the <cmath> header file. */
#ifdef CPPUNIT_HAVE_CMATH 
#undef CPPUNIT_HAVE_CMATH
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
#define CPPUNIT_HAVE_RTTI  1 
#endif

/* define if the compiler has stringstream */
#ifndef CPPUNIT_HAVE_SSTREAM 
#define CPPUNIT_HAVE_SSTREAM  1 
#endif

/* Define if you have the <strstream> header file. */
#ifndef CPPUNIT_HAVE_STRSTREAM 
#define CPPUNIT_HAVE_STRSTREAM  1 
#endif

/* Define to 1 to use type_info::name() for class names */
#ifndef CPPUNIT_USE_TYPEINFO_NAME 
#define CPPUNIT_USE_TYPEINFO_NAME  CPPUNIT_HAVE_RTTI 
#endif

/* _INCLUDE_CPPUNIT_CONFIG_MAC_H */
#endif
