#include <cppunit/Portability.h>

#if defined(CPPUNIT_HAVE_UNIX_DLL_LOADER)
#include <cppunit/plugin/DynamicLibraryManager.h>

#include <dlfcn.h>
#include <unistd.h>


CPPUNIT_NS_BEGIN


DynamicLibraryManager::LibraryHandle 
DynamicLibraryManager::doLoadLibrary( const std::string &libraryName )
{
  return ::dlopen( libraryName.c_str(), RTLD_NOW | RTLD_GLOBAL );
}


void 
DynamicLibraryManager::doReleaseLibrary()
{
  ::dlclose( m_libraryHandle);
}


DynamicLibraryManager::Symbol 
DynamicLibraryManager::doFindSymbol( const std::string &symbol )
{
  return ::dlsym ( m_libraryHandle, symbol.c_str() );
}


std::string 
DynamicLibraryManager::getLastErrorDetail() const
{
  return "";
}


CPPUNIT_NS_END


#endif // defined(CPPUNIT_HAVE_UNIX_DLL_LOADER)
