#include <cppunit/plugin/DynamicLibraryManager.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)
#include <cppunit/plugin/DynamicLibraryManagerException.h>

CPPUNIT_NS_BEGIN


DynamicLibraryManager::DynamicLibraryManager( const std::string &libraryFileName )
    : m_libraryHandle( NULL )
    , m_libraryName( libraryFileName )
{
  loadLibrary( libraryFileName );
}


DynamicLibraryManager::~DynamicLibraryManager()
{
  releaseLibrary();
}


DynamicLibraryManager::Symbol 
DynamicLibraryManager::findSymbol( const std::string &symbol )
{
  try
  {
    Symbol symbolPointer = doFindSymbol( symbol );
    if ( symbolPointer != NULL )
      return symbolPointer;
  }
  catch ( ... )
  {
  }

  throw DynamicLibraryManagerException( m_libraryName, 
                                        symbol,
                                        DynamicLibraryManagerException::symbolNotFound );
  return NULL;    // keep compiler happy
}


void
DynamicLibraryManager::loadLibrary( const std::string &libraryName )
{
  try
  {
    releaseLibrary();
    m_libraryHandle = doLoadLibrary( libraryName );
    if ( m_libraryHandle != NULL )
      return;
  }
  catch (...)
  {
  }

  throw DynamicLibraryManagerException( m_libraryName,
                                        getLastErrorDetail(),
                                        DynamicLibraryManagerException::loadingFailed );
}


void 
DynamicLibraryManager::releaseLibrary()
{
  if ( m_libraryHandle != NULL )
  {
    doReleaseLibrary();
    m_libraryHandle = NULL;
  }
}


CPPUNIT_NS_END


#endif // !defined(CPPUNIT_NO_TESTPLUGIN)
