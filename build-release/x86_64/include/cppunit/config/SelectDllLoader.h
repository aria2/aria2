#ifndef CPPUNIT_CONFIG_SELECTDLLLOADER_H
#define CPPUNIT_CONFIG_SELECTDLLLOADER_H

/*! \file
 * Selects DynamicLibraryManager implementation.
 *
 * Don't include this file directly. Include Portability.h instead.
 */

/*!
 * \def CPPUNIT_NO_TESTPLUGIN
 * \brief If defined, then plug-in related classes and functions will not be compiled.
 * 
 * \internal
 * CPPUNIT_HAVE_WIN32_DLL_LOADER
 * If defined, Win32 implementation of DynamicLibraryManager will be used.
 * 
 * CPPUNIT_HAVE_BEOS_DLL_LOADER
 * If defined, BeOs implementation of DynamicLibraryManager will be used.
 * 
 * CPPUNIT_HAVE_UNIX_DLL_LOADER
 * If defined, Unix implementation (dlfcn.h) of DynamicLibraryManager will be used.
 */

/*!
 * \def CPPUNIT_PLUGIN_EXPORT
 * \ingroup WritingTestPlugIn
 * \brief A macro to export a function from a dynamic library
 *
 * This macro export the C function following it from a dynamic library. 
 * Exporting the function makes it accessible to the DynamicLibraryManager.
 *
 * Example of usage:
 * \code
 * #include <cppunit/include/plugin/TestPlugIn.h>
 *
 * CPPUNIT_PLUGIN_EXPORT CppUnitTestPlugIn *CPPUNIT_PLUGIN_EXPORTED_NAME(void)
 * {
 *   ...
 *   return &myPlugInInterface;
 * }
 * \endcode
 */

#if !defined(CPPUNIT_NO_TESTPLUGIN)

// Is WIN32 platform ?
#if defined(WIN32)
#define CPPUNIT_HAVE_WIN32_DLL_LOADER 1
#undef CPPUNIT_PLUGIN_EXPORT
#define CPPUNIT_PLUGIN_EXPORT extern "C" __declspec(dllexport)

// Is BeOS platform ?
#elif defined(__BEOS__)
#define CPPUNIT_HAVE_BEOS_DLL_LOADER 1

// Is Unix platform and have shl_load() (hp-ux)
#elif defined(CPPUNIT_HAVE_SHL_LOAD)
#define CPPUNIT_HAVE_UNIX_SHL_LOADER 1

// Is Unix platform and have include <dlfcn.h>
#elif defined(CPPUNIT_HAVE_LIBDL)
#define CPPUNIT_HAVE_UNIX_DLL_LOADER 1

// Otherwise, disable support for DllLoader
#else
#define CPPUNIT_NO_TESTPLUGIN 1
#endif

#if !defined(CPPUNIT_PLUGIN_EXPORT)
#define CPPUNIT_PLUGIN_EXPORT extern "C"
#endif // !defined(CPPUNIT_PLUGIN_EXPORT)

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)

#endif // CPPUNIT_CONFIG_SELECTDLLLOADER_H
