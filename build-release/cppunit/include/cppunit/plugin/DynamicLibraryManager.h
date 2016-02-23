#ifndef CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGER_H
#define CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGER_H

#include <cppunit/Portability.h>
#include <string>

#if !defined(CPPUNIT_NO_TESTPLUGIN)

CPPUNIT_NS_BEGIN


/*! \brief Manages dynamic libraries.
 *
 * The Dynamic Library Manager provides a platform independent way to work with
 * dynamic library. It load a specific dynamic library, and can returns specific
 * symbol exported by the dynamic library.
 *
 * If an error occurs, a DynamicLibraryManagerException is thrown.
 *
 * \internal Implementation of the OS independent methods is in 
 * DynamicLibraryManager.cpp.
 *
 * \internal Porting to a new platform:
 * - Adds platform detection in config/SelectDllLoader.h. Should define a specific
 *   macro for that platform of the form: CPPUNIT_HAVE_XYZ_DLL_LOADER, where
 *   XYZ is the platform.
 * - Makes a copy of UnixDynamicLibraryManager.cpp and named it after the platform.
 * - Updated the 'guard' in your file (CPPUNIT_HAVE_XYZ_DLL_LOADER) so that it is
 *   only processed if the matching platform has been detected.
 * - Change the implementation of methods doLoadLibrary(), doReleaseLibrary(), 
 *   doFindSymbol() in your copy. Those methods usually maps directly to OS calls.
 * - Adds the file to the project.
 */
class DynamicLibraryManager
{
public:
  typedef void *Symbol;
  typedef void *LibraryHandle;

  /*! \brief Loads the specified library.
   * \param libraryFileName Name of the library to load.
   * \exception DynamicLibraryManagerException if a failure occurs while loading
   *            the library (fail to found or load the library).
   */
  DynamicLibraryManager( const std::string &libraryFileName );

  /// Releases the loaded library..
  ~DynamicLibraryManager();

  /*! \brief Returns a pointer on the specified symbol exported by the library.
   * \param symbol Name of the symbol exported by the library.
   * \return Pointer on the symbol. Should be casted to the actual type. Never \c NULL.
   * \exception DynamicLibraryManagerException if the symbol is not found.
   */
  Symbol findSymbol( const std::string &symbol );

private:
  /*! Loads the specified library.
   * \param libraryName Name of the library to load.
   * \exception DynamicLibraryManagerException if a failure occurs while loading
   *            the library (fail to found or load the library).
   */
  void loadLibrary( const std::string &libraryName );

  /*! Releases the loaded library.
   * 
   * \warning Must NOT throw any exceptions (called from destructor).
   */
  void releaseLibrary();

  /*! Loads the specified library.
   * 
   * May throw any exceptions (indicates failure).
   * \param libraryName Name of the library to load.
   * \return Handle of the loaded library. \c NULL indicates failure.
   */
  LibraryHandle doLoadLibrary( const std::string &libraryName );

  /*! Releases the loaded library.
   *
   * The handle of the library to free is in \c m_libraryHandle. It is never
   * \c NULL.
   * \warning Must NOT throw any exceptions (called from destructor).
   */
  void doReleaseLibrary();

  /*! Returns a pointer on the specified symbol exported by the library.
   * 
   * May throw any exceptions (indicates failure).
   * \param symbol Name of the symbol exported by the library.
   * \return Pointer on the symbol. \c NULL indicates failure.
   */
  Symbol doFindSymbol( const std::string &symbol );

  /*! Returns detailed information about doLoadLibrary() failure.
   *
   * Called just after a failed call to doLoadLibrary() to get extra
   * error information.
   *
   * \return Detailed information about the failure of the call to
   *         doLoadLibrary() that just failed.
   */
  std::string getLastErrorDetail() const;

  /// Prevents the use of the copy constructor.
  DynamicLibraryManager( const DynamicLibraryManager &copy );

  /// Prevents the use of the copy operator.
  void operator =( const DynamicLibraryManager &copy );

private:
  LibraryHandle m_libraryHandle;
  std::string m_libraryName;
};


CPPUNIT_NS_END

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)

#endif  // CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGER_H
