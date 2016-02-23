// //////////////////////////////////////////////////////////////////////////
// Header file DumperListener.h for class DumperListener
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////
#ifndef DUMPERLISTENER_H
#define DUMPERLISTENER_H

#include <cppunit/portability/CppUnitStack.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestPath.h>


/// TestListener that prints a flatten or hierarchical view of the test tree.
class DumperListener : public CPPUNIT_NS::TestListener
{
public:
  DumperListener( bool flatten );

  virtual ~DumperListener();

  void startTest( CPPUNIT_NS::Test *test );

  void endTest( CPPUNIT_NS::Test *test );

  void startSuite( CPPUNIT_NS::Test *suite );

  void endSuite( CPPUNIT_NS::Test *suite );

  void endTestRun( CPPUNIT_NS::Test *test, 
                   CPPUNIT_NS::TestResult *eventManager );

private:
  /// Prevents the use of the copy constructor.
  DumperListener( const DumperListener &other );

  /// Prevents the use of the copy operator.
  void operator =( const DumperListener &other );

  void printPath( CPPUNIT_NS::Test *test, 
                  bool isSuite );

  void printFlattenedPath( bool isSuite );

  void printIndentedPathChild();

  std::string makeIndentString( int indentLevel );

private:
  bool m_flatten;
  CPPUNIT_NS::TestPath m_path;
  
  int m_suiteCount;
  int m_testCount;
  int m_suiteWithTestCount;

  CppUnitStack<bool> m_suiteHasTest;
};



// Inlines methods for DumperListener:
// -----------------------------------



#endif  // DUMPERLISTENER_H
