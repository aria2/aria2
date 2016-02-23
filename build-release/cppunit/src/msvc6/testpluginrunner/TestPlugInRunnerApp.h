// TestPlugInRunner.h : main header file for the TESTPLUGINRUNNER application
//

#if !defined(AFX_TESTPLUGINRUNNER_H__C64A0384_27BB_4A9A_854C_B19BF07A81F8__INCLUDED_)
#define AFX_TESTPLUGINRUNNER_H__C64A0384_27BB_4A9A_854C_B19BF07A81F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// TestPlugInRunnerApp:
// See TestPlugInRunner.cpp for the implementation of this class
//

class TestPlugInRunnerApp : public CWinApp
{
public:
	TestPlugInRunnerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TestPlugInRunnerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(TestPlugInRunnerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPLUGINRUNNER_H__C64A0384_27BB_4A9A_854C_B19BF07A81F8__INCLUDED_)
