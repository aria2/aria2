#if !defined(AFX_CDXCDYNAMICBAR_H__910C28F6_6854_11D3_803A_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICBAR_H__910C28F6_6854_11D3_803A_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// cdxCDynamicBar.h : header file
//

#include "SizeCBar.h"
#include	"cdxCDynamicDialog.h"

/*
 * cdxCDynamicDlgBarT
 * ==================
 * A resizable dialog bar.
 * The entire bar stuff is handled using
 *   CSizingControlBar by Cristi Posea <cristip@dundas.com>
 *   http://www.codeguru.com/docking/docking_window2.shtml
 *   titled "Resizable Docking Window 2".
 * To use it, the following steps must be performed:
 *
 * a) Create a new dialog say
 *       CMyBarDlg
 *
 * b) Change its base class from CDialog to cdxCDynamicBarDlg.
 *
 * c) In your mainframe, add a member variable
 *       cdxCDynamicDlgBarT<MyBarDialog> m_wndMyBar;
 *
 * e) Add the following code to your CMainFrame::OnCreate()
 *
 *    if (!m_wndMyBar.Create(_T("My Bar"), this, CSize(200, 100),
 *         TRUE, AFX_IDW_CONTROLBAR_FIRST + 32))
 *    { 
 *			 TRACE0("Failed to create mybar\n");
 *			 return -1;      // fail to create
 *    }
 *
 *		m_wndMyBar.SetBarStyle(m_wndMyBar.GetBarStyle() |
 *			 CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
 *
 *		m_wndMyBar.EnableDocking(CBRS_ALIGN_ANY);
 *		EnableDocking(CBRS_ALIGN_ANY);              // <---- needed only once for the frame
 *		DockControlBar(&m_wndMyBar, AFX_IDW_DOCKBAR_LEFT);
 *
 * f) Refer to URL stated above to learn more about the features of the
 *    CSizingControlBar class.
 */

/*
 * cdxCDynamicBarDlg
 * =================
 * The child dialog.
 */

class cdxCDynamicBarDlg : public cdxCDynamicChildDlg
{
	DECLARE_DYNAMIC(cdxCDynamicBarDlg);

	friend class cdxCDynamicBar;

public:
	const UINT	m_nID;

public:
	cdxCDynamicBarDlg(UINT idd, CWnd *pParent = NULL) : m_nID(idd), cdxCDynamicChildDlg(idd,pParent) { }
	virtual ~cdxCDynamicBarDlg() {}

	//
	// Create() without parameters :)
	//

	virtual bool Create(cdxCDynamicBar *pBar);

	//
	// this handler might be used to update things
	//
protected:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) { UpdateDialogControls(pTarget,bDisableIfNoHndler); }

	//
	// this catches OnOK, OnCancel and OnClose
	// to protect the dialog from being closed accidentially
	//
protected:
	virtual void OnOK() {}
	virtual void OnCancel() {}
	afx_msg void OnClose() { OnCancel(); }

	DECLARE_MESSAGE_MAP();
};

/*
 * cdxCDynamicBar
 * ==============
 * The bar.
 */

class cdxCDynamicBar : public CSizingControlBar
{
	DECLARE_DYNAMIC(cdxCDynamicBar);

private:
	cdxCDynamicBarDlg	& m_rDlg;
	CRect					m_rectBorder;

public:
	cdxCDynamicBar(cdxCDynamicBarDlg & rDlg) : m_rDlg(rDlg), m_rectBorder(0,0,0,0) {}
	virtual ~cdxCDynamicBar() {}

// Attributes
public:
    virtual BOOL Create(LPCTSTR lpszWindowName, CWnd* pParentWnd,
        CSize sizeDefault, BOOL bHasGripper, UINT nID,
        DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cdxCDynamicBar)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	// Generated message map functions
protected:
	//{{AFX_MSG(cdxCDynamicBar)
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();
};

/*
 * cdxCDynamicBarT
 * ===============
 * A nice template class, makes life easier :)
 */

template<class DLG>
class cdxCDynamicBarT : public cdxCDynamicBar
{
public:
	DLG		m_wndDlg;

public:
	cdxCDynamicBarT() : m_wndDlg(), cdxCDynamicBar(m_wndDlg) {}
	virtual ~cdxCDynamicBarT() {  m_wndDlg.DestroyWindow(); cdxCDynamicBar::DestroyWindow(); }
};

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicBarDlg inlines
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDXCDYNAMICBAR_H__910C28F6_6854_11D3_803A_000000000000__INCLUDED_)
