// TestPlugInRunner.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TestPlugInRunnerApp.h"
#include "TestPlugInRunnerDlg.h"
#include "TestPlugInRunnerModel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HINSTANCE g_testRunnerResource;

/////////////////////////////////////////////////////////////////////////////
// TestPlugInRunnerApp

BEGIN_MESSAGE_MAP(TestPlugInRunnerApp, CWinApp)
	//{{AFX_MSG_MAP(TestPlugInRunnerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TestPlugInRunnerApp construction

TestPlugInRunnerApp::TestPlugInRunnerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only TestPlugInRunnerApp object

TestPlugInRunnerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// TestPlugInRunnerApp initialization

BOOL TestPlugInRunnerApp::InitInstance()
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

  g_testRunnerResource = AfxGetResourceHandle();

  SetRegistryKey(_T("CppUnit Test Plug-In Runner"));

  {
  TestPlugInRunnerModel model;
	TestPlugInRunnerDlg dlg( &model );
	m_pMainWnd = &dlg;
	dlg.DoModal();

  }
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
