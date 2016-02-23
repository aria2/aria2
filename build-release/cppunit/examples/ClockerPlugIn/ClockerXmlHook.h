// //////////////////////////////////////////////////////////////////////////
// Header file ClockerXmlHook.h for class ClockerXmlHook
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/06/14
// //////////////////////////////////////////////////////////////////////////
#ifndef CLOCKERXMLHOOK_H
#define CLOCKERXMLHOOK_H

#include <cppunit/XmlOutputterHook.h>

class ClockerModel;



/// XML output hook to add test timing and test hierarchy timing.
class ClockerXmlHook : public CPPUNIT_NS::XmlOutputterHook
{
public:
  /*! Constructs a ClockerXmlHook object.
   */
  ClockerXmlHook( ClockerModel *model );

  /// Destructor.
  virtual ~ClockerXmlHook();

  void endDocument( CPPUNIT_NS::XmlDocument *document );

  void failTestAdded( CPPUNIT_NS::XmlDocument *document,
                      CPPUNIT_NS::XmlElement *testElement,
                      CPPUNIT_NS::Test *test,
                      CPPUNIT_NS::TestFailure *failure );

  void successfulTestAdded( CPPUNIT_NS::XmlDocument *document,
                            CPPUNIT_NS::XmlElement *testElement,
                            CPPUNIT_NS::Test *test );

  void statisticsAdded( CPPUNIT_NS::XmlDocument *document,
                        CPPUNIT_NS::XmlElement *statisticsElement );

private:
  /// Prevents the use of the copy constructor.
  ClockerXmlHook( const ClockerXmlHook &other );

  /// Prevents the use of the copy operator.
  void operator =( const ClockerXmlHook &other );

  void addTimedTest( CPPUNIT_NS::XmlElement *parentElement, 
                     int testIndex );

private:
  ClockerModel *m_model;
};



// Inlines methods for ClockerXmlHook:
// -----------------------------------



#endif  // CLOCKERXMLHOOK_H
