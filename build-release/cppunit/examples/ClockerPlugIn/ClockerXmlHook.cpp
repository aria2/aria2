// //////////////////////////////////////////////////////////////////////////
// Implementation file ClockerXmlHook.cpp for class ClockerXmlHook
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/06/14
// //////////////////////////////////////////////////////////////////////////

#include <cppunit/Test.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/XmlElement.h>
#include "ClockerModel.h"
#include "ClockerXmlHook.h"


ClockerXmlHook::ClockerXmlHook( ClockerModel *model )
    : m_model( model )
{
}


ClockerXmlHook::~ClockerXmlHook()
{
}


void 
ClockerXmlHook::endDocument( CPPUNIT_NS::XmlDocument *document )
{
  CPPUNIT_NS::XmlElement *testTreeElement = new CPPUNIT_NS::XmlElement( "TimedTestTree" );
  document->rootElement().addElement( testTreeElement );

  addTimedTest( testTreeElement, 0 );
}


void 
ClockerXmlHook::addTimedTest( CPPUNIT_NS::XmlElement *parentElement, 
                              int testIndex )
{
  std::string elementName = m_model->isSuite( testIndex ) ? "TimedSuite" : "TimedTest";
  CPPUNIT_NS::XmlElement *testElement = new CPPUNIT_NS::XmlElement( elementName );
  parentElement->addElement( testElement );
  testElement->addAttribute( "id", testIndex );

  const CPPUNIT_NS::TestPath &path = m_model->testPathFor( testIndex );
  testElement->addElement( new CPPUNIT_NS::XmlElement( "Name", 
                                                    path.getChildTest()->getName() ) );
  testElement->addElement( new CPPUNIT_NS::XmlElement( "TestPath", path.toString() ) );
  testElement->addElement( new CPPUNIT_NS::XmlElement( "Time", 
                                 ClockerModel::timeStringFor( 
                                    m_model->testTimeFor( testIndex ) ) ) );

  if ( m_model->isSuite( testIndex ) )
  {
    for ( int childIndex =0; childIndex < m_model->childCountFor( testIndex ); ++childIndex )
      addTimedTest( testElement, m_model->childAtFor( testIndex, childIndex ) );
  }
}


void 
ClockerXmlHook::failTestAdded( CPPUNIT_NS::XmlDocument *document,
                               CPPUNIT_NS::XmlElement *testElement,
                               CPPUNIT_NS::Test *test,
                               CPPUNIT_NS::TestFailure *failure )
{
  successfulTestAdded( document, testElement, test );
}


void 
ClockerXmlHook::successfulTestAdded( CPPUNIT_NS::XmlDocument *document,
                                     CPPUNIT_NS::XmlElement *testElement,
                                     CPPUNIT_NS::Test *test )
{
  int testIndex = m_model->indexOf( test );
  double time = (testIndex >= 0) ? m_model->testTimeFor( testIndex ) : 0.0;
  const CPPUNIT_NS::TestPath &path = m_model->testPathFor( testIndex );
  testElement->addElement( new CPPUNIT_NS::XmlElement( "TestPath", path.toString() ) );
  testElement->addElement( new CPPUNIT_NS::XmlElement( "Time",
                                   ClockerModel::timeStringFor( time ) ) );
}


void 
ClockerXmlHook::statisticsAdded( CPPUNIT_NS::XmlDocument *document,
                                 CPPUNIT_NS::XmlElement *statisticsElement )
{
  statisticsElement->addElement( 
      new CPPUNIT_NS::XmlElement( "TotalElapsedTime",
           ClockerModel::timeStringFor( m_model->totalElapsedTime() ) ) );
  statisticsElement->addElement( 
      new CPPUNIT_NS::XmlElement( "AverageTestCaseTime",
           ClockerModel::timeStringFor( m_model->averageTestCaseTime() ) ) );
}
