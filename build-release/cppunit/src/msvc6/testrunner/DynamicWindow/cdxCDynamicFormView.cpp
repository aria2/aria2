// cdxCDynamicFormView.cpp : implementation file
//

#include "stdafx.h"
#include "cdxCDynamicFormView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicFormView
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCDynamicFormView, CFormView)

/////////////////////////////////////////////////////////////////////////////
// creation
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicFormView, CFormView)
	//{{AFX_MSG_MAP(cdxCDynamicFormView)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	ON_WM_PARENTNOTIFY()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicFormView message handlers
/////////////////////////////////////////////////////////////////////////////

/*
 * OnInitialUpdate()
 * -----------------
 * These functions set up the form view.
 * New to this version is that the correct size will now be read
 * automatically.
 */

void cdxCDynamicFormView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	DoInitWindow(*this,GetTotalSize());
}

/////////////////////////////////////////////////////////////////////////////

BOOL cdxCDynamicFormView::DestroyWindow()
{
	DoOnDestroy();
	return CFormView::DestroyWindow();
}

void cdxCDynamicFormView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	DoOnSize(nType, cx, cy);
}

void cdxCDynamicFormView::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CFormView::OnSizing(fwSide, pRect);
	DoOnSizing(fwSide, pRect);
}

void cdxCDynamicFormView::OnTimer(UINT nIDEvent) 
{
	CFormView::OnTimer(nIDEvent);
	DoOnTimer(nIDEvent);
}

void cdxCDynamicFormView::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CFormView::OnGetMinMaxInfo(lpMMI);
	DoOnGetMinMaxInfo(lpMMI);
}

void cdxCDynamicFormView::OnParentNotify(UINT message, LPARAM lParam) 
{
	CFormView::OnParentNotify(message, lParam);
	DoOnParentNotify(message, lParam);
}

void cdxCDynamicFormView::OnDestroy() 
{
	DoOnDestroy();
	CFormView::OnDestroy();
}
