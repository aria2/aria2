#ifndef _INCLUDE_CPPUNIT_CONFIG_BCB5_H
#define _INCLUDE_CPPUNIT_CONFIG_BCB5_H 1

#define HAVE_CMATH 1
 
/* include/cppunit/config-bcb5.h. Manually adapted from 
   include/cppunit/config-auto.h */

/* define to 1 if the compiler implements namespaces */
#ifndef CPPUNIT_HAVE_NAMESPACES 
#define CPPUNIT_HAVE_NAMESPACES  1 
#endif

/* define if library uses std::string::compare(string,pos,n) */
#ifndef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST 
#define CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST  0
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

/* Define to 1 to use type_info::name() for class names */
#ifndef CPPUNIT_USE_TYPEINFO_NAME 
#define CPPUNIT_USE_TYPEINFO_NAME  CPPUNIT_HAVE_RTTI 
#endif

#define CPPUNIT_HAVE_SSTREAM 1

/* Name of package */
#ifndef CPPUNIT_PACKAGE 
#define CPPUNIT_PACKAGE  "cppunit" 
#endif
 
/* _INCLUDE_CPPUNIT_CONFIG_BCB5_H */
#endif
