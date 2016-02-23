// cdxCDynamicBar.cpp : implementation file
//

#include "stdafx.h"
#include "cdxCDynamicBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCDynamicBarDlg,cdxCDynamicChildDlg);

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicBarDlg, cdxCDynamicChildDlg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg functions
/////////////////////////////////////////////////////////////////////////////

bool cdxCDynamicBarDlg::Create(cdxCDynamicBar *pBar)
{
	return cdxCDynamicChildDlg::Create(m_nID,(CWnd *)pBar) != FALSE;
}





/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBar
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCDynamicBar,CSizingControlBar);

/////////////////////////////////////////////////////////////////////////////
// construction
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicBar, CSizingControlBar)
	//{{AFX_MSG_MAP(cdxCDynamicBar)
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBar message handlers
/////////////////////////////////////////////////////////////////////////////

/*
 * create bar & dialog
 */

BOOL cdxCDynamicBar::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd,
        CSize sizeDefault, BOOL bHasGripper, UINT nID,
        DWORD dwStyle)
{
	if(!( CSizingControlBar::Create(	lpszWindowName,
												pParentWnd,
												sizeDefault,
												bHasGripper,
												nID,
												dwStyle|WS_CLIPCHILDREN) ))
	{
		ASSERT(false);
		return FALSE;
	}

	if(!( m_rDlg.Create(this) ))
	{
		DestroyWindow();
		ASSERT(false);
		return FALSE;
	}

	ASSERT(::IsWindow(m_hWnd));
	ASSERT(m_rDlg.IsWindow());
	ASSERT(!m_rectBorder.IsRectNull());

	// the following code will even be provided by 
	m_szMin.cx				=	m_rectBorder.left + m_rectBorder.right;
	m_szMin.cy				=	m_rectBorder.top  + m_rectBorder.bottom;
	m_szMin					+=	m_rDlg.m_szMin;

	return TRUE;
}
		  
/*
 * route command UI updates to dialog
 */

void cdxCDynamicBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CSizingControlBar::OnUpdateCmdUI(pTarget,bDisableIfNoHndler);
	if(m_rDlg.IsWindow())
		m_rDlg.OnUpdateCmdUI(pTarget,bDisableIfNoHndler);
}

/*
 * when sizing starts, we'll force the super-duper anti-flickering mode :
 */

void cdxCDynamicBar::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CSizingControlBar::OnSizing(fwSide, pRect);
	m_rDlg.StartAntiFlickering((fwSide == WMSZ_BOTTOM) ||
										(fwSide == WMSZ_BOTTOMRIGHT) ||
										(fwSide == WMSZ_RIGHT));
}

/*
 * let my dialog cover the entire area
 */

void cdxCDynamicBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBar::OnSize(nType, cx, cy);
	if(::IsWindow(m_hWnd) && m_rDlg.IsWindow() && (nType != SIZE_MINIMIZED))
	{
		m_rDlg.SetWindowPos(	NULL,0,0,cx,cy,	SWP_NOACTIVATE|
															SWP_NOOWNERZORDER|
															SWP_NOZORDER);
															
	}	
}

/*
 * OnNcCalcSize() is used to calculate the optimum
 * min size for the dialog.
 */

void cdxCDynamicBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	m_rectBorder			=	lpncsp->rgrc[0];		// load initial rectangle

	CSizingControlBar::OnNcCalcSize(bCalcValidRects, lpncsp);

	ASSERT(m_rectBorder.left <= lpncsp->rgrc[0].left);
	ASSERT(m_rectBorder.top <= lpncsp->rgrc[0].top);
	ASSERT(m_rectBorder.right >= lpncsp->rgrc[0].right);
	ASSERT(m_rectBorder.bottom >= lpncsp->rgrc[0].bottom);

	m_rectBorder.left		=	lpncsp->rgrc[0].left - m_rectBorder.left;
	m_rectBorder.top		=	lpncsp->rgrc[0].top  - m_rectBorder.top;
	m_rectBorder.right	=	m_rectBorder.right  - lpncsp->rgrc[0].right;
	m_rectBorder.bottom	=	m_rectBorder.bottom - lpncsp->rgrc[0].bottom;
	m_szMin					=	m_rDlg.m_szMin;
	m_szMin.cx				+=	m_rectBorder.left + m_rectBorder.right;
	m_szMin.cy				+=	m_rectBorder.top  + m_rectBorder.bottom;
}

/*
 * route commands a long...
 */

BOOL cdxCDynamicBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if(m_rDlg.IsWindow() && m_rDlg.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	return CSizingControlBar::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/*
 * route commands ...
 */

BOOL cdxCDynamicBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(m_rDlg.IsWindow() && m_rDlg.OnCommand(wParam, lParam))
		return TRUE;
	return CSizingControlBar::OnCommand(wParam, lParam);
}
