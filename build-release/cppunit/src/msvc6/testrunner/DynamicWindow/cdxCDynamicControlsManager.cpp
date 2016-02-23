// cdxCDynamicControlsManager.cpp: implementation of the cdxCDynamicControlsManager class.
//
//////////////////////////////////////////////////////////////////////

/*
 * you should define OEMRESOURCE
 * in your project settings (C/C++, General) !
 */

#include "stdafx.h"
#include "cdxCDynamicControlsManager.h"

#include	<winuser.h>
#include	<afxmt.h>

#ifndef OBM_SIZE
#define	OBM_SIZE		32766
#pragma message("*** NOTE: cdxCDynamicControlsManager.cpp: Please define OEMRESOURCE in your project settings !")
// taken from WinresRc.h
// if not used for any reason
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Some static variables
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

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::ControlData::ControlEntry
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////////////

cdxCDynamicControlsManager::ControlData::ControlEntry::ControlEntry(CWnd & ctrl, ControlData & rMaster)
:	m_rMaster(rMaster), m_rCtrl(ctrl)
{
	if(m_pNext = rMaster.m_pCtrl)
		m_pNext->m_pPrev	=	this;
	rMaster.m_pCtrl	=	this;
	m_pPrev				=	NULL;

	// raise total counter
	++rMaster.m_rMaster.m_iTotalCnt;
}

cdxCDynamicControlsManager::ControlData::ControlEntry::~ControlEntry()
{
	if(m_pPrev)
	{
		if(m_pPrev->m_pNext = m_pNext)
			m_pNext->m_pPrev	=	m_pPrev;
	}
	else
	{
		ASSERT( m_rMaster.m_pCtrl == this );
		if(m_rMaster.m_pCtrl = m_pNext)
			m_pNext->m_pPrev	=	NULL;
	}

	// lower
	ASSERT( m_rMaster.m_rMaster.m_iTotalCnt > 0 );
	++m_rMaster.m_rMaster.m_iTotalCnt;
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::ControlData
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////////////

/*
 * constructor
 * copies all paramaters and gets the controls initial position using
 * GetWindowPos().
 *
 * NOTE that the constructor need ctrl.m_hWnd to exist (in contrast to
 * Add()
 */

cdxCDynamicControlsManager::ControlData::ControlData(cdxCDynamicControlsManager & rMaster, CWnd & ctrl, const PositionSetup & rPosSetup)
:	m_rMaster(rMaster), m_pCtrl(NULL),
	m_pNext(NULL), m_pPrev(NULL),
	m_posSetup(rPosSetup)
{
	ASSERT(::IsWindow(ctrl.m_hWnd));		// control must have already been created !
	ASSERT(rPosSetup.IsValid());

	//
	// get initial values
	//

	WINDOWPLACEMENT	wpl;
	VERIFY( ctrl.GetWindowPlacement(&wpl) );

	m_rectOriginal	=	wpl.rcNormalPosition;

	//
	// remember control
	//

	new ControlEntry(ctrl,*this);
	ASSERT(m_pCtrl != NULL);

	//
	// link us to the cdxCDynamicControlsManager's list
	//

	if(m_pNext = m_rMaster.m_pFirst)
		m_pNext->m_pPrev	=	this;
	m_pPrev	=	NULL;
	m_rMaster.m_pFirst	=	this;
}

/*
 * detach from list
 * The m_Ctrl deletes all children by itself
 */

cdxCDynamicControlsManager::ControlData::~ControlData()
{
	//
	// delete all control references
	//

	while(m_pCtrl)
		delete m_pCtrl;

	//
	// unlink from list
	//

	if(m_pPrev)
	{
		if(m_pPrev->m_pNext = m_pNext)
			m_pNext->m_pPrev	=	m_pPrev;
	}
	else
	{
		ASSERT(m_rMaster.m_pFirst == this);
		if(m_rMaster.m_pFirst = m_pNext)
			m_pNext->m_pPrev	=	NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::ControlData virtuals
/////////////////////////////////////////////////////////////////////////////

/*
 * checks whether the CWnd is part of this control data
 */

bool cdxCDynamicControlsManager::ControlData::IsMember(CWnd & ctrl) const
{
	for(const ControlEntry *pEntry = m_pCtrl; pEntry; pEntry = pEntry->GetNext())
		if(*pEntry == ctrl)
			return true;

	return false;
}

/*
 * removes a CWnd from this chain
 */

bool cdxCDynamicControlsManager::ControlData::Rem(CWnd & ctrl)
{
	for(ControlEntry *pEntry = m_pCtrl; pEntry; pEntry = pEntry->GetNext())
		if(*pEntry == ctrl)
		{
			delete pEntry;
			return true;
		}

	return false;
}

/*
 * Get current position
 */

CRect cdxCDynamicControlsManager::ControlData::GetCurrentPosition() const
{
	if(!IsUsed())
	{
		ASSERT(false);			// all sub-controls have been deleted
		return CRect(0,0,0,0);
	}

	WINDOWPLACEMENT	wpl;
	VERIFY( m_pCtrl->GetCWnd().GetWindowPlacement(&wpl) );
	return CRect(wpl.rcNormalPosition);
}

/*
 * modify initial setup
 * NOTE: this function does not move the controls.
 * You habe to call cdxCDynamicControlsManager::ReorganizeControls past
 * using this function
 */

bool cdxCDynamicControlsManager::ControlData::Modify(const CRect & rectOriginal, const PositionSetup & rSetup)
{
	if((rectOriginal.left > rectOriginal.right) ||
		(rectOriginal.top > rectOriginal.bottom) ||
		!rSetup.IsValid())
	{
		ASSERT(false);			// bad function call
		return false;
	}

	m_rectOriginal	=	rectOriginal;
	m_posSetup		=	rSetup;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// handling events from CWnd
/////////////////////////////////////////////////////////////////////////////

/*
 * this function initializes the following members:
 * m_pWnd			-	the window handle
 * m_szCurrent		-	the current window's size
 * m_szMin			-	the minimum window's size (taken from current size)
 * m_szMax			-	the maximum window's size (set to (0,0) <=> don't change maximum)
 * m_wndSizeIcon	-	the icon (if wanted)
 *
 * parameters:
 * rWnd				-	the window to supervise
 * fd					-	in which directions can we size the window (does only apply to user-sizing)
 * bSizeIcon		-	do you want a sizable icon ?
 * pBaseClientSize-	if non-zero, this defines the real (normal) size of the
 *							window relative to all furher calculations will be made.
 *							if zero, the current window's size will be taken.
 */

void cdxCDynamicControlsManager::DoInitWindow(CWnd & rWnd, Freedom fd, bool bSizeIcon, const CSize * pBaseClientSize)
{
	ASSERT(m_pWnd == NULL);					// you MUST NOT call this function twice !
	ASSERT(::IsWindow(rWnd.m_hWnd));		// rWnd MUST already exist !!

	m_pWnd		=	&rWnd;
	m_Freedom	=	fd;

	//
	// get current's window size
	//

	CRect			rect;
	m_pWnd->GetWindowRect(&rect);
	CRect			rectClient;
	m_pWnd->GetClientRect(&rectClient);

	if(!pBaseClientSize)
	{
		m_szClientRelative.cx	=	rectClient.Width();
		m_szClientRelative.cy	=	rectClient.Height();

		m_szMin.cx	=	rect.Width();
		m_szMin.cy	=	rect.Height();
	}
	else
	{
		ASSERT((pBaseClientSize->cx > 0) && (pBaseClientSize->cy > 0));

		m_szClientRelative	=	*pBaseClientSize;
		m_szMin.cx				=	m_szClientRelative.cx + (rect.Width() - rectClient.Width());
		m_szMin.cy				=	m_szClientRelative.cy + (rect.Height() - rectClient.Height());
	}

	m_szMax.cx	=	0;
	m_szMax.cy	=	0;

	//
	// set up icon if wanted
	//

	if(bSizeIcon)
	{
		VERIFY( m_pWndSizeIcon = new cdxCSizeIconCtrl );
		VERIFY( m_pWndSizeIcon->Create(m_pWnd,m_idSizeIcon) );	// creates my control; id is SIZE_CONTROL_ID

		AddSzControl(*m_pWndSizeIcon,mdRepos,mdRepos);
		m_pWndSizeIcon->ShowWindow(SW_SHOW);		// finally - show it
	}
}

/////////////////////////////////////////////////////////////////////////////

/*
 * fill in MINMAXINFO as requested
 * Call your CWnd's OnGetMinMaxInfo first !
 *
 * changed due to a but report by Michel Wassink <mww@mitutoyo.nl>
 */

void cdxCDynamicControlsManager::DoOnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if(IsReady() && !IsDisabled())
	{
		lpMMI->ptMinTrackSize.x	=	m_szMin.cx;
		lpMMI->ptMinTrackSize.y	=	m_szMin.cy;

		if(m_Freedom & fdHoriz)
		{
			if(m_szMax.cx)
				lpMMI->ptMaxTrackSize.x	=	m_szMax.cx;
		}
		else
			lpMMI->ptMaxTrackSize.x	=	m_szMin.cx;

		if(m_Freedom & fdVert)
		{
			if(m_szMax.cy)
				lpMMI->ptMaxTrackSize.y	=	m_szMax.cy;
		}
		else
			lpMMI->ptMaxTrackSize.y	=	m_szMin.cy;
	}
}

/*
 * handle OnSize - we can't rely on cx,cy being client dimensions
 * as stated in the documentation ...
 */

void cdxCDynamicControlsManager::DoOnSize(UINT nType, int cx, int cy)
{
	if(IsReady() && (nType != SIZE_MINIMIZED) && !IsDisabled())
		ReorganizeControls(true);
}

/*
 * free all memory
 * after having called this function, you can reuse the object
 * although I wouldn't recommend to do so :)
 */

void cdxCDynamicControlsManager::DoDestroyWindow()
{
	while(m_pFirst)
	{
		OnDeleteControlPosition(*m_pFirst);
		delete m_pFirst;
	}

	if(m_pWndSizeIcon)
	{
		if(::IsWindow(m_pWndSizeIcon->m_hWnd))
			m_pWndSizeIcon->DestroyWindow();
		delete m_pWndSizeIcon;
		m_pWndSizeIcon	=	NULL;
	}

	m_pWnd = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// control positioning
/////////////////////////////////////////////////////////////////////////////

/*
 * Reposition (without current rectangle size)
 *
 * rectWin		-	window rectangle (including border)
 * rectClient	-	window rectangle (client area)
 *                Note that since release 6, rectClient.left and .top might be > zero
 *					   (for scrolling)
 * bRedraw		-	invalidate & update window
 */

void cdxCDynamicControlsManager::ReorganizeControlsAdvanced(const CRect & rectWin, CRect rectClient, bool bRedraw)
{
	ASSERT(IsReady());

	if(!GetTotalChildCnt())
		return;

	//
	// we won't go smaller with the whole window than
	// m_szMin
	//

	if(rectWin.Width() < m_szMin.cx)
		rectClient.right	+=	(m_szMin.cx - rectWin.Width());
	if(rectWin.Height() < m_szMin.cy)
		rectClient.bottom	+=	(m_szMin.cy - rectWin.Height());

	//
	// we new replace all controls
	//

	CSize	szDelta;
	szDelta.cx	=	rectClient.Width() - m_szClientRelative.cx;
	szDelta.cy	=	rectClient.Height() - m_szClientRelative.cy;

	CPoint	pntOffset	=	rectClient.TopLeft();

	// newly added code by Rodger Bernstein

	AFX_SIZEPARENTPARAMS layout;
	ControlData				*sz;
	bool						bManual	=	true;

	if(!( layout.hDWP = ::BeginDeferWindowPos(GetTotalChildCnt()) ))
	{
		TRACE(_T("*** ERROR[cdxCDynamicControlsManager::ReorganizeControlsAdvanced()]: BeginDeferWindowPos() failed.\n"));
	}
	else
	{
		for(sz = m_pFirst; sz; sz = sz->GetNext())
			sz->OnSize(szDelta, &layout, &pntOffset);

		if(!::EndDeferWindowPos(layout.hDWP))
		{
			TRACE(_T("*** ERROR[cdxCDynamicControlsManager::ReorganizeControlsAdvanced()]: EndDeferWindowPos() failed.\n"));
		}
		else
			bManual	=	false;
	}

	if(bManual)
		for(sz = m_pFirst; sz; sz = sz->GetNext())
			sz->OnSize(szDelta, NULL);

	if(bRedraw && m_pWnd->IsWindowVisible())
	{
		m_pWnd->RedrawWindow(NULL, NULL, RDW_UPDATENOW | RDW_NOERASE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// misc
/////////////////////////////////////////////////////////////////////////////

/*
 * change minimum and maximum height of window.
 *
 * szMin						-	new minimum size (use GetMinSize() to leave it as being before)
 * szMax						-	new maximum size ( "  GetMaxSize() ")
 *									Set to CSize(0,0) if you don't want a maximum size.
 * bResizeIfNecessary	-	call FixWindowSize() past calculating new sizes.
 *
 * returns false if szMin and szMax are illegal (e.g. szMin > szMax)
 */

bool cdxCDynamicControlsManager::SetMinMaxSize(const CSize & szMin, const CSize & szMax, bool bResizeIfNecessary)
{
	ASSERT(IsReady());			// DoInitWindow() not called ?

	if((szMax.cx && (szMin.cx > szMax.cx)) ||
		(szMax.cy && (szMin.cy > szMax.cy)))
	{
		return false;
	}

	m_szMin	=	szMin;
	m_szMax	=	szMax;

	if(bResizeIfNecessary)
		FixWindowSize();

	return true;
}

/*
 * this function ensure that the window's size is between m_szMin and m_szMax.
 * returns true if window size has been changed
 */

bool cdxCDynamicControlsManager::FixWindowSize()
{
	ASSERT(IsReady());			// use DoInitWindow() first !

	CSize	szCurrent	=	GetWindowSize(*m_pWnd),
			szDelta;

	if(m_szMax.cx && (szCurrent.cx > m_szMax.cx))
		szDelta.cx		=	m_szMax.cx - szCurrent.cx;		// is negative
	else
		if(szCurrent.cx < m_szMin.cx)
			szDelta.cx	=	m_szMin.cx - szCurrent.cx;		// is positive
		else
			szDelta.cx	=	0;

	if(m_szMax.cy && (szCurrent.cy > m_szMax.cy))
		szDelta.cy		=	m_szMax.cy - szCurrent.cy;		// is negative
	else
		if(szCurrent.cy < m_szMin.cy)
			szDelta.cy	=	m_szMin.cy - szCurrent.cy;		// is positive
		else
			szDelta.cy	=	0;

	if(!szDelta.cx && !szDelta.cy)
		return false;			// nothing to do

	StretchWindow(*m_pWnd,szDelta);
	return true;
}

/////////////////////////////////////////////////////////////////////////////

/*
 * hide and show icon
 */

void cdxCDynamicControlsManager::HideSizeIcon()
{
	if(m_pWndSizeIcon && ::IsWindow(m_pWndSizeIcon->m_hWnd))
		m_pWndSizeIcon->ShowWindow(SW_HIDE);
}

void cdxCDynamicControlsManager::ShowSizeIcon()
{
	if(m_pWndSizeIcon && ::IsWindow(m_pWndSizeIcon->m_hWnd))
		m_pWndSizeIcon->ShowWindow(SW_SHOW);
}

/////////////////////////////////////////////////////////////////////////////
// static functions: window sizing
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

bool cdxCDynamicControlsManager::StretchWindow(CWnd & rWnd, const CSize & szDelta)
{
	ASSERT(::IsWindow(rWnd.m_hWnd));

	WINDOWPLACEMENT	wpl;
	rWnd.GetWindowPlacement(&wpl);

	wpl.rcNormalPosition.left		-=	szDelta.cx / 2;
	wpl.rcNormalPosition.right		+=	(szDelta.cx + 1) / 2;
	wpl.rcNormalPosition.top		-=	szDelta.cy / 2;
	wpl.rcNormalPosition.bottom	+=	(szDelta.cy + 1) / 2;
//	wpl.flags	=	SW_SHOWNA|SW_SHOWNOACTIVATE;

	if((wpl.rcNormalPosition.left >= wpl.rcNormalPosition.right) ||
		(wpl.rcNormalPosition.top >= wpl.rcNormalPosition.bottom))
		return false;

	VERIFY( rWnd.SetWindowPos(	NULL,
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
 *
 * STATIC
 */

bool cdxCDynamicControlsManager::StretchWindow(CWnd & rWnd, int iAddPcnt)
{
	ASSERT(::IsWindow(rWnd.m_hWnd));

	CSize	szDelta	=	GetWindowSize(rWnd);

	szDelta.cx	=	(szDelta.cx * iAddPcnt) / 100;
	szDelta.cy	=	(szDelta.cy * iAddPcnt) / 100;

	return StretchWindow(rWnd,szDelta);
}

/*
 * get current window's size
 *
 * STATIC
 */

CSize cdxCDynamicControlsManager::GetWindowSize(CWnd & rWnd)
{
	ASSERT(::IsWindow(rWnd.m_hWnd));

	CRect	rect;
	rWnd.GetWindowRect(&rect);
	return CSize(rect.Width(),rect.Height());
}

/////////////////////////////////////////////////////////////////////////////
// static functions: window & registry
/////////////////////////////////////////////////////////////////////////////

/*
 * stores a window's position and visiblity to the registry.
 *
 *	return false if any error occured
 *
 * STATIC
 */

bool cdxCDynamicControlsManager::StoreWindowPosition(CWnd & rWnd, LPCTSTR lpszProfile)
{
	ASSERT(::IsWindow(rWnd.m_hWnd) && lpszProfile && *lpszProfile);
	// can't use an empty profile section string; see CWinApp::GetProfileInt() for further information

	WINDOWPLACEMENT	wpl;
	VERIFY( rWnd.GetWindowPlacement(&wpl) );
	BOOL	bVisible	=	rWnd.IsWindowVisible();
	int	iState	=	REGVAL_NOSTATE;

	if(rWnd.IsIconic())
		iState	=	REGVAL_ICONIC;
	else
		if(rWnd.IsZoomed())
			iState	=	REGVAL_MAXIMIZED;

	return	AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Valid,	REGVAL_INVALID) &&	// invalidate first
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Left,		wpl.rcNormalPosition.left) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Right,		wpl.rcNormalPosition.right) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Top,		wpl.rcNormalPosition.top) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Bottom,	wpl.rcNormalPosition.bottom) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Visible,	bVisible ? REGVAL_VISIBLE : REGVAL_HIDDEN) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_State,		iState) &&
				AfxGetApp()->WriteProfileInt(lpszProfile,	lpszRegVal_Valid,	REGVAL_VALID);		// validate position
}

/*
 * load the registry data stored by StoreWindowPosition()
 * returns true if data have been found in the registry
 *
 * STATIC
 */

bool cdxCDynamicControlsManager::RestoreWindowPosition(CWnd & rWnd, LPCTSTR lpszProfile, UINT restoreFlags)
{
	ASSERT(::IsWindow(rWnd.m_hWnd) && lpszProfile && *lpszProfile);
	// can't use an empty profile section string; see CWinApp::GetProfileInt() for further information

	//
	// first, we check whether the position had been saved successful any time before
	//

	if( AfxGetApp()->GetProfileInt(lpszProfile,lpszRegVal_Valid,REGVAL_INVALID) != REGVAL_VALID )
		return false;

	//
	// get old position
	//

	WINDOWPLACEMENT	wpl;
	VERIFY( rWnd.GetWindowPlacement(&wpl) );

	//
	// read registry
	//

	int	iState	=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_State, REGVAL_NOSTATE);

	//
	// get window's previous normal position
	//

	wpl.rcNormalPosition.left		=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_Left,		wpl.rcNormalPosition.left);
	wpl.rcNormalPosition.right		=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_Right,		wpl.rcNormalPosition.right);
	wpl.rcNormalPosition.top		=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_Top,		wpl.rcNormalPosition.top);
	wpl.rcNormalPosition.bottom	=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_Bottom,	wpl.rcNormalPosition.bottom);

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

	rWnd.MoveWindow(	wpl.rcNormalPosition.left,wpl.rcNormalPosition.top,
							wpl.rcNormalPosition.right - wpl.rcNormalPosition.left,
							wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top,
							showCmd == SW_SHOWNA);

	if(showCmd != SW_SHOWNA)
	{
		// read updated position

		VERIFY( rWnd.GetWindowPlacement(&wpl) );
		wpl.showCmd	=	showCmd;
		rWnd.SetWindowPlacement(&wpl);
	}
	
	//
	// get visiblity
	//

	if(restoreFlags & rflg_visibility)
	{
		int	i	=	AfxGetApp()->GetProfileInt(lpszProfile,	lpszRegVal_Visible, REGVAL_NOSTATE);
		if(i == REGVAL_VISIBLE)
			rWnd.ShowWindow(SW_SHOW);
		else
			if(i == REGVAL_HIDDEN)
				rWnd.ShowWindow(SW_HIDE);
	}

	return true;
}

