#include <cppunit/plugin/DynamicLibraryManagerException.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)

CPPUNIT_NS_BEGIN


DynamicLibraryManagerException::DynamicLibraryManagerException( 
                                         const std::string &libraryName,
                                         const std::string &errorDetail,
                                         Cause cause )
    : std::runtime_error( "" ),
      m_cause( cause )
{
  if ( cause == loadingFailed )
    m_message = "Failed to load dynamic library: " + libraryName + "\n" + 
                errorDetail;
  else
    m_message = "Symbol [" + errorDetail + "] not found in dynamic libary:" + 
                libraryName;
}


DynamicLibraryManagerException::Cause 
DynamicLibraryManagerException::getCause() const
{
  return m_cause;
}


const char *
DynamicLibraryManagerException::what() const throw()
{
  return m_message.c_str();
}


CPPUNIT_NS_END


#endif // !defined(CPPUNIT_NO_TESTPLUGIN)
