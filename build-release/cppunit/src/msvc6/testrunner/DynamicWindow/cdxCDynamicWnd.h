// cdxCDynamicWnd.h: interface for the cdxCDynamicWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CDXCDYNAMICWND_H__1FEFDD69_5C1C_11D3_800D_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICWND_H__1FEFDD69_5C1C_11D3_800D_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxwin.h>
#include <afxtempl.h>

class cdxCSizeIconCtrl;
class cdxCDynamicWnd;

#ifndef DECLARE_CDX_HIDDENFUNC
#define	DECLARE_CDX_HIDDENFUNC(name)		name
#endif
#ifndef DECLARE_CDX_HIDDENENUM
#define	DECLARE_CDX_HIDDENENUM(name)		enum name
#endif
#ifndef DECLARE_CDX_HIDDENSTRUCT
#define	DECLARE_CDX_HIDDENSTRUCT(name)	struct name
#endif

#pragma warning(disable: 4100)
#pragma warning(disable: 4706)

/*
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 9
 * ---------------------------
 * A) To make groupboxes work with WS_CLIPCHILDREN windows, set
 *    the WS_EX_TRANSPARENT flag for this child window.
 *    THIS IS NOT A PROBLEM WITH THIS CLASS BUT WITH MFC AT ALL
 *    (you can check it by test-viewing the group box in the
 *     resource editor).
 * B) The property sheet now has the WS_CLIPCHILDREN flag and it
 *    uses flSWPCopyBits.
 * C) The same applies to cdxCDynamicBar.
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 8
 * ---------------------------
 * A) Flags flSWPCopyBits added (which will be cleared by default)
 *		This leads into far less flickering but ensures proper updates
 *		for all child controls.
 *		Some controls do need you to clear this flag.
 *		It ensures that I don't use SWP_NOCOPYBITS.
 *    (Michel Wassink)
 *
 *    IMPORTANT:
 *    People should use the WS_CLIPCHILDREN flag for DIALOGS to
 *    avoid flickering !!!!!
 *
 * B) Added ModifyFlags() and GetFlags().
 *    (To help people modifying my flags)
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 7
 * ---------------------------
 * A) Bug in two overloads taking SBYTE parameters removed.
 *    (Uwe Keim)
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 6
 * ---------------------------
 * A) Added some #pragma warning(disable) to avoid ugly warnings
 *    when compiling using warning level 4.
 *    (Rick Hullinger)
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 5
 * ---------------------------
 * A) AddSzControl(...) overloads for control IDs added
 *    (Uwe Keim)
 * B) Design issue: AddSzControl() with bRepos == true didn't used
 *    DoMoveCtrl() as it would supposed to be.
 *    (Hans Bühler, concerning an issue of Roberto del Noce.
 * C) Layout-Algorithm little changed:
 *    If you now want to provide extra information by deriving
 *    a class from cdxCDynamicLayoutInfo, you no longer overwrite Layout()
 *    but DoCreateLayoutInfo().
 *    (Hans Bühler)
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 4
 * ---------------------------
 * A) BEGIN_DYNAMIC_MAP() now takes TWO parameters:
 *    The class itself and its base-class.
 *    This way even maps defined for bade-classes will work properly.
 *    (Rick Hullinger)
 *    If this feature offends your code, define _CDX_SIMPLE_DYNAMIC_MAPS in your
 *    project's settings to switch back to the old behaviour.
 *    However, it's strongly recommended to modify the BEGIN_DYNAMIC_MAP()
 *    declarations since the final release will surely have this feature.
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 3
 * ---------------------------
 * A) The size icon is now displayed using the right colors.
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 2
 * ---------------------------
 * A) changed cdxCDynamicWnd::BYTE to cdxCDynamicWnd::SBYTE
 *    changed cdxCDynamicWnd::BYTES to cdxCDynamicWnd::SBYTES
 *    to avoid conflicts with Window's BYTE data type.
 *    (Joshua Jensen)
 * B) Dialogs will be centered and sized to 110% by default.
 *    (Hans Bühler)
 * C) Bug when avoiding flAntiFlicker
 * ---------------------------
 * cdxCDynamicWnd beta 1 fix 1
 * ---------------------------
 * A) ::Get/SetWindowPlacement() needs length in structure
 *    (Joshua Jensen)
 */


/*
 * cdxCDynamicLayoutInfo
 * =====================
 * Layout information class.
 * This class is derived from CObject and made dynamic using
 * DECLARE_DYNAMIC.
 * You can derive your own class from it to provide more information
 * to your own DoMoveCtrl() function (if you have one).
 */

class cdxCDynamicLayoutInfo : public CObject
{
	DECLARE_DYNAMIC(cdxCDynamicLayoutInfo);

public:
	CSize		m_szCurrent,			// current client size
				m_szInitial,			// initial client size
				m_szDelta;				// current - initial
	UINT		m_nCtrlCnt;				// number of controls (>=0)
	CPoint	m_pntScrollPos;		// current scrolling position
	bool		m_bUseScrollPos;		// use scroll pos if m_szDelta < 0

public:
	cdxCDynamicLayoutInfo() : m_bUseScrollPos(false) 
        {
        }

	cdxCDynamicLayoutInfo(cdxCDynamicWnd *pWnd) : m_bUseScrollPos(false) 
        { 
          operator=(pWnd); 
        }

	virtual ~cdxCDynamicLayoutInfo() 
        {
        }

	bool operator=(cdxCDynamicWnd *pWnd);

	bool IsInitial() const { return !m_szDelta.cx && !m_szDelta.cy && (!m_bUseScrollPos || (!m_pntScrollPos.x && !m_pntScrollPos.y)); }
};

/*
 * cdxCDynamicWnd
 * ==============
 * The dynamic window manager.
 */

class cdxCDynamicWnd
{
public:
	// add sz control mode types

	enum Mode		// flags for AddSzControl()
	{
		mdNone			=	0,					// does nothing
		mdResize			=	1,					// resize in that dimension
		mdRepos			=	2,					// reposition
		mdRelative		=	3,					// center (size by delta/2 and repos by delta/2)
	};

	// freedom

	enum Freedom
	{
		fdNone			=	0,					// might be used but I don't imagine what you want from this ??
		fdHoriz			=	0x01,				// horizantally sizable only
		fdVert			=	0x02,				// vertically sizable only
		fdAll				=	fdHoriz|fdVert,// sizable in all directions

		fdHorz			=	fdHoriz,			// synonyms
		fdX				=	fdHoriz,
		fdY				=	fdVert
	};

	// some flags

	enum Flags
	{
		flSizeIcon		=	0x01,				// create size icon
		flAntiFlicker	=	0x02,				// some utility func
		flSWPCopyBits	=	0x04,				// make SetWindowPos() don't use SWP_NOCOPYBITS. This may lead
													// into improper results for SOME child controls but speeds up redrawing (less flickering)

		_fl_reserved_	=	0x0000ffff,		// reserved
		_fl_freeuse_	=	0xffff0000		// free 4 u
	};

	// some constants

	enum
	{
		DEFAULT_TIMER_ID	=	0x7164
	};

	// byte percentage

	enum { X1=0, Y1=1, X2=2, Y2=3 };

	typedef signed char	SBYTE;
	typedef SBYTE			SBYTES[4];

	// some internal data; might be of any interest for you

	class Position : public CRect
	{
	public:

	public:
		SBYTES		m_Bytes;
		CSize			m_szMin;

	public:
		Position() : CRect(0,0,0,0) {}
		Position(const CRect & rect, const SBYTES & bytes, const CSize & szMin = M_szNull) : CRect(rect), m_szMin(szMin) { operator=(bytes); }
		~Position() {}

		void operator=(const CRect & rectInitial) { *this = rectInitial; }
		void operator=(const SBYTES & bytes) { for(int i=0; i<4; ++i) m_Bytes[i] = bytes[i]; }
		void operator=(const CSize & szMin) { m_szMin = szMin; }

		void Apply(HWND hwnd, CRect & rectNewPos, const cdxCDynamicLayoutInfo & li) const;
	};

private:
	CWnd						*m_pWnd;				// the parent window
	cdxCSizeIconCtrl		*m_pSizeIcon;		// size icon (if wanted)
	bool						m_bIsAntiFlickering;

protected:
	int						m_iDisabled;		// disabled counter
	DWORD						m_dwClassStyle;	// stored for AntiFlickering feature
	CMap<HWND,HWND,Position,const Position &>
								m_Map;				// controllers

public:
	Freedom					m_Freedom;			// in which direction may we modify the window's size ?
	UINT						m_nFlags;
	CSize						m_szInitial;		// initial client size
	CSize						m_szMin,				// min/max CLIENT size (set to zero to disable)
								m_szMax;
	UINT						m_idSizeIcon;		// id of size icon (default to AFX_IDW_SIZE_BOX)
	UINT						m_nMyTimerID;		// id of the timer used by me
	bool						m_bUseScrollPos;	// use scroll position when moving controls

public:
	cdxCDynamicWnd(Freedom fd, UINT nFlags);
	virtual ~cdxCDynamicWnd() { DoOnDestroy(); }

	//
	// status
	//

	bool IsValid() const { return m_pWnd != NULL; }
	bool IsWindow() const { return IsValid() && ::IsWindow(m_pWnd->m_hWnd); }
	bool IsUp() const { return IsWindow() && !m_pWnd->IsIconic(); }
	bool IsDisabled() const { return m_iDisabled > 0; }
	CWnd *Window() const { return m_pWnd; }

	virtual UINT GetCtrlCount() const { return m_Map.GetCount(); }

	//
	// basics
	//

	bool Enable() { return --m_iDisabled <= 0; }
	void Disable() { ++m_iDisabled; }
	UINT ModifyFlags(UINT nAdd, UINT nRem = 0) { UINT n = m_nFlags; m_nFlags &= ~nRem; m_nFlags |= nAdd; return n; }
	UINT GetFlags() const { return m_nFlags; }

	//
	// client size stuff
	//

	virtual CSize GetCurrentClientSize() const;
	CSize GetBorderSize() const;

	//
	// AddSzControl for HWNDs
	//

	bool AddSzXControl(HWND hwnd, SBYTE x1, SBYTE x2, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(hwnd,x1,0,x2,0,szMin,bReposNow); }
	bool AddSzXControl(HWND hwnd, Mode md, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(hwnd,md,mdNone,szMin,bReposNow); }
	bool AddSzYControl(HWND hwnd, SBYTE y1, SBYTE y2, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(hwnd,0,y1,0,y2,szMin,bReposNow); }
	bool AddSzYControl(HWND hwnd, Mode md, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(hwnd,mdNone,md,szMin,bReposNow); }

	bool AddSzControl(HWND hwnd, Mode mdX, Mode mdY, const CSize & szMin = M_szNull, bool bReposNow = true);
	bool AddSzControl(HWND hwnd, SBYTE x1, SBYTE y1, SBYTE x2, SBYTE y2, const CSize & szMin = M_szNull, bool bReposNow = true);

	bool AddSzControl(HWND hwnd, HWND hLikeThis, bool bReposNow = true);
	bool AddSzControl(HWND hwnd, const SBYTES & bytes, const CSize & szMin = M_szNull, bool bReposNow = true);

	virtual bool AddSzControl(HWND hwnd, const Position & pos, bool bReposNow = true);					// virtual entry point

	//
	// AddSzControl for IDss
	//

	bool AddSzXControl(UINT id, SBYTE x1, SBYTE x2, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzXControl(GetSafeChildHWND(id),x1,x2,szMin,bReposNow); }
	bool AddSzXControl(UINT id, Mode md, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzXControl(GetSafeChildHWND(id),md,szMin,bReposNow); }
	bool AddSzYControl(UINT id, SBYTE y1, SBYTE y2, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzYControl(GetSafeChildHWND(id),y1,y2,szMin,bReposNow); }
	bool AddSzYControl(UINT id, Mode md, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzYControl(GetSafeChildHWND(id),md,szMin,bReposNow); }

	bool AddSzControl(UINT id, Mode mdX, Mode mdY, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(GetSafeChildHWND(id),mdX,mdY,szMin,bReposNow); }
	bool AddSzControl(UINT id, SBYTE x1, SBYTE y1, SBYTE x2, SBYTE y2, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(GetSafeChildHWND(id),x1,y1,x2,y2,szMin,bReposNow); }

	bool AddSzControl(UINT id, HWND hLikeThis, bool bReposNow = true) { return AddSzControl(GetSafeChildHWND(id),hLikeThis,bReposNow); }
	bool AddSzControl(UINT id, const SBYTES & bytes, const CSize & szMin = M_szNull, bool bReposNow = true) { return AddSzControl(GetSafeChildHWND(id),bytes,szMin,bReposNow); }

	bool AddSzControl(UINT id, const Position & pos, bool bReposNow = true) { return AddSzControl(GetSafeChildHWND(id),pos,bReposNow); }

	//
	// all controls
	//

	void AllControls(Mode mdX, Mode mdY, bool bOverwrite = false, bool bReposNow = true);
	void AllControls(SBYTE x1, SBYTE y1, SBYTE x2, SBYTE y2, bool bOverwrite = false, bool bReposNow = true);
	void AllControls(const SBYTES & bytes, bool bOverwrite = false, bool bReposNow = true);
	
	// etc

	bool GetControlPosition(HWND hwnd, Position & pos) { return m_Map.Lookup(hwnd,pos) != FALSE; }
	bool RemSzControl(HWND hwnd, bool bMoveToInitialPos = false);
	bool UpdateControlPosition(HWND hwnd);

	//
	// operational
	//

	virtual void Layout();

	//
	// you have to delegate work to these 
	//

protected:
	void DoInitWindow(CWnd & rWnd, const CSize & szInitial);
	void DoInitWindow(CWnd & rWnd);		// short-cut
	void DoOnDestroy();

	void DoOnParentNotify(UINT message, LPARAM lParam);
	void DoOnTimer(UINT nIDEvent);
	void DoOnSize(UINT nType, int cx, int cy);
	void DoOnSizing(UINT fwSide, LPRECT pRect);
	void DoOnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

	//
	// some advanced virtuals
	//

protected:
	virtual bool DoMoveCtrl(HWND hwnd, UINT id, CRect & rectNewPos, const cdxCDynamicLayoutInfo & li);
	virtual void DoDestroyCtrl(HWND hwnd);

	virtual void OnInitialized() {}
	virtual void OnDestroying() {}

	virtual cdxCDynamicLayoutInfo *DoCreateLayoutInfo() { return new cdxCDynamicLayoutInfo(this); }

	//
	// misc utility functions
	//

public:
	virtual void StartAntiFlickering(bool bIsBotRight);
	HWND GetSafeChildHWND(UINT nID);

	//
	// some operators
	//
public:
	operator CWnd * () const { return m_pWnd; }

	//
	// private members (hidden from classview)
	//
private:
	// DON'T USE
	DECLARE_CDX_HIDDENFUNC( cdxCDynamicWnd(const cdxCDynamicWnd & w) ) { ASSERT(false); }
	void DECLARE_CDX_HIDDENFUNC( operator=(const cdxCDynamicWnd & w) ) { ASSERT(false); }
	// helpers
	void DECLARE_CDX_HIDDENFUNC( _translate(Mode md, SBYTE & b1, SBYTE & b2) );

	//
	// DYNAMIC_MAPping
	//
public:
	DECLARE_CDX_HIDDENENUM( __dynEntryType )
	{
		__end,
		__bytes,
		__modes
	};
	DECLARE_CDX_HIDDENSTRUCT( __dynEntry )
	{
		__dynEntryType	type;
		UINT				id;
		SBYTE				b1,b2,b3,b4;
	};
protected:
	virtual const __dynEntry * DECLARE_CDX_HIDDENFUNC( __getDynMap(const __dynEntry *pLast) ) const { return NULL; }

public:
	static const CSize		M_szNull;			// for the "Config" class
	static const SBYTES		TopLeft,
									TopRight,
									BotLeft,
									BotRight;
};

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicLayoutInfo DYNAMIC MAP macros
/////////////////////////////////////////////////////////////////////////////

/*
 * Macros that can be used to implement an automatic setup
 * for any dynamic window.
 * If you use these, you don't need to use AddSzControl():
 */


// declare map
#ifndef DECLARE_DYNAMIC_MAP
#define DECLARE_DYNAMIC_MAP()		\
protected:											\
	virtual const __dynEntry *__getDynMap(const __dynEntry *pLast) const;	\
private:												\
	static const __dynEntry __M_dynEntry[];
#endif

// begin the map and set freedom/size icon flags
#ifdef _CDX_SIMPLE_DYNAMIC_MAPS
#ifndef BEGIN_DYNAMIC_MAP
#define	BEGIN_DYNAMIC_MAP(CLASS)	\
	const cdxCDynamicWnd::__dynEntry *CLASS::__getDynMap(const __dynEntry *pLast) const { return __M_dynEntry; }	\
	const cdxCDynamicWnd::__dynEntry CLASS::__M_dynEntry[]	=	{
#endif
#else

// begin a dynamic map that even takes care of maps defined for base-class versions
#ifndef BEGIN_DYNAMIC_MAP
#define	BEGIN_DYNAMIC_MAP(CLASS,BASECLASS)	\
	const cdxCDynamicWnd::__dynEntry *CLASS::__getDynMap(const __dynEntry *pLast)	const\
	{	\
		if(pLast == __M_dynEntry)	\
			return NULL;				\
		return (pLast = BASECLASS::__getDynMap(pLast)) ? pLast : __M_dynEntry;	\
	}																									\
	const cdxCDynamicWnd::__dynEntry CLASS::__M_dynEntry[]	=	{
#endif
#endif

// end up map
#ifndef END_DYNAMIC_MAP
#define	END_DYNAMIC_MAP()									{	cdxCDynamicWnd::__end	}	};
#endif

// declare operations
#ifndef DYNAMIC_MAP_ENTRY_EX
#define	DYNAMIC_MAP_ENTRY_EX(ID,X1,Y1,X2,Y2)		{	cdxCDynamicWnd::__bytes,	ID,	X1,Y1,X2,Y2	},
#define	DYNAMIC_MAP_XENTRY_EX(ID,X1,X2)				DYNAMIC_MAP_ENTRY_EX(ID,X1,0,X2,0)
#define	DYNAMIC_MAP_YENTRY_EX(ID,Y1,Y2)				DYNAMIC_MAP_ENTRY_EX(ID,0,Y1,0,Y2)
#define	DYNAMIC_MAP_ENTRY(ID,MODEX,MODEY)			{	cdxCDynamicWnd::__modes,	ID,	cdxCDynamicWnd::##MODEX,cdxCDynamicWnd::##MODEY	},
#define	DYNAMIC_MAP_XENTRY(ID,MODEX)					DYNAMIC_MAP_XENTRY(ID,MODEX,mdNone)
#define	DYNAMIC_MAP_YENTRY(ID,MODEY)					DYNAMIC_MAP_YENTRY(ID,mdNone,MODEY)
#endif

// use this ID for the default position at the head of your map
#ifndef DYNAMIC_MAP_DEFAULT_ID
#define	DYNAMIC_MAP_DEFAULT_ID							0
#endif

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicLayoutInfo inlines
/////////////////////////////////////////////////////////////////////////////

/*
 * auto-fill in struct
 */

inline bool cdxCDynamicLayoutInfo::operator=(cdxCDynamicWnd *pWnd)
{
	if(!pWnd || !pWnd->IsUp())
		return false;

	m_szCurrent			=	pWnd->GetCurrentClientSize();
	m_szInitial			=	pWnd->m_szInitial;
	m_szDelta			=	m_szCurrent - m_szInitial;
	m_nCtrlCnt			=	pWnd->GetCtrlCount();

	if(m_bUseScrollPos == pWnd->m_bUseScrollPos)
	{
		m_pntScrollPos.x	=	pWnd->Window()->GetScrollPos(SB_HORZ);
		m_pntScrollPos.y	=	pWnd->Window()->GetScrollPos(SB_VERT);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicWnd inlines
/////////////////////////////////////////////////////////////////////////////

/*
 * Add a control
 */

inline bool cdxCDynamicWnd::AddSzControl(HWND hwnd, const SBYTES & bytes, const CSize & szMin, bool bReposNow)
{
	if(!::IsWindow(hwnd))
	{
		// Note that this might happen if you call 
		TRACE(_T("*** NOTE[cdxCDynamicWnd::AddSzControl(HWND,const SBYTES &,const CSize &,bool)]: Handle 0x%lx is not a valid window.\n"),(DWORD)hwnd);
		return false;
	}

	WINDOWPLACEMENT	wpl;
	wpl.length	=	sizeof(WINDOWPLACEMENT);
	VERIFY( ::GetWindowPlacement(hwnd,&wpl) );

	return AddSzControl(hwnd,Position(wpl.rcNormalPosition,bytes,szMin),bReposNow);
}

/*
 * Add control that behaves like another
 */

inline bool cdxCDynamicWnd::AddSzControl(HWND hwnd, HWND hLikeThis, bool bReposNow)
{
	if(!::IsWindow(hwnd))
	{
		TRACE(_T("*** NOTE[cdxCDynamicWnd::AddSzControl(HWND,HWND,bool)]: Handle 0x%lx is not a valid window.\n"),(DWORD)hwnd);
		return false;
	}

	Position	pos;

	if(!m_Map.Lookup(hLikeThis,pos))
	{
		TRACE(_T("*** NOTE[cdxCDynamicWnd::AddSzControl(HWND,HWND,bool)]: For the 'hLikeThis' handle 0x%lx there hasn't been made an entry for yet.\n"),(DWORD)hLikeThis);
		return false;
	}

	return AddSzControl(hwnd,pos);
}

/*
 * old
 */

inline bool cdxCDynamicWnd::AddSzControl(HWND hwnd, Mode mdX, Mode mdY, const CSize & szMin, bool bReposNow)
{
	SBYTES	b;
	_translate(mdX,b[X1],b[X2]);
	_translate(mdY,b[Y1],b[Y2]);
	return AddSzControl(hwnd,b,szMin,bReposNow);
}

/*
 * old
 */

inline bool cdxCDynamicWnd::AddSzControl(HWND hwnd, SBYTE x1, SBYTE y1, SBYTE x2, SBYTE y2, const CSize & szMin, bool bReposNow)
{
	SBYTES b;
	b[X1]	=	x1,
	b[X2]	=	x2,
	b[Y1]	=	y1,
	b[Y2]	=	y2;
	return AddSzControl(hwnd,b,szMin,bReposNow);
}

/////////////////////////////////////////////////////////////////////////////

/*
 * short-cut
 */

inline void cdxCDynamicWnd::AllControls(Mode mdX, Mode mdY, bool bOverwrite, bool bReposNow)
{
	SBYTES	b;
	_translate(mdX,b[X1],b[X2]);
	_translate(mdY,b[Y1],b[Y2]);
	AllControls(b,bOverwrite,bReposNow);
}


/*
 * short-cut
 */

inline void cdxCDynamicWnd::AllControls(SBYTE x1, SBYTE y1, SBYTE x2, SBYTE y2, bool bOverwrite, bool bReposNow)
{
	SBYTES b;
	b[X1]	=	x1,
	b[X2]	=	x2,
	b[Y1]	=	y1,
	b[Y2]	=	y2;
	AllControls(b,bOverwrite,bReposNow);
}


/////////////////////////////////////////////////////////////////////////////

/*
 * get size of current client area
 */

inline CSize cdxCDynamicWnd::GetCurrentClientSize() const
{
	if(!IsWindow())
	{
		ASSERT(false);
		return M_szNull;
	}

	CRect	rect;
	m_pWnd->GetClientRect(rect);

	return rect.Size();
}

/*
 * get difference between window and client size
 */

inline CSize cdxCDynamicWnd::GetBorderSize() const
{
	if(!IsUp())
	{
		ASSERT(false);
		return M_szNull;
	}

	CRect	r1,r2;
	m_pWnd->GetWindowRect(r1);
	m_pWnd->GetClientRect(r2);

	return r1.Size() - r2.Size();
}

/////////////////////////////////////////////////////////////////////////////

/*
 * translates a "mode" into percentage
 */

inline void cdxCDynamicWnd::_translate(Mode md, SBYTE & b1, SBYTE & b2)
{
	switch(md)
	{
		default				:	ASSERT(false);
		case	mdNone		:	b1	=	0;		b2	=	0;		break;
		case	mdResize		:	b1	=	0;		b2	=	100;	break;
		case	mdRepos		:	b1	=	100;	b2	=	100;	break;
		case	mdRelative	:	b1	=	50;	b2	=	50;	break;
	}
}

/*
 * gets HWND of a child given by ID
 */

inline HWND cdxCDynamicWnd::GetSafeChildHWND(UINT nID)
{
	if(!IsWindow())
	{
		ASSERT(false);
		return 0;
	}

	HWND	h	=	::GetDlgItem(m_pWnd->m_hWnd,nID);
	ASSERT(h!=0);
	return h;
}
	
#pragma warning(default: 4100)
#pragma warning(default: 4706)

#endif // !defined(AFX_CDXCDYNAMICWND_H__1FEFDD69_5C1C_11D3_800D_000000000000__INCLUDED_)
