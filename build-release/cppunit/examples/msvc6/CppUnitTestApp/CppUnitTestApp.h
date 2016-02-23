// CppUnitTestApp.h : main header file for the CPPUNITTESTAPP application
//

#if !defined(AFX_CPPUNITTESTAPP_H__6569C745_ED89_4902_9794_AD8422583BC1__INCLUDED_)
#define AFX_CPPUNITTESTAPP_H__6569C745_ED89_4902_9794_AD8422583BC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CppUnitTestApp:
// See CppUnitTestApp.cpp for the implementation of this class
//

class CppUnitTestApp : public CWinApp
{
public:
	CppUnitTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CppUnitTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CppUnitTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  void RunTests();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPPUNITTESTAPP_H__6569C745_ED89_4902_9794_AD8422583BC1__INCLUDED_)
