#include <cppunit/Portability.h>

#if defined(CPPUNIT_HAVE_BEOS_DLL_LOADER)
#include <cppunit/plugin/DynamicLibraryManager.h>

#include <kernel/image.h>


CPPUNIT_NS_BEGIN


DynamicLibraryManager::LibraryHandle 
DynamicLibraryManager::doLoadLibrary( const std::string &libraryName )
{
  return (LibraryHandle)::load_add_on( libraryName.c_str() );
}


void 
DynamicLibraryManager::doReleaseLibrary()
{
  ::unload_add_on( (image_id)m_libraryHandle );
}


DynamicLibraryManager::Symbol 
DynamicLibraryManager::doFindSymbol( const std::string &symbol )
{
  void *symbolPointer;
  if ( ::get_image_symbol( (image_id)m_libraryHandle, 
                           symbol.c_str(), 
                           B_SYMBOL_TYPE_TEXT, 
                           &symbolPointer ) == B_OK )
    return symnolPointer;
  return NULL;
}


std::string 
DynamicLibraryManager::getLastErrorDetail() const
{
  return "";
}


CPPUNIT_NS_END


#endif // defined(CPPUNIT_HAVE_BEOS_DLL_LOADER)
