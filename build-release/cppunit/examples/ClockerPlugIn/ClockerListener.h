// //////////////////////////////////////////////////////////////////////////
// Header file ClockerListener.h for class ClockerListener
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////
#ifndef CLOCKERLISTENER_H
#define CLOCKERLISTENER_H

#include <cppunit/TestListener.h>

class ClockerModel;


/// TestListener that prints a flatten or hierarchical view of the test tree.
class ClockerListener : public CPPUNIT_NS::TestListener
{
public:
  ClockerListener( ClockerModel *model,
                   bool text );

  virtual ~ClockerListener();

  void startTestRun( CPPUNIT_NS::Test *test, 
                     CPPUNIT_NS::TestResult *eventManager );

  void endTestRun( CPPUNIT_NS::Test *test, 
                   CPPUNIT_NS::TestResult *eventManager );

  void startTest( CPPUNIT_NS::Test *test );

  void endTest( CPPUNIT_NS::Test *test );

  void startSuite( CPPUNIT_NS::Test *suite );

  void endSuite( CPPUNIT_NS::Test *suite );

private:
  void printStatistics() const;

  void printTest( int testIndex,
                  const std::string &indentString ) const;

  void printTestIndent( const std::string &indent,
                        const int indentLength ) const;

  void printTime( double time ) const;

  /// Prevents the use of the copy constructor.
  ClockerListener( const ClockerListener &other );

  /// Prevents the use of the copy operator.
  void operator =( const ClockerListener &other );

private:
  ClockerModel *m_model;
  bool m_text;
};



// Inlines methods for ClockerListener:
// -----------------------------------



#endif  // CLOCKERLISTENER_H
