#ifndef CPPUNITUI_MFC_MFCTESTRUNNER_H
#define CPPUNITUI_MFC_MFCTESTRUNNER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <cppunit/Portability.h>
#include <cppunit/portability/CppUnitVector.h>

/* Refer to MSDN documentation to know how to write and use MFC extension DLL:
   mk:@MSITStore:h:\DevStudio\MSDN\98VSa\1036\vcmfc.chm::/html/_mfcnotes_tn033.htm#_mfcnotes_how_to_write_an_mfc_extension_dll
   
   This can be found in the index with "mfc extension"
   The basic:
   Using:
   - your application must use MFC DLL
   - memory allocation is done using the same heap
   - you must define the symbol _AFX_DLL

   Building:
   - you must define the symbol _AFX_DLL and _AFX_EXT
   - export class using AFX_EXT_CLASS
 */

CPPUNIT_NS_BEGIN

  class Test;
  class TestSuite;


/*! \brief MFC test runner.
 * \ingroup ExecutingTest
 *
 * Use this to launch the MFC TestRunner. Usually called from you CWinApp subclass:
 *
 * \code
 * #include <cppunit/ui/mfc/MfcTestRunner.h>
 * #include <cppunit/extensions/TestFactoryRegistry.h>
 *
 * void 
 * CHostAppApp::RunUnitTests()
 * {
 *   CppUnit::MfcTestRunner runner;
 *   runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
 *
 *   runner.run();    
 * }
 * \endcode
 * \see CppUnit::TextTestRunner, CppUnit::TestFactoryRegistry.
 */
class AFX_EXT_CLASS MfcTestRunner
{
public:
  MfcTestRunner();
  virtual ~MfcTestRunner();

  void run();

  void addTest( Test *test );

  void addTests( const CppUnitVector<Test *> &tests );

protected:
  Test *getRootTest();

  TestSuite *m_suite;

  typedef CppUnitVector<Test *> Tests;
  Tests m_tests;
};


CPPUNIT_NS_END

#endif // CPPUNITUI_MFC_MFCTESTRUNNER_H