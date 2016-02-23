// cdxCDynamicControlsManager.h: interface for the cdxCDynamicControlsManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CDXCDYNAMICCONTROLSMANAGER_H__6517AE13_5D12_11D2_BE4C_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICCONTROLSMANAGER_H__6517AE13_5D12_11D2_BE4C_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include	"cdxCSizeIconCtrl.h"
#include	<afxpriv.h>
typedef cdxCSizeIconCtrl	cdxCSizeCtrl;

//
// cdxCDynamicControlsManager.h : header file
// -----------------------------------------------------------------------
// Author:  Hans Bühler (hans.buehler@student.hu-berlin.de)
//          codex design (http://www-pool.mathematik.hu-berlin.de/~codex
// Version: 1.5
// Release: 5 (Mar 1999 to www.codeguru.com)
// -----------------------------------------------------------------------
// Changes for V1.1:
// - cdxCSizeCtrl is now only a typedef on cdxCSizeIconCtrl which is been
//   but in two extra files (header and impl.) to make it available to
//   the programmer even if you don't use cdxCDynamicControls.
//   The include/impl file for cdxCSizeIconCtrl must be available.
// - GetSizeIconBitmap() is been deleted.
// - ICON_CONTROL_ID has been changed to AFX_IDW_SIZE_BOX
// Changes for V1.2:
// - cdxCDynamicControlsManager::DoOnGetMinMaxInfo() has been modified
//   (thanks to a bug report by Michel Wassink <mww@mitutoyo.nl>):
//   Now, if you don't call SetMaxSize(), the maximum position of a window
//   will not be changed.
//	  BUG: Under W95 and W98, resizing didn't work properly REMOVED.
// Changes for V1.3:
// - FindSzControl() and RemSzControl() have been added.
// Changes for V1.4:
// - RestoreWindowPosition() is been speed up.
// - FixWindowSize() has been debugged
// - RemSzControl() is been made more comfortable.
//   You can now remove controls properly from the dynamic controls manager.
//   Moreover, the embedded ControlPosition class has now some more virtual
//   functions for your own use.
//   For example, you can modify the way a control reacts later now.
// - Enable() / Disable() have been added.
//   Using them you can temporarily disable the automatic repositioning.
// Changes to V1.5
// - Flickering of controls during resize fixed thanks to great hint
//   by Rodger Bernstein.
// -----------------------------------------------------------------------
// Comments welcome.
//

/*
 * cdxCDynamicControlsManager
 * ==========================
 * Makes any CWnd derived class capable of dynamic control resizing
 * and repositioning.
 * Moreover, it can set a window's max/min tracking size (the size
 * the user can change) and add a nice sizing icon to the windows
 * lower right corner (if the window does not have one - as dialogs).
 *
 * To make any CWnd derived capable of automatically displaying
 * its controls, you embed a member of this class in your window
 * (or you derive your class from both this class and your window
 * base class - that depends on how you want to use the member
 * functions of this class).
 *
 * Then, the following functions must be called
 *
 *		DoInitWindow()			-	Must be called after the window became
 *										valid (i.e. CWnd::m_hWnd is non-zero)
 *										and has its initial (minimum) size.
 *		DoOnSize()				-	by the OnSize() message handler
 *		DoOnGetMinMaxInfo()	-	by the OnGetMinMaxInfo() message handler
 *		DoDestroyWindow()		-	by DestroyWindow().
 *
 * See cdxCSizingDialog.h for an example.
 * 
 * NOTE:
 * Unfortunately, we cannot derive this class from CObject, because
 * those macros DECLARE_xxx are too lame to handle multipile derived
 * classes if both classes have been derived from the same base-class
 * CObject.
 */

class cdxCDynamicControlsManager
{
	//
	// various constants
	//
public:
	enum Mode		// flags for AddSzControl()
	{
		mdNone		=	0,					// does nothing
		mdResize		=	1,					// resize in that dimension
		mdRepos		=	2,					// reposition
		mdRelative	=	3,					// center (size by delta/2 and repos by delta/2)
		md__Last		=	mdRelative
	};

	enum Freedom
	{
		fdNone		=	0,					// might be used but I don't imagine what you want from this ??
		fdHoriz		=	0x01,				// horizantally sizable only
		fdVert		=	0x02,				// vertically sizable only
		fdAll			=	fdHoriz|fdVert	// sizable in all directions
	};

	enum RestoreFlags
	{
		rflg_none			=	0,			// only load window position
		rflg_state			=	0x01,		// make window iconic/zoomed if been before
		rflg_visibility	=	0x02,		// hide/show window as been before
		rflg_all				=	rflg_state|rflg_visibility
	};

	enum
	{
		ICON_CONTROL_ID	=	AFX_IDW_SIZE_BOX
	};

	//
	// a positioning parameter
	//
public:
	class PositionSetup
	{
	public:
		BYTE	m_dX1pcnt,m_dX2pcnt,			// how positioning should work (see docs)
				m_dY1pcnt,m_dY2pcnt;
	public:
		PositionSetup(BYTE dX1pcnt = 0, BYTE dX2pcnt = 0, BYTE dY1pcnt = 0, BYTE dY2pcnt = 0) : m_dX1pcnt(dX1pcnt), m_dX2pcnt(dX2pcnt), m_dY1pcnt(dY1pcnt), m_dY2pcnt(dY2pcnt) {}
		~PositionSetup() {}

		// validity check
		bool IsValid() const { return (m_dX1pcnt <= m_dX2pcnt) && (m_dY1pcnt <= m_dY2pcnt); }

		// transform
		CRect Transform(const CRect & rectOriginal, const CSize & szDelta) const;

		// quick-use
		CRect operator()(const CRect & rectOriginal, const CSize & szDelta) const { return Transform(rectOriginal,szDelta); }
	};

	//
	// an astract handle to a sizeable control that you can
	// use to add further controls to
	// see discussion of AddSzControl()
	//
public:
	class ControlPosition
	{
	protected:
		ControlPosition() {}
	public:
		virtual ~ControlPosition() {}

	public:
		virtual bool IsMember(CWnd & ctrl) const = NULL;
		virtual bool IsUsed() const = NULL;

		virtual void Add(CWnd & ctrl) = NULL;
		virtual bool Rem(CWnd & ctrl) = NULL;

		virtual bool Modify(const CRect & rectOriginal, const PositionSetup & rSetup) = NULL;
		virtual CRect GetCurrentPosition() const = NULL;
		virtual CRect GetOriginalPosition() const = NULL;
		virtual PositionSetup GetPositionSetup() const = NULL; 
	};

	//
	// internal storage class for controls and their
	// original positions and their behaviour settings
	//
private:
	class ControlData : public ControlPosition
	{
		//
		// all controls with the same positioning arguments
		// (used by Add())
		// Note that the window is not need to be already created
		//
	private:
		struct ControlEntry
		{
		private:
			ControlData		& m_rMaster;			// container
			ControlEntry	*m_pNext,*m_pPrev;	// next, prev
			CWnd				& m_rCtrl;				// the control

		public:
			ControlEntry(CWnd & ctrl, ControlData & rMaster);
			virtual ~ControlEntry();

			void Position(AFX_SIZEPARENTPARAMS *lpSz, int x, int y, int wid, int hi, bool bAll);

			void Add(CWnd & ctrl, int x, int y, int wid, int hi);

			ControlEntry *GetNext() { return m_pNext; }
			CWnd & GetCWnd() { return m_rCtrl; }
			const ControlEntry *GetNext() const { return m_pNext; }
			const CWnd & GetCWnd() const { return m_rCtrl; }

			bool operator==(const CWnd & ctrl) const { return &m_rCtrl == &ctrl; }
		};

		friend struct ControlEntry;

	private:
		cdxCDynamicControlsManager	& m_rMaster;			// the master class
		ControlData						*m_pNext,*m_pPrev;	// a linked list (root in m_rMaster.m_pFirst)

		ControlEntry					*m_pCtrl;				// control link list
		PositionSetup					m_posSetup;
		CRect								m_rectOriginal;		// original position of control(s)

	public:
		ControlData(cdxCDynamicControlsManager & rMaster, CWnd & ctrl, const PositionSetup & rPosSetup);
		virtual ~ControlData();

		virtual bool IsUsed() const { return m_pCtrl != NULL; }

		//
		// access to CWnds
		//

		virtual bool IsMember(CWnd & ctrl) const;
		virtual void Add(CWnd & ctrl) { new ControlEntry(ctrl,*this); }
		virtual bool Rem(CWnd & ctrl);

		//
		// positioning
		//

		virtual bool Modify(const CRect & rectOriginal, const PositionSetup & rSetup);
		virtual CRect GetCurrentPosition() const;
		virtual CRect GetOriginalPosition() const { return CRect(m_rectOriginal); }
		virtual PositionSetup GetPositionSetup() const { return PositionSetup(m_posSetup); }

		//
		// helpers
		//

		ControlData *GetNext() { return m_pNext; }
		const ControlData *GetNext() const { return m_pNext; }

		//
		// events
		//

		void OnSize(const CSize & szDelta, AFX_SIZEPARENTPARAMS *lpSz, const CPoint *pOffset = NULL);
	};

	//
	// my members
	//

	friend class ControlData;
	friend struct ControlData::ControlEntry;

private:
	ControlData		*m_pFirst;
	CWnd				*m_pWnd;						// Use Init() !!!!!!!!!
	CSize				m_szClientRelative,		// original's window size (client !!) - used in OnSize() to calculate delta size
						m_szMin,						// minimum size (whole window)
						m_szMax;						// maximum (whole window)
	Freedom			m_Freedom;					// what is allowed
	cdxCSizeCtrl	*m_pWndSizeIcon;			// the icon control
	int				m_iDisabledCnt;			// counts Disable() and Enable()
	UINT				m_iTotalCnt;				// total counter of all ControlData::ControlEntry objects

public:
	UINT				m_idSizeIcon;				// ID of the icon control (you can set this to change the default, ICON_CONTROL_ID)
	bool				m_bApplyScrollPosition;	// fix scroll position for controls (set this in your constructor)

public:
	cdxCDynamicControlsManager() : m_pFirst(NULL), m_pWnd(NULL), m_Freedom(fdAll), m_pWndSizeIcon(NULL), m_idSizeIcon(ICON_CONTROL_ID), m_iDisabledCnt(0), m_iTotalCnt(0), m_bApplyScrollPosition(false) {}
	virtual ~cdxCDynamicControlsManager() { DoDestroyWindow(); }

	//
	// check status
	//

	bool IsReady() const { return (m_pWnd != NULL) && ::IsWindow(m_pWnd->m_hWnd); }
	UINT GetTotalChildCnt() const { return m_iTotalCnt; }

	//
	// get some basics
	//

	const CSize & GetMinSize() const { return m_szMin; }
	const CSize & GetMaxSize() const { return m_szMax; }
	Freedom GetFreedom() const { return m_Freedom; }

	//
	// wanna change some basics ?
	//
	bool SetMinMaxSize(const CSize & szMin, const CSize & szMax, bool bResizeIfNecessary = true);
	bool FixWindowSize();
	void SetFreedom(Freedom fd) { m_Freedom = fd; }
	void HideSizeIcon();
	void ShowSizeIcon();

	//
	// add controls to handle
	//
	ControlPosition *AddSzControl(CWnd & ctrl, Mode modeX, Mode modeY);
	ControlPosition *AddSzXControl(CWnd & ctrl, Mode modeX) { return AddSzControl(ctrl,modeX,mdNone); }
	ControlPosition *AddSzYControl(CWnd & ctrl, Mode modeY) { return AddSzControl(ctrl,mdNone,modeY); }

	ControlPosition *AddSzControlEx(CWnd & ctrl, BYTE dX1pcnt, BYTE dX2pcnt, BYTE dY1pcnt, BYTE dY2pcnt) { return AddSzControlEx(ctrl,PositionSetup(dX1pcnt,dX2pcnt,dY1pcnt,dY2pcnt)); }
	ControlPosition *AddSzXControlEx(CWnd & ctrl, BYTE dX1pcnt, BYTE dX2pcnt) { return AddSzControlEx(ctrl,dX1pcnt,dX2pcnt,0,0); }
	ControlPosition *AddSzYControlEx(CWnd & ctrl, BYTE dY1pcnt, BYTE dY2pcnt) { return AddSzControlEx(ctrl,0,0,dY1pcnt,dY2pcnt); }

	virtual ControlPosition *AddSzControlEx(CWnd & ctrl, const PositionSetup & rSetup);

	//
	// advanced (new to V1.3)
	//
	ControlPosition *FindSzControl(CWnd & ctrl);
	const ControlPosition *FindSzControl(CWnd & ctrl) const;
	bool RemSzControl(CWnd & ctrl, bool bAutoDeleteUnusedControlPos = true);

	//
	// advanced (new to V1,4)
	//
	virtual bool Enable(bool bForce = false) { if(!bForce) { --m_iDisabledCnt; ASSERT(m_iDisabledCnt >= 0); } else m_iDisabledCnt = 0; return m_iDisabledCnt == 0; }
	virtual void Disable() { ++m_iDisabledCnt; }
	virtual bool IsDisabled() const { return m_iDisabledCnt > 0; }

	//
	// these must be called by the appropiate message handlers of the window
	// class you're deriving from
	//
public:
	void DoInitWindow(CWnd & rWnd, Freedom fd = fdAll, bool bSizeIcon = false, const CSize * pBaseClientSize = NULL);
	void DoOnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	void DoOnSize(UINT nType, int cx, int cy);
	void DoDestroyWindow();

	//
	// some helpers
	//
	void ReorganizeControls(bool bRedraw = true);
	void ReorganizeControlsAdvanced(const CRect & rectWin, CRect rectClient, bool bRedraw = true);
	bool StretchWindow(const CSize & szDelta) { ASSERT(IsReady()); return StretchWindow(*m_pWnd,szDelta); }
	bool StretchWindow(int iAddPcnt) { ASSERT(IsReady()); return StretchWindow(*m_pWnd,iAddPcnt); }
	CSize GetWindowSize() { ASSERT(IsReady()); return GetWindowSize(*m_pWnd); }
	bool RestoreWindowPosition(LPCTSTR lpszProfile, UINT restoreFlags = rflg_none) { ASSERT(IsReady()); return RestoreWindowPosition(*m_pWnd,lpszProfile,restoreFlags); }
	bool StoreWindowPosition(LPCTSTR lpszProfile) { ASSERT(IsReady()); return StoreWindowPosition(*m_pWnd,lpszProfile); }

	//
	// helpers; static
	//
public:
	static bool StretchWindow(CWnd & rWnd, const CSize & szDelta);
	static bool StretchWindow(CWnd & rWnd, int iAddPcnt);
	static CSize GetWindowSize(CWnd & rWnd);
	static bool RestoreWindowPosition(CWnd & rWnd, LPCTSTR lpszProfile, UINT restoreFlags = rflg_none);
	static bool StoreWindowPosition(CWnd & rWnd, LPCTSTR lpszProfile);

	//
	// some virtuals
	//
protected:
	virtual void OnDeleteControlPosition(ControlPosition & rWillBeDeleted) {}
	virtual CRect GetRealClientRect() const;

	//
	// misc
	//
public:
	/* removed */ //static CBitmap & GetSizeIconBitmap(CSize * pSzBmp = NULL);
	static CImageList & GetSizeIconImageList(CSize * pSzBmp = NULL) { if(pSzBmp) *pSzBmp = cdxCSizeIconCtrl::M_ilImage.Size(); return cdxCSizeIconCtrl::M_ilImage; }
};

/*
 * cdxCSizeCtrl
 * ============
 * Is now a typedef to cdxCSizeIconCtrl - see above.
 */

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::PositionSetup inlines
/////////////////////////////////////////////////////////////////////////////

/*
 * this function transforms a control's original position (rectOriginal) into
 * its new rectangle by taking the the difference between the original window's
 * size (szDelta).
 */

inline CRect cdxCDynamicControlsManager::PositionSetup::Transform(const CRect & rectOriginal, const CSize & szDelta) const
{
	CRect	rectNew;

	rectNew.left	=	rectOriginal.left   + (szDelta.cx * (int)m_dX1pcnt) / 100;
	rectNew.right	=	rectOriginal.right  + (szDelta.cx * (int)m_dX2pcnt) / 100;
	rectNew.top		=	rectOriginal.top    + (szDelta.cy * (int)m_dY1pcnt) / 100;
	rectNew.bottom	=	rectOriginal.bottom + (szDelta.cy * (int)m_dY2pcnt) / 100;

	return rectNew;
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::ControlData::ControlEntry inlines
/////////////////////////////////////////////////////////////////////////////


/*
 * add a control that has the same coordinates as the
 * control embedded in the ControlData object.
 * The coordinates are needed to immediately place the
 * control to the original control's position.
 */

inline void cdxCDynamicControlsManager::ControlData::ControlEntry::Add(CWnd & ctrl, int x, int y, int wid, int hi)
{
	VERIFY( m_pNext = new ControlEntry(ctrl,m_rMaster) );
	m_pNext->Position(NULL,x,y,wid,hi,false);
}

/*
 * apply new position to all "ControlEntry" controls
 * we don't change the z-order here !
 */

inline void cdxCDynamicControlsManager::ControlData::ControlEntry::Position(AFX_SIZEPARENTPARAMS *lpSz, int x, int y, int wid, int hi, bool bAll)
{
	if(::IsWindow(m_rCtrl.m_hWnd))		// those window don't need to exist :)
	{
		if (lpSz != NULL)
			AfxRepositionWindow(lpSz, m_rCtrl.m_hWnd, CRect(CPoint(x,y),CSize(wid,hi)));
		else
		{
			VERIFY( m_rCtrl.SetWindowPos(&CWnd::wndBottom,x,y,wid,hi,
							SWP_NOCOPYBITS|SWP_NOOWNERZORDER|
							SWP_NOACTIVATE|SWP_NOZORDER) );
		}
	}
	if(m_pNext && bAll)
		m_pNext->Position(lpSz, x,y,wid,hi,true);
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager::ControlData inlines
/////////////////////////////////////////////////////////////////////////////
/*
 * called by cdxCDynamicControlsManager::ReorganizeControls() if the size of the window has been changed.
 * repositions all controls applied to this ControlData
 */

inline void cdxCDynamicControlsManager::ControlData::OnSize(const CSize & szDelta, AFX_SIZEPARENTPARAMS *lpSz, const CPoint *pOffset)
{
	if(m_pCtrl)
	{
		CRect	rectNew	=	m_posSetup(m_rectOriginal,szDelta);
		if(pOffset)
			rectNew	+=	*pOffset;
		m_pCtrl->Position(lpSz, rectNew.left,rectNew.top,rectNew.Width(),rectNew.Height(),true);
	}
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicControlsManager inlines
/////////////////////////////////////////////////////////////////////////////

/*
 * add a control - we leave that work
 * to the embedded ControlData class
 */

inline cdxCDynamicControlsManager::ControlPosition *cdxCDynamicControlsManager::AddSzControlEx(CWnd & ctrl, const PositionSetup & rSetup)
{
	ASSERT(IsReady());			// don't called DoInitWindow() before ?
	ASSERT(rSetup.IsValid());

	ControlData	*si	=	new ControlData(*this, ctrl, rSetup);
	ASSERT(si != NULL);			// if you don't throw exceptions :)
	return si;
}

/*
 * find a control's ControlPosition
 */

inline const cdxCDynamicControlsManager::ControlPosition *cdxCDynamicControlsManager::FindSzControl(CWnd & ctrl) const
{
	ASSERT(::IsWindow(ctrl.m_hWnd));	// will work for exiting windows only !

	for(const ControlData *si = m_pFirst; si; si = si->GetNext())
		if(si->IsMember(ctrl))
			return si;

	return NULL;
}

inline cdxCDynamicControlsManager::ControlPosition *cdxCDynamicControlsManager::FindSzControl(CWnd & ctrl)
{
	ASSERT(::IsWindow(ctrl.m_hWnd));	// will work for exiting windows only !

	for(ControlData *si = m_pFirst; si; si = si->GetNext())
		if(si->IsMember(ctrl))
			return si;

	return NULL;
}

/*
 * delete an entry for a control
 * ctrl									-	the control
 * bAutoDeleteUnusedControlPos	-	if true, and unused ControlPosition (no more
 *												CWnds are bound to it) will be deleted.
 *												Note that you can use OnDeleteControlPosition()
 *												to find out which one will be deleted if any.
 *
 * returns true of the control has been found and deleted
 */

inline bool cdxCDynamicControlsManager::RemSzControl(CWnd & ctrl, bool bAutoDeleteUnusedControlPos)
{
	for(ControlData *si = m_pFirst; si; si = si->GetNext())
		if(si->Rem(ctrl))
		{
			if(!si->IsUsed() && bAutoDeleteUnusedControlPos)
			{
				OnDeleteControlPosition(*si);
				delete si;
			}
			return true;
		}

	return false;
}

/*
 * adding controls by my nice constants
 */

inline cdxCDynamicControlsManager::ControlPosition *cdxCDynamicControlsManager::AddSzControl(CWnd & ctrl, Mode modeX, Mode modeY)
{
	BYTE	dX1pcnt	=	0,
			dX2pcnt	=	0,
			dY1pcnt	=	0,
			dY2pcnt	=	0;

	switch(modeX)
	{
		default	:			ASSERT(false);			// unknown value for modeX
		case	mdNone	:	break;
		case	mdRepos	:	dX1pcnt	=	dX2pcnt	=	100;
								break;
		case	mdResize	:	dX2pcnt	=	100;
								break;
		case	mdRelative:	dX1pcnt	=	dX2pcnt	=	100 / 2;
								break;
	}

	switch(modeY)
	{
		default	:			ASSERT(false);			// unknown value for modeY
		case	mdNone	:	break;
		case	mdRepos	:	dY1pcnt	=	dY2pcnt	=	100;
								break;
		case	mdResize	:	dY2pcnt	=	100;
								break;
		case	mdRelative:	dY1pcnt	=	dY2pcnt	=	100 / 2;
								break;
	}

	return AddSzControlEx(ctrl,dX1pcnt,dX2pcnt,dY1pcnt,dY2pcnt);
}

/////////////////////////////////////////////////////////////////////////////

/*
 * Reposition
 */

inline void cdxCDynamicControlsManager::ReorganizeControls(bool bRedraw)
{
	ASSERT(IsReady());

	CRect	clrect,winrect;
	m_pWnd->GetClientRect(clrect);
	m_pWnd->GetWindowRect(&winrect);

	if(m_bApplyScrollPosition)
	{
		if(m_pWnd->IsKindOf(RUNTIME_CLASS(CScrollView)))
		{
			clrect	+=	((CScrollView *)m_pWnd)->GetDeviceScrollPosition();
		}
		else
			clrect	+=	CPoint(m_pWnd->GetScrollPos(SB_HORZ),m_pWnd->GetScrollPos(SB_VERT));
	}
	ReorganizeControlsAdvanced(winrect,clrect,bRedraw);
}

/*
 *  get client rect
 */

inline CRect cdxCDynamicControlsManager::GetRealClientRect() const
{
	ASSERT(IsReady());
	CRect	r;
	
	return r;
}

#endif // !defined(AFX_CDXCDYNAMICCONTROLSMANAGER_H__6517AE13_5D12_11D2_BE4C_000000000000__INCLUDED_)
