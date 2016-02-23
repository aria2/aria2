#include <cppunit/TestResult.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "DumperListener.h"



class DumperPlugIn : public CppUnitTestPlugIn
{
public:
  DumperPlugIn()
    : m_dumper( NULL )
  {
  }

  ~DumperPlugIn()
  {
    delete m_dumper;
  }


  void initialize( CPPUNIT_NS::TestFactoryRegistry *registry,
                   const CPPUNIT_NS::PlugInParameters &parameters )
  {
    bool flatten = false;
    if ( parameters.getCommandLine() == "flat" )
      flatten = true;

    m_dumper = new DumperListener( flatten );
  }


  void addListener( CPPUNIT_NS::TestResult *eventManager )
  {
    eventManager->addListener( m_dumper );
  }


  void removeListener( CPPUNIT_NS::TestResult *eventManager )
  {
    eventManager->removeListener( m_dumper );
  }


  void addXmlOutputterHooks( CPPUNIT_NS::XmlOutputter *outputter )
  {
  }


  void removeXmlOutputterHooks()
  {
  }


  void uninitialize( CPPUNIT_NS::TestFactoryRegistry *registry )
  {
  }

private:
  DumperListener *m_dumper;
};


CPPUNIT_PLUGIN_EXPORTED_FUNCTION_IMPL( DumperPlugIn );

CPPUNIT_PLUGIN_IMPLEMENT_MAIN();