#ifndef CPPUNIT_CONFIG_CPPUNITAPI
#define CPPUNIT_CONFIG_CPPUNITAPI

#undef CPPUNIT_API

#ifdef WIN32

// define CPPUNIT_DLL_BUILD when building CppUnit dll.
#ifdef CPPUNIT_BUILD_DLL
#define CPPUNIT_API __declspec(dllexport)
#endif

// define CPPUNIT_DLL when linking to CppUnit dll.
#ifdef CPPUNIT_DLL
#define CPPUNIT_API __declspec(dllimport)
#endif

#ifdef CPPUNIT_API
#undef CPPUNIT_NEED_DLL_DECL
#define CPPUNIT_NEED_DLL_DECL 1
#endif

#endif


#ifndef CPPUNIT_API
#define CPPUNIT_API
#undef CPPUNIT_NEED_DLL_DECL
#define CPPUNIT_NEED_DLL_DECL 0
#endif

 
#endif  // CPPUNIT_CONFIG_CPPUNITAPI
