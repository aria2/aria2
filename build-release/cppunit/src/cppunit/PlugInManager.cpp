#include <cppunit/config/SourcePrefix.h>
#include <cppunit/XmlOutputterHook.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/plugin/PlugInManager.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/plugin/DynamicLibraryManager.h>


CPPUNIT_NS_BEGIN


PlugInManager::PlugInManager()
{
}


PlugInManager::~PlugInManager()
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
    unload( *it );
}


void
PlugInManager::load( const std::string &libraryFileName,
                     const PlugInParameters &parameters )
{
  PlugInInfo info;
  info.m_fileName = libraryFileName;
  info.m_manager = new DynamicLibraryManager( libraryFileName );

  TestPlugInSignature plug = (TestPlugInSignature)info.m_manager->findSymbol( 
        CPPUNIT_STRINGIZE( CPPUNIT_PLUGIN_EXPORTED_NAME ) );
  info.m_interface = (*plug)();

  m_plugIns.push_back( info );
  
  info.m_interface->initialize( &TestFactoryRegistry::getRegistry(), parameters );
}


void 
PlugInManager::unload( const std::string &libraryFileName )
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
  {
    if ( (*it).m_fileName == libraryFileName )
    {
      unload( *it );
      m_plugIns.erase( it );
      break;
    }
  }
}


void 
PlugInManager::addListener( TestResult *eventManager )
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
    (*it).m_interface->addListener( eventManager );
}


void 
PlugInManager::removeListener( TestResult *eventManager )
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
    (*it).m_interface->removeListener( eventManager );
}


void 
PlugInManager::unload( PlugInInfo &plugIn )
{
  try
  {
    plugIn.m_interface->uninitialize( &TestFactoryRegistry::getRegistry() );
    delete plugIn.m_manager;
  }
  catch (...)
  {
    delete plugIn.m_manager;
    plugIn.m_manager = NULL;
    throw;
  }
}


void 
PlugInManager::addXmlOutputterHooks( XmlOutputter *outputter )
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
    (*it).m_interface->addXmlOutputterHooks( outputter );
}


void 
PlugInManager::removeXmlOutputterHooks()
{
  for ( PlugIns::iterator it = m_plugIns.begin(); it != m_plugIns.end(); ++it )
    (*it).m_interface->removeXmlOutputterHooks();
}


CPPUNIT_NS_END

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)
