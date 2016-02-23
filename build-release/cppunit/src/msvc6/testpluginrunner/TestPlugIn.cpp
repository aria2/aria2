// //////////////////////////////////////////////////////////////////////////
// Implementation file TestPlugIn.cpp for class TestPlugIn
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/23
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TestPlugIn.h"
#include <cppunit/TestCase.h>
#include <cppunit/plugin/DynamicLibraryManagerException.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include "TestPlugInException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



TestPlugIn::TestPlugIn( const std::string fileName ) :
    m_fileName( fileName )
{
  m_copyFileName = m_fileName + "-hotrunner";
}


TestPlugIn::~TestPlugIn()
{
  deleteDllCopy();
}


void 
TestPlugIn::deleteDllCopy()
{
  m_manager.unload( m_copyFileName );
  ::DeleteFile( m_copyFileName.c_str() );
}


class NullTest : public CPPUNIT_NS::TestCase
{
public:
  NullTest( std::string name ) : TestCase( name ) 
  {
  }

  ~NullTest() 
  {
  }

  void runTests()
  {
    CPPUNIT_ASSERT_MESSAGE( "Failed to load" + getName(), FALSE );
  }
};


CPPUNIT_NS::Test *
TestPlugIn::makeTest()
{
  reloadDll();
  return CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
}


void 
TestPlugIn::reloadDll()
{
  m_manager.unload( m_copyFileName );
  makeDllCopy();
  loadDll();
}


void 
TestPlugIn::makeDllCopy()
{
  if ( ::CopyFile( m_fileName.c_str(), m_copyFileName.c_str(), FALSE ) == FALSE )
  {
    throw TestPlugInException( "Failed to copy DLL" + m_fileName +
        " to " + m_copyFileName, TestPlugInException::failedToCopyDll );
  }
}


void 
TestPlugIn::loadDll()
{
  try
  {
    m_manager.load( m_copyFileName );
  }
  catch ( CPPUNIT_NS::DynamicLibraryManagerException &e )
  {
    throw TestPlugInException( e.what(), 
                               TestPlugInException::failedToLoadDll );
  }
}
