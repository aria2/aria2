// CppUnitTestApp.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CppUnitTestApp.h"
#include "CppUnitTestAppDlg.h"
#include <cppunit/ui/mfc/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CppUnitTestApp

BEGIN_MESSAGE_MAP(CppUnitTestApp, CWinApp)
	//{{AFX_MSG_MAP(CppUnitTestApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CppUnitTestApp construction

CppUnitTestApp::CppUnitTestApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CppUnitTestApp object

CppUnitTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CppUnitTestApp initialization

BOOL 
CppUnitTestApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
# if _MSC_VER < 1300   // vc6
	Enable3dControls();			// Call this when using MFC in a shared DLL
# endif
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  SetRegistryKey(_T("Local AppWizard-Generated Applications"));

  RunTests();        

  
  // Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


void 
CppUnitTestApp::RunTests()
{
  CPPUNIT_NS::MfcUi::TestRunner runner;

  runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );

  runner.run();
}
