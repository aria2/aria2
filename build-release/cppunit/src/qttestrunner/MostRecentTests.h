// //////////////////////////////////////////////////////////////////////////
// Header file MostRecentTests.h for class MostRecentTests
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////
#ifndef MOSTRECENTTESTS_H
#define MOSTRECENTTESTS_H

#include <cppunit/Test.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qobject.h>


/*! \class MostRecentTests
 * \brief This class represents the list of the recent tests.
 */
class MostRecentTests : public QObject
{
  Q_OBJECT
public:
  /*! Constructs a MostRecentTests object.
   */
  MostRecentTests();

  /*! Destructor.
   */
  virtual ~MostRecentTests();

  void setTestToRun( CPPUNIT_NS::Test *test );
  CPPUNIT_NS::Test *testToRun();

  int testCount();
  QString testNameAt( int index );
  CPPUNIT_NS::Test *testAt( int index );

signals:
  void listChanged();
  void testToRunChanged( CPPUNIT_NS::Test *testToRun );

public slots:
  void selectTestToRun( int index );

private:
  /// Prevents the use of the copy constructor.
  MostRecentTests( const MostRecentTests &copy );

  /// Prevents the use of the copy operator.
  void operator =( const MostRecentTests &copy );

private:
  QList<CPPUNIT_NS::Test> m_tests;
};



// Inlines methods for MostRecentTests:
// ------------------------------------



#endif  // MOSTRECENTTESTS_H
