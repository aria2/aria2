// cdxCDynamicWnd.cpp: implementation of the cdxCDynamicWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cdxCDynamicWnd.h"
#include	"cdxCSizeIconCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4100)
#pragma warning(disable: 4706)


IMPLEMENT_DYNAMIC(cdxCDynamicLayoutInfo,CObject);

//////////////////////////////////////////////////////////////////////
// cdxCDynamicWnd::Position
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Positioning engine
//////////////////////////////////////////////////////////////////////

/*
 * Standard Controller's Position() routine
 * This has the same functionality as known from the former
 * cdxCDynamicControlsManager class.
 *
 * One exception is the new "szMin" property which allows
 * the class to "hide" the control if it becomes too small
 * (it will be moved outside the client area).
 */

void cdxCDynamicWnd::Position::Apply(HWND hwnd, CRect & rectNewPos, const cdxCDynamicLayoutInfo & li) const
{
	if(li.m_bUseScrollPos)
	{
		rectNewPos.left	=	left   - li.m_pntScrollPos.x;
		rectNewPos.right	=	right  - li.m_pntScrollPos.x;
		rectNewPos.top		=	top    - li.m_pntScrollPos.y;
		rectNewPos.bottom	=	bottom - li.m_pntScrollPos.y;

		if(li.m_szDelta.cx >= 0)
		{
			rectNewPos.left	+=	(m_Bytes[X1] * li.m_szDelta.cx) / 100;
			rectNewPos.right	+=	(m_Bytes[X2] * li.m_szDelta.cx) / 100;
		}
		if(li.m_szDelta.cy >= 0)
		{
			rectNewPos.top		+=	(m_Bytes[Y1] * li.m_szDelta.cy) / 100;
			rectNewPos.bottom	+=	(m_Bytes[Y2] * li.m_szDelta.cy) / 100;
		}
	}
	else
	{
		rectNewPos.left	=	left   + (m_Bytes[X1] * li.m_szDelta.cx) / 100;
		rectNewPos.right	=	right  + (m_Bytes[X2] * li.m_szDelta.cx) / 100;
		rectNewPos.top		=	top    + (m_Bytes[Y1] * li.m_szDelta.cy) / 100;
		rectNewPos.bottom	=	bottom + (m_Bytes[Y2] * li.m_szDelta.cy) / 100;
	}

	if(rectNewPos.left + m_szMin.cx >= rectNewPos.right)
	{
		rectNewPos.right	=	-10;
		rectNewPos.left	=	rectNewPos.right - m_szMin.cx;
	}
	if(rectNewPos.top + m_szMin.cy >= rectNewPos.bottom)
	{
		rectNewPos.bottom	=	-10;
		rectNewPos.top		=	rectNewPos.bottom - m_szMin.cy;
	}
}

//////////////////////////////////////////////////////////////////////
// cdxCDynamicWnd
//////////////////////////////////////////////////////////////////////

const CSize	cdxCDynamicWnd::M_szNull(0,0);
const cdxCDynamicWnd::SBYTES	cdxCDynamicWnd::TopLeft		=	{ 0,0,0,0 },
										cdxCDynamicWnd::TopRight	=	{ 100,0,100,0 },
										cdxCDynamicWnd::BotLeft		=	{ 0,100,0,100 },
										cdxCDynamicWnd::BotRight	=	{ 100,100,100,100 };


//////////////////////////////////////////////////////////////////////
// construction
//////////////////////////////////////////////////////////////////////

/*
 * construction
 */

cdxCDynamicWnd::cdxCDynamicWnd(Freedom fd, UINT nFlags)
:	m_pWnd(NULL),
	m_iDisabled(0),
	m_Freedom(fd),
	m_szInitial(M_szNull),
	m_szMin(0,0),
	m_szMax(0,0),
	m_bUseScrollPos(false),
	m_pSizeIcon(NULL),
	m_idSizeIcon(AFX_IDW_SIZE_BOX),
	m_nMyTimerID(0),
	m_nFlags(nFlags)
{
}


//////////////////////////////////////////////////////////////////////
// control work
//////////////////////////////////////////////////////////////////////

/*
 * AddSzControl()
 * --------------
 * Add a control that will react on changes to the parent window's size.
 *		hwnd			-	the child control.
 *		pos			-	describes what to do at all.
 *		bReposNow	-	true to immediately make the control change its position
 *							if necessary, false if not
 *							In the latter case you may like to call Layout() afterwards.
 *
 * returns false if an invalid window has been passed to this funciton.
 */

bool cdxCDynamicWnd::AddSzControl(HWND hwnd, const Position & pos, bool bReposNow)
{
	if(!IsWindow())
	{
		ASSERT(IsWindow());
		return false;			// NO assert if hwnd is invalid
	}

	if(!::IsWindow(hwnd))
	{
		TRACE(_T("*** NOTE[cdxCDynamicWnd::AddSzControl(HWND,const Position &,bool)]: Handle 0x%lx is not a valid window.\n"),(DWORD)hwnd);
		return false;
	}

	m_Map.SetAt(hwnd,pos);

	if(bReposNow)
		UpdateControlPosition(hwnd);

	return true;
}

/*
 * AllControls()
 * -------------
 * Apply positioning to all controls of the window.
 *		bytes			-	positioning data
 *		bOverwrite	-	overwrite any existing positioning data.
 *		bReposNow	-	true to immediately make the control change its position
 *							if necessary, false if not
 *							In the latter case you may like to call Layout() afterwards.
 */

void cdxCDynamicWnd::AllControls(const SBYTES & bytes, bool bOverwrite, bool bReposNow)
{
	if(!IsWindow())
	{
		ASSERT(false);
		return;
	}

	Position	pos;
	UINT		nCnt	=	0;

	for(HWND hwnd = ::GetWindow(m_pWnd->m_hWnd,GW_CHILD); hwnd; hwnd = ::GetNextWindow(hwnd,GW_HWNDNEXT))
	{
		if(bOverwrite || !m_Map.Lookup(hwnd,pos))
			if(AddSzControl(hwnd,bytes,false))
				++nCnt;
	}

	if(nCnt && bReposNow)
		Layout();
}

/*
 * RemSzControl()
 * --------------
 * Removes a control from the internal list.
 * The control will remain at its initial position if bMoveToInitialPos is false
 * Returns false if an error occured.
 */

bool cdxCDynamicWnd::RemSzControl(HWND hwnd, bool bMoveToInitialPos)
{
	if(!::IsWindow(hwnd) || !IsWindow())
		return false;

	if(bMoveToInitialPos)
	{
		Position	pos;

		if(!m_Map.Lookup(hwnd,pos))
			return false;

		VERIFY( ::SetWindowPos(hwnd,HWND_TOP,
									pos.left,pos.top,pos.Width(),pos.Height(),
									SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER) );
	}

	return m_Map.RemoveKey(hwnd) != FALSE;
}

/*
 * UpdateControlPosition()
 * =======================
 * Move control to its desired position.
 * returns false if HWND is not valid.
 */

bool cdxCDynamicWnd::UpdateControlPosition(HWND hwnd)
{
	if(!IsWindow())
	{
		ASSERT(IsWindow());
		return false;			// NO assert if hwnd is invalid
	}

	if(!::IsWindow(hwnd))
	{
		TRACE(_T("*** NOTE[cdxCDynamicWnd::UpdateControlPosition()]: Handle 0x%lx is not a valid window.\n"),(DWORD)hwnd);
		return false;
	}

	cdxCDynamicLayoutInfo	*pli	=	DoCreateLayoutInfo();
	ASSERT(pli != NULL);

	if(!pli || !pli->IsInitial())
	{
		try
		{
			CRect							rectNew;
			WINDOWPLACEMENT			wpl;
			wpl.length	=	sizeof(WINDOWPLACEMENT);
			VERIFY(::GetWindowPlacement(hwnd,&wpl) );

			rectNew	=	wpl.rcNormalPosition;

			if(DoMoveCtrl(hwnd,::GetDlgCtrlID(hwnd),rectNew,*pli) &&
				(rectNew != wpl.rcNormalPosition) )
			{
				VERIFY( ::SetWindowPos(hwnd,HWND_TOP,
												rectNew.left,rectNew.top,rectNew.Width(),rectNew.Height(),
												SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER) );
			}
		}
		catch(...)
		{
			delete pli;
			throw;
		}
	}

	delete pli;

	return true;
}


//////////////////////////////////////////////////////////////////////
// main layout engine
//////////////////////////////////////////////////////////////////////

/*
 * Layout()
 * --------
 * Iterates through all child windows and calls DoMoveCtrl() for them.
 * This function is NOT virtual.
 * To implement your own layout algorithm, please
 * a) overwrite DoCreateLayoutInfo() to return an object of a class
 *    derived from cdxCDynamicLayoutInfo.
 *    You can put any user-data into your object; it will be passed
 *    on to the DoMoveCtrl() function.
 * b) overwrite DoMoveCtrl() and implement the layout logic.
 *    An example can be found in the example project, anytime.
 */

void cdxCDynamicWnd::Layout()
{
	if(!IsWindow())
	{
		ASSERT(IsWindow());
		return;
	}
	
	// resize stuff

	cdxCDynamicLayoutInfo	*pli		=	DoCreateLayoutInfo();

	if(!pli)
	{
		ASSERT(false);		// YOU MUST PROVIDE A LAYOUT INFO BLOCK !
		return;
	}

	try
	{
		HDWP							hdwp		=	::BeginDeferWindowPos(pli->m_nCtrlCnt);
		HWND							hwnd;
		bool							bRepeat;
		CRect							rectNew;
		UINT							id;
		WINDOWPLACEMENT			wpl;
		DWORD							swpFlags	=	SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER|(!(m_nFlags & flSWPCopyBits) ? SWP_NOCOPYBITS : 0);

		if(!( hwnd = ::GetWindow(m_pWnd->m_hWnd,GW_CHILD) ))
		{
			TRACE(_T("*** NOTE[cdxCDynamicWnd::Layout()]: The window at 0x%lx does not have child windows.\n"),(DWORD)m_pWnd->m_hWnd);
			return;
		}

		do
		{
			bRepeat				=	false;

			for(; hwnd; hwnd = ::GetNextWindow(hwnd,GW_HWNDNEXT))
			{
				wpl.length	=	sizeof(WINDOWPLACEMENT);

				if(!::GetWindowPlacement(hwnd,&wpl))
				{
					ASSERT(false);		// GetWindowPlacement() failed
					continue;
				}

				rectNew	=	wpl.rcNormalPosition;
				ASSERT(rectNew.left >= 0);
				id			=	::GetDlgCtrlID(hwnd);

				if(!DoMoveCtrl(hwnd,id,rectNew,*pli) ||
					(rectNew == wpl.rcNormalPosition) )
				{
					// window doesn't need to be moved
					// (position is not been changed)
					continue;
				}

				if(hdwp)
				{
					if(!( hdwp = ::DeferWindowPos(hdwp,hwnd,HWND_TOP,
															rectNew.left,rectNew.top,rectNew.Width(),rectNew.Height(),
															swpFlags) ))
					{
						TRACE(_T("*** ERROR[cdxCDynamicWnd::ReorganizeControls()]: DeferWindowPos() failed ??\n"));
						bRepeat	=	true;
						break;		// error; we'll repeat the loop by using SetWindòwPos()
										// this won't look good, but work :)
					}
				}
				else
				{
					VERIFY( ::SetWindowPos(hwnd,HWND_TOP,
												rectNew.left,rectNew.top,rectNew.Width(),rectNew.Height(),
												swpFlags) );
				}
			}
		}
		while(bRepeat);

		if(hdwp)
		{
			VERIFY( ::EndDeferWindowPos(hdwp) );
		}
	}
	catch(...)
	{
		delete pli;
		throw;
	}

	delete pli;
}

//////////////////////////////////////////////////////////////////////
// message work
//////////////////////////////////////////////////////////////////////

/*
 * DoMoveCtrl()
 * ------------
 * This virtual function is used to calculate a child window's new position
 * based on the some data (from the cdxCDynamicLayoutInfo object).
 * This standard routine is made to implement the algorithm as known from
 * the cdxCDynamicControlsManager.
 * You can implement your own code if you are not satisfied with the
 * following function.
 * If you need global data, overwrite DoCreateLayoutInfo() which will
 * be called by Layout() and which you can use to collect these data
 * once for the entire layout process.
 *
 * PARAMETERS:
 *
 *		hwnd			-	handle of the child control
 *		id				-	its id
 *		rectNewPos	-	write the new position here in.
 *							initially contains the current position
 *		li				-	Some information on the parent window.
 *							You can provide extra information here
 *							by overwriting DoCreateLayoutInfo().
 *
 * RETURN CODES:
 *
 * return false if you don't want to move the control
 * return true if you updated the control's position and stored it into "rectNewPos"
 * If you don't change it, the control will not be moved.
 *
 * #### don't move the control by yourself. Layout() will do for you to ensure
 *      that as little flickering as possible will occur.
 */

bool cdxCDynamicWnd::DoMoveCtrl(HWND hwnd, UINT id, CRect & rectNewPos, const cdxCDynamicLayoutInfo & li)
{
	Position	pos;

	if(!GetControlPosition(hwnd,pos))
		return false;

	pos.Apply(hwnd,rectNewPos,li);
	return true;
}	

/*
 * DoDestroyCtrl()
 * ---------------
 * Called when a child window is about being destroyed.
 * We use it to remove our "Position" data from our database.
 */
 
void cdxCDynamicWnd::DoDestroyCtrl(HWND hwnd)
{
	m_Map.RemoveKey(hwnd);
}

//////////////////////////////////////////////////////////////////////
// initialization & clean-uo
//////////////////////////////////////////////////////////////////////

/*
 * DoInitWindow()
 * --------------
 * This function sets up the window pointer.
 * It is recommended that "rWnd" points to an existing CWnd.
 * However, it doesn't need to exist as long as you
 * 1) provide a non-zero "szInitial" object.
 * 2) don't want a size icon.
 *
 *	PARAMETERS:
 *
 *		rWnd			-	reference to your window ("*this")
 *							the window must exist (::IsWindow(rWnd.m_hWnd) must be true)
 *		fd				-	Freedom (in which direction(s) your window shall be sizable BY THE USER):
 *							Possible values: fdAll, fdHorz, fdVert and fdNone.
 *							This is only applied to user-actions; resizing + layout may work
 *							even if the freedom parameter is fdNone (in that case user cannot resize
 *							your window, but you can).
 *		flags			-	several flags:
 *								flSizeIcon		-	creates a size icon
 *								flAntiFlicker	-	activates anti-flickering stuff
 *		[szInitial	-	initial client size]
 */

void cdxCDynamicWnd::DoInitWindow(CWnd & rWnd)
{
	ASSERT(::IsWindow(rWnd.m_hWnd));	// ensure the window exists ...

	m_pWnd		=	&rWnd;
	DoInitWindow(rWnd,GetCurrentClientSize());
}

void cdxCDynamicWnd::DoInitWindow(CWnd & rWnd, const CSize & szInitial)
{
	ASSERT(::IsWindow(rWnd.m_hWnd) && szInitial.cx && szInitial.cy);	// ensure the window exists ...

	m_pWnd		=	&rWnd;
	m_szInitial	=	szInitial;
	m_szMin		=	szInitial;

	/*
	 * this window will flicker somewhat deadly if you do not have the
	 * WS_CLIPCHILDREN style set for you window.
	 * You may like to use the following line anywhere
	 * to apply it:
	 
		  CWnd::ModifyStyle(0,WS_CLIPCHILDREN);

    */

#ifdef _DEBUG
	if(!(rWnd.GetStyle() & WS_CLIPCHILDREN) && !(m_nFlags & flSWPCopyBits))
	{
		TRACE(_T("***\n")
					_T("*** cdxCDynamicWnd class note: If your window flickers too much, add the WS_CLIPCHILDREN style to it\n")
					_T("***                            or try to set the flSWPCopyBits flags !!!\n")
					_T("***\n"));
	}
#endif

	//
	// now, if a DYNAMIC MAP is been defined,
	// we start working with it
	//

	const __dynEntry	*pEntry,*pLast	=	NULL;
	UINT					nInitCnt	=	GetCtrlCount();

	if(pLast = __getDynMap(pLast))
	{
		HWND		hwnd;
		SBYTES	bytes;

		for(pEntry = pLast; pEntry->type != __end; ++pEntry)
		{
			if((pEntry->id != DYNAMIC_MAP_DEFAULT_ID)
				&& !( hwnd = ::GetDlgItem(m_pWnd->m_hWnd,pEntry->id) ))
			{
				TRACE(_T("*** NOTE[cdxCDynamicWnd::DoInitWindow()]: Dynamic map initialization: There's no control with the id 0x%lx !\n"),pEntry->id);
				continue;
			}

			switch(pEntry->type)
			{
				case	__bytes:

					bytes[X1]	=	pEntry->b1;
					bytes[Y1]	=	pEntry->b2;
					bytes[X2]	=	pEntry->b3;
					bytes[Y2]	=	pEntry->b4;
					break;

				case	__modes:
					
					_translate((Mode)pEntry->b1,bytes[X1],bytes[X2]);
					_translate((Mode)pEntry->b2,bytes[Y1],bytes[Y2]);
					break;

				default:

					ASSERT(false);		// never come here !!!!!
					break;
			}

			if(pEntry->id == DYNAMIC_MAP_DEFAULT_ID)
				AllControls(bytes,false,false);
			else
				AddSzControl(hwnd,bytes,M_szNull,false);
		}
	}

	//
	// handle creation flags
	//

	if(m_nFlags & flSizeIcon)
	{
		m_pSizeIcon	=	new cdxCSizeIconCtrl;
		VERIFY( m_pSizeIcon->Create(m_pWnd) );

		AddSzControl(m_pSizeIcon->m_hWnd,BotRight,M_szNull,false);
		m_pSizeIcon->ShowWindow(SW_SHOW);
	}

	m_bIsAntiFlickering	=	false;
	m_nMyTimerID			=	DEFAULT_TIMER_ID;
	m_dwClassStyle			=	::GetClassLong(*m_pWnd,GCL_STYLE) & (CS_VREDRAW|CS_HREDRAW);

	OnInitialized();

	if(nInitCnt < GetCtrlCount())
		Layout();
}


/*
 * DoDestroyWindow()
 * -----------------
 * Clean up.
 */

void cdxCDynamicWnd::DoOnDestroy()
{
	if(IsWindow())
		OnDestroying();

	m_iDisabled		=	1;
	m_pWnd			=	NULL;
	m_Map.RemoveAll();

	if(m_pSizeIcon)
	{
		m_pSizeIcon->DestroyWindow();
		delete m_pSizeIcon;

		m_pSizeIcon	=	NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// message work
//////////////////////////////////////////////////////////////////////

/*
 * DoOnSize()
 * ----------
 * Calls Layout() if necessary.
 */

void cdxCDynamicWnd::DoOnSize(UINT nType, int cx, int cy)
{
	if(!IsDisabled() &&
		IsWindow() &&
		(nType != SIZE_MINIMIZED))
	{
		Layout();
	}
}

/*
 * DoOnSizing()
 * ------------
 * This is my turbo-new-super-duper anti-flickering function
 * StartAntiFlickering() is called by the following handler.
 */

void cdxCDynamicWnd::DoOnSizing(UINT fwSide, LPRECT pRect)
{
	if(m_nMyTimerID && !IsDisabled() && IsWindow() && (m_nFlags & flAntiFlicker))
		StartAntiFlickering(	(fwSide == WMSZ_BOTTOM) ||
									(fwSide == WMSZ_BOTTOMRIGHT) ||
									(fwSide == WMSZ_RIGHT));
}

/*
 * StartAntiFlickering()
 * ---------------------
 * This routine modifies the CS_VREDRAW and CS_HREDRAW CLASS style
 * flags.
 * If you don't like this, set "m_nMyTimerID" to 0.
 *		bIsBotRight	-	true if the window is sized in right, bottom or bot/right direction.
 */

void cdxCDynamicWnd::StartAntiFlickering(bool bIsBotRight)
{
	if(IsWindow() && m_nMyTimerID)
	{
		DWORD	dw	=	m_dwClassStyle;
		if(bIsBotRight)
			dw	&=	~(CS_VREDRAW|CS_HREDRAW);
		else
			dw	|=	CS_VREDRAW|CS_HREDRAW;

		m_pWnd->KillTimer(m_nMyTimerID);
		m_pWnd->SetTimer(m_nMyTimerID,120,NULL);

		if(!m_bIsAntiFlickering)
		{
			::SetClassLong(*m_pWnd,GCL_STYLE,dw);
			m_bIsAntiFlickering	=	true;
		}
	}
}

/*
 * DoOnTimer()
 * -----------
 * Processes the timer associated to my DoOnSizing() routine.
 * Changes back the class style.
 */

void cdxCDynamicWnd::DoOnTimer(UINT nIDEvent) 
{
	if(IsWindow() && (nIDEvent == m_nMyTimerID))
	{
		m_pWnd->KillTimer(m_nMyTimerID);
		if(m_bIsAntiFlickering)
		{
			::SetClassLong(*m_pWnd,GCL_STYLE,m_dwClassStyle);
			m_bIsAntiFlickering	=	false;
		}
	}
}

//////////////////////////////////////////////////////////////////////

/*
 * DoOnGetMinMaxInfo()
 * -------------------
 * fill in MINMAXINFO as requested
 * Call your CWnd's OnGetMinMaxInfo first !
 * [changed due to a bug reported by Michel Wassink <mww@mitutoyo.nl>]
 */

void cdxCDynamicWnd::DoOnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if(IsWindow() && !IsDisabled()  &&  IsUp())
	{
		CSize	szDelta	=	GetBorderSize();

		lpMMI->ptMinTrackSize.x	=	m_szMin.cx + szDelta.cx;
		lpMMI->ptMinTrackSize.y	=	m_szMin.cy + szDelta.cy;

		if(m_Freedom & fdHoriz)
		{
			if(m_szMax.cx > 0)
				lpMMI->ptMaxTrackSize.x	=	m_szMax.cx + szDelta.cx;
		}
		else
			lpMMI->ptMaxTrackSize.x	=	lpMMI->ptMinTrackSize.x;

		if(m_Freedom & fdVert)
		{
			if(m_szMax.cy > 0)
				lpMMI->ptMaxTrackSize.y	=	m_szMax.cy + szDelta.cy;
		}
		else
			lpMMI->ptMaxTrackSize.y	=	lpMMI->ptMinTrackSize.y;
	}
}

//////////////////////////////////////////////////////////////////////

/*
 * DoOnParentNotify()
 * ------------------
 * When a child window is been destroyed, we remove the appropiate
 * HWND entries.
 */

void cdxCDynamicWnd::DoOnParentNotify(UINT message, LPARAM lParam) 
{
	if(!lParam || (message != WM_DESTROY))
		return;

	DoDestroyCtrl((HWND)lParam);
}



