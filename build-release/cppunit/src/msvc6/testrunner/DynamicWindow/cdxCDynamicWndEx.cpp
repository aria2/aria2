// cdxCDynamicWndEx.cpp: implementation of the cdxCDynamicWndEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cdxCDynamicWndEx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Some static variables (taken from cdxCDynamicControlsManager)
/////////////////////////////////////////////////////////////////////////////

#define	REGVAL_NOSTATE		-1
#define	REGVAL_VISIBLE		1
#define	REGVAL_HIDDEN		0
#define	REGVAL_MAXIMIZED	1
#define	REGVAL_ICONIC		0
#define	REGVAL_INVALID		0
#define	REGVAL_VALID		1

/*
 * registry value names
 * (for StoreWindowPosition()/RestoreWindowPosition())
 */

static LPCTSTR	lpszRegVal_Left		=	_T("Left"),
					lpszRegVal_Right		=	_T("Right"),
					lpszRegVal_Top			=	_T("Top"),
					lpszRegVal_Bottom		=	_T("Bottom"),
					lpszRegVal_Visible	=	_T("Visibility"),
					lpszRegVal_State		=	_T("State"),
					lpszRegVal_Valid		=	_T("(valid)");

LPCTSTR	cdxCDynamicWndEx::M_lpszAutoPosProfileSection	=	_T("WindowPositions");

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicWndEx
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicWndEx stretches windows
/////////////////////////////////////////////////////////////////////////////

static inline CString _makeFullProfile(LPCTSTR lpszBase, const CString & str)
{
	CString	s	=	lpszBase;

	if(s.GetLength() && (s[s.GetLength()-1] != _T('\\')))
		s	+=	_T('\\');

	s	+=	str;
	return s;
}

void cdxCDynamicWndEx::OnInitialized()
{
	ASSERT(IsWindow());

	if(!m_strAutoPos.IsEmpty())
	{
#if _MSC_VER < 1300   // vc6
		if(!RestoreWindowPosition(_makeFullProfile(M_lpszAutoPosProfileSection,m_strAutoPos),rflg_all))
#else                 // vc7
		if(!RestoreWindowPosition(_makeFullProfile(M_lpszAutoPosProfileSection,m_strAutoPos),"",rflg_all))
#endif
		{
			Window()->CenterWindow();
			StretchWindow(10);
		}
	}
}

void cdxCDynamicWndEx::OnDestroying()
{
	if(!m_strAutoPos.IsEmpty() && IsWindow())
		StoreWindowPosition(_makeFullProfile(M_lpszAutoPosProfileSection,m_strAutoPos));
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicWndEx stretches windows
/////////////////////////////////////////////////////////////////////////////

/*
 * stretches the window by szDelta (i.e. if szDelta is 100, the window is enlarged by 100 pixels)
 * stretching means that the center point of the window remains
 *
 * returns false if the window would be smaller than (1,1)
 *
 * NOTE: this function does NOT care of the min/max dimensions of a window
 *			Use MoveWindow() if you need to take care of it.
 *
 * STATIC
 */

bool cdxCDynamicWndEx::StretchWindow(const CSize & szDelta)
{
	if(!IsWindow())
	{
		ASSERT(false);
		return false;
	}

	CWnd	*pWnd	=	Window();

	WINDOWPLACEMENT	wpl;
	pWnd->GetWindowPlacement(&wpl);

	wpl.rcNormalPosition.left		-=	szDelta.cx / 2;
	wpl.rcNormalPosition.right		+=	(szDelta.cx + 1) / 2;
	wpl.rcNormalPosition.top		-=	szDelta.cy / 2;
	wpl.rcNormalPosition.bottom	+=	(szDelta.cy + 1) / 2;
//	wpl.flags	=	SW_SHOWNA|SW_SHOWNOACTIVATE;

	if((wpl.rcNormalPosition.left >= wpl.rcNormalPosition.right) ||
		(wpl.rcNormalPosition.top >= wpl.rcNormalPosition.bottom))
		return false;

	VERIFY( pWnd->SetWindowPos(NULL,
										wpl.rcNormalPosition.left,
										wpl.rcNormalPosition.top,
										wpl.rcNormalPosition.right - wpl.rcNormalPosition.left,
										wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top,
										SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER) );

	return true;
}

/*
 * stretch window by a percent value
 * the algorithm calculates the new size for both dimensions by:
 *
 *  newWid = oldWid + (oldWid * iAddPcnt) / 100
 *
 * NOTE: iAddPcnt may even be nagtive, but it MUST be greater than -100.
 * NOTE: this function does NOT care of the min/max dimensions of a window
 *
 * The function will return false if the new size would be empty.
 */

bool cdxCDynamicWndEx::StretchWindow(int iAddPcnt)
{
	if(!IsWindow())
	{
		ASSERT(false);
		return false;
	}

	CSize	szDelta	=	GetCurrentClientSize() + GetBorderSize();

	szDelta.cx	=	(szDelta.cx * iAddPcnt) / 100;
	szDelta.cy	=	(szDelta.cy * iAddPcnt) / 100;

	return StretchWindow(szDelta);
}


/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicWndEx registry positioning
/////////////////////////////////////////////////////////////////////////////

/*
 * stores a window's position and visiblity to the registry.
 *	return false if any error occured
 */

bool cdxCDynamicWndEx::StoreWindowPosition(LPCTSTR lpszProfile, 
                                           const CString &entryPrefix)
{
	if(!IsWindow() || !lpszProfile || !*lpszProfile)
	{
		ASSERT(false);
		return false;
	}

	CWnd	*pWnd	=	Window();

	WINDOWPLACEMENT	wpl;
	VERIFY( pWnd->GetWindowPlacement(&wpl) );

	BOOL	bVisible	=	pWnd->IsWindowVisible();
	int	iState	=	REGVAL_NOSTATE;

	if(pWnd->IsIconic())
		iState	=	REGVAL_ICONIC;
	else
		if(pWnd->IsZoomed())
			iState	=	REGVAL_MAXIMIZED;

	CWinApp	*app	=	AfxGetApp();

	if(!app->m_pszRegistryKey || !*app->m_pszRegistryKey)
	{
		TRACE(_T("*** NOTE[cdxCDynamicWndEx::StoreWindowPosition()]: To properly store and restore a window's position, please call CWinApp::SetRegistryKey() in you app's InitInstance() !\n"));
		return false;
	}

	return	app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Valid,	REGVAL_INVALID) &&	// invalidate first
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Left,		wpl.rcNormalPosition.left) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Right,		wpl.rcNormalPosition.right) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Top,		wpl.rcNormalPosition.top) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Bottom,	wpl.rcNormalPosition.bottom) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Visible,	bVisible ? REGVAL_VISIBLE : REGVAL_HIDDEN) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_State,		iState) &&
				app->WriteProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Valid,	REGVAL_VALID);		// validate position
}

/*
 * load the registry data stored by StoreWindowPosition()
 * returns true if data have been found in the registry
 */

bool cdxCDynamicWndEx::RestoreWindowPosition(LPCTSTR lpszProfile, 
                                             const CString &entryPrefix, 
                                             UINT restoreFlags)
{
	if(!IsWindow() || !lpszProfile || !*lpszProfile)
	{
		ASSERT(false);
		return false;
	}

	CWnd		*pWnd	=	Window();
	CWinApp	*app	=	AfxGetApp();

	if(!app->m_pszRegistryKey || !*app->m_pszRegistryKey)
	{
		TRACE(_T("*** NOTE[cdxCDynamicWndEx::RestoreWindowPosition()]: To properly store and restore a window's position, please call CWinApp::SetRegistryKey() in you app's InitInstance() !\n"));
		return false;
	}

	//
	// first, we check whether the position had been saved successful any time before
	//

	if( app->GetProfileInt(lpszProfile,entryPrefix+lpszRegVal_Valid,REGVAL_INVALID) != REGVAL_VALID )
		return false;

	//
	// get old position
	//

	WINDOWPLACEMENT	wpl;
	VERIFY( pWnd->GetWindowPlacement(&wpl) );

	//
	// read registry
	//

	int	iState	=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_State, REGVAL_NOSTATE);

	//
	// get window's previous normal position
	//

	wpl.rcNormalPosition.left		=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Left,		wpl.rcNormalPosition.left);
	wpl.rcNormalPosition.right		=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Right,		wpl.rcNormalPosition.right);
	wpl.rcNormalPosition.top		=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Top,		wpl.rcNormalPosition.top);
	wpl.rcNormalPosition.bottom	=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Bottom,	wpl.rcNormalPosition.bottom);

	if(wpl.rcNormalPosition.left > wpl.rcNormalPosition.right)
	{
		long	l	=	wpl.rcNormalPosition.right;
		wpl.rcNormalPosition.right	=	wpl.rcNormalPosition.left;
		wpl.rcNormalPosition.left	=	l;
	}
	if(wpl.rcNormalPosition.top > wpl.rcNormalPosition.bottom)
	{
		long	l	=	wpl.rcNormalPosition.bottom;
		wpl.rcNormalPosition.bottom	=	wpl.rcNormalPosition.top;
		wpl.rcNormalPosition.top	=	l;
	}

	//
	// get restore stuff
	//

	UINT	showCmd	=	SW_SHOWNA;
	
	if(restoreFlags & rflg_state)
	{
		if(iState == REGVAL_MAXIMIZED)
			showCmd	=	SW_MAXIMIZE;
		else
			if(iState == REGVAL_ICONIC)
				showCmd	=	SW_MINIMIZE;
	}

	//
	// use MoveWindow() which takes care of WM_GETMINMAXINFO
	//

	pWnd->MoveWindow(	wpl.rcNormalPosition.left,wpl.rcNormalPosition.top,
							wpl.rcNormalPosition.right - wpl.rcNormalPosition.left,
							wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top,
							showCmd == SW_SHOWNA);

	if(showCmd != SW_SHOWNA)
	{
		// read updated position

		VERIFY( pWnd->GetWindowPlacement(&wpl) );
		wpl.showCmd	=	showCmd;
		pWnd->SetWindowPlacement(&wpl);
	}
	
	//
	// get visiblity
	//

	if(restoreFlags & rflg_visibility)
	{
		int	i	=	app->GetProfileInt(lpszProfile,	entryPrefix+lpszRegVal_Visible, REGVAL_NOSTATE);
		if(i == REGVAL_VISIBLE)
			pWnd->ShowWindow(SW_SHOW);
		else
			if(i == REGVAL_HIDDEN)
				pWnd->ShowWindow(SW_HIDE);
	}

	return true;
}

