#ifndef CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGEREXCEPTION_H
#define CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGEREXCEPTION_H

#include <cppunit/Portability.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)
#include <stdexcept>
#include <string>


CPPUNIT_NS_BEGIN


/*! \brief Exception thrown by DynamicLibraryManager when a failure occurs.
 *
 * Use getCause() to know what function caused the failure.
 *
 */
class DynamicLibraryManagerException : public std::runtime_error
{
public:
  enum Cause
  {
    /// Failed to load the dynamic library
    loadingFailed =0,
    /// Symbol not found in the dynamic library
    symbolNotFound
  };

  /// Failed to load the dynamic library or Symbol not found in the dynamic library.
  DynamicLibraryManagerException( const std::string &libraryName,
                                  const std::string &errorDetail,
                                  Cause cause );

  ~DynamicLibraryManagerException() throw()
  {
  }

  Cause getCause() const;

  const char *what() const throw();

private:
  std::string m_message;
  Cause m_cause;
};


CPPUNIT_NS_END

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)

#endif  // CPPUNIT_PLUGIN_DYNAMICLIBRARYMANAGEREXCEPTION_H
