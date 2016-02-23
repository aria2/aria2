// //////////////////////////////////////////////////////////////////////////
// Header file TestRunner.h for class TestRunner
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/19
// //////////////////////////////////////////////////////////////////////////
#ifndef CPPUNIT_QTUI_QTTESTRUNNER_H
#define CPPUNIT_QTUI_QTTESTRUNNER_H

#include <cppunit/portability/CppUnitVector.h>
#include "Config.h"

CPPUNIT_NS_BEGIN


  class Test;
  class TestSuite;


/*! 
 * \brief QT test runner.
 * \ingroup ExecutingTest
 *
 * Here is an example of usage:
 * \code
 * #include <cppunit/extensions/TestFactoryRegistry.h>
 * #include <cppunit/ui/qt/TestRunner.h>
 *
 * [...]
 *
 * void 
 * QDepWindow::runTests()
 * {
 *   CppUnit::QtUi::TestRunner runner;
 *   runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
 *   runner.run( true );
 * }
 * \endcode
 *
 */
class QTTESTRUNNER_API QtTestRunner
{
public:
  /*! Constructs a TestRunner object.
   */
  QtTestRunner();

  /*! Destructor.
   */
  virtual ~QtTestRunner();

  void run( bool autoRun =false );

  void addTest( Test *test );

private:
  /// Prevents the use of the copy constructor.
  QtTestRunner( const QtTestRunner &copy );

  /// Prevents the use of the copy operator.
  void operator =( const QtTestRunner &copy );

  Test *getRootTest();

private:
  typedef CppUnitVector<Test *> Tests;
  Tests *_tests;

  TestSuite *_suite;
};


#if CPPUNIT_HAVE_NAMESPACES
  namespace QtUi
  {
    /*! Qt TestRunner (DEPRECATED).
     * \deprecated Use CppUnit::QtTestRunner instead.
     */
    typedef CPPUNIT_NS::QtTestRunner TestRunner;
  }
#endif


CPPUNIT_NS_END

#endif  // CPPUNIT_QTUI_QTTESTRUNNER_H
