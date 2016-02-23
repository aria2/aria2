#include <cppunit/config/SourcePrefix.h>  // disabled unwanted warning on vc++ 6.0
#include <cppunit/TestResult.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "ClockerXmlHook.h"
#include "ClockerListener.h"
#include "ClockerModel.h"



class ClockerPlugIn : public CppUnitTestPlugIn
{
public:
  ClockerPlugIn()
    : m_dumper( NULL )
    , m_model( NULL )
    , m_xmlHook( NULL )
  {
  }

  ~ClockerPlugIn()
  {
    delete m_dumper;
    delete m_model;
    delete m_xmlHook;
  }


  void initialize( CPPUNIT_NS::TestFactoryRegistry *registry,
                   const CPPUNIT_NS::PlugInParameters &parameters )
  {
    bool text = false;
    if ( parameters.getCommandLine() == "text" )
      text = true;

    m_model = new ClockerModel();
    m_dumper = new ClockerListener( m_model, text );
    m_xmlHook = new ClockerXmlHook( m_model );
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
    outputter->addHook( m_xmlHook );
  }


  void removeXmlOutputterHooks()
  {
  }


  void uninitialize( CPPUNIT_NS::TestFactoryRegistry *registry )
  {
  }

private:
  ClockerListener *m_dumper;
  ClockerModel *m_model;
  ClockerXmlHook *m_xmlHook;
};


CPPUNIT_PLUGIN_EXPORTED_FUNCTION_IMPL( ClockerPlugIn );

CPPUNIT_PLUGIN_IMPLEMENT_MAIN();