#if !defined(AFX_CDXCDYNAMICPROPSHEET_H__82427297_6456_11D3_802D_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICPROPSHEET_H__82427297_6456_11D3_802D_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// cdxCDynamicPropSheet.h : header file
//

#include	"cdxCDynamicWndEx.h"
#pragma warning(disable: 4100)

class cdxCDynamicPropPage;

/*
 * cdxCDynamicPropSheet
 * ====================
 * Dynamic property sheet.
 */

class cdxCDynamicPropSheet : public CPropertySheet, public cdxCDynamicWndEx
{
	DECLARE_DYNCREATE(cdxCDynamicPropSheet);

	enum { flDefault = flAntiFlicker|flSizeIcon|flSWPCopyBits };

	friend class cdxCDynamicPropPage;

private:
	Position	m_PagePos;
	bool		m_bHasPos;

public:
	cdxCDynamicPropSheet(Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(UINT sheetAutoPosID, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(UINT sheetAutoPosID, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(LPCTSTR lpszSheetAutoPosID, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicPropSheet(LPCTSTR lpszSheetAutoPosID, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0, Freedom fd = fdAll, UINT nFlags = flDefault);
	virtual ~cdxCDynamicPropSheet() { DoOnDestroy(); }

	// ops
public:
	virtual void AddPage( cdxCDynamicPropPage & rPage );
	virtual void RemovePage( cdxCDynamicPropPage & rPage );
	void AddPage( cdxCDynamicPropPage *pPage ) { ASSERT(pPage != NULL); AddPage(*pPage); }
	void RemovePage( cdxCDynamicPropPage *pPage ) { ASSERT(pPage != NULL); RemovePage(*pPage); }
	void RemovePage( int nPage );

	BOOL IsWizard() const { return (m_psh.dwFlags & PSH_WIZARD) != 0; }

	// events
protected:
	virtual void OnInitPage(cdxCDynamicPropPage & rPage);
	virtual void OnSetActive(cdxCDynamicPropPage & rPage, BOOL bStatus) { if(IsWindow() && IsWizard()) Layout(); }
	virtual void OnKillActive(cdxCDynamicPropPage & rPage, BOOL bStatus) {}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cdxCDynamicPropSheet)
	public:
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(cdxCDynamicPropSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();
	DECLARE_DYNAMIC_MAP();
};

/*
 * cdxCDynamicPropPage
 * ===================
 * The page for our sheet.
 */

class cdxCDynamicPropPage : public CPropertyPage, public cdxCDynamicWnd
{
	DECLARE_DYNCREATE(cdxCDynamicPropPage)

	friend class cdxCDynamicPropSheet;

	enum { flDefault = flAntiFlicker };

private:
	cdxCDynamicPropSheet	*m_pSheet;
	bool						m_bFirstHit;

public:
	cdxCDynamicPropPage() : cdxCDynamicWnd(fdAll,flDefault), m_pSheet(NULL), m_bFirstHit(false) {}
	cdxCDynamicPropPage(UINT nID, UINT nIDCaption = 0) : CPropertyPage(nID,nIDCaption), cdxCDynamicWnd(fdAll,flDefault), m_pSheet(NULL), m_bFirstHit(false) {}
	cdxCDynamicPropPage(LPCTSTR lpszID, UINT nIDCaption = 0) : CPropertyPage(lpszID,nIDCaption), cdxCDynamicWnd(fdAll,flDefault), m_pSheet(NULL), m_bFirstHit(false) {}
	virtual ~cdxCDynamicPropPage() { DoOnDestroy(); }

	cdxCDynamicPropSheet *GetSheet() const { return m_pSheet; }

// Dialog Data
	//{{AFX_DATA(cdxCDynamicPropPage)
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(cdxCDynamicPropPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(cdxCDynamicPropPage)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//////////////////////////////////////////////////////////////////////
// inlines
//////////////////////////////////////////////////////////////////////

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(Freedom fd, UINT nFlags)
:	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(nIDCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(pszCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(UINT sheetAutoPosID, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(nIDCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
	if(sheetAutoPosID)
		ActivateAutoPos(sheetAutoPosID);
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(UINT sheetAutoPosID, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(pszCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
	if(sheetAutoPosID)
		ActivateAutoPos(sheetAutoPosID);
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(LPCTSTR lpszSheetAutoPosID, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(nIDCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
	if(lpszSheetAutoPosID && *lpszSheetAutoPosID)
		ActivateAutoPos(lpszSheetAutoPosID);
}

inline cdxCDynamicPropSheet::cdxCDynamicPropSheet(LPCTSTR lpszSheetAutoPosID, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage, Freedom fd, UINT nFlags)
:	CPropertySheet(pszCaption,pParentWnd,iSelectPage),
	cdxCDynamicWndEx(fd,nFlags),
	m_bHasPos(false)
{
	if(lpszSheetAutoPosID && *lpszSheetAutoPosID)
		ActivateAutoPos(lpszSheetAutoPosID);
}

#pragma warning(default: 4100)

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDXCDYNAMICPROPSHEET_H__82427297_6456_11D3_802D_000000000000__INCLUDED_)
