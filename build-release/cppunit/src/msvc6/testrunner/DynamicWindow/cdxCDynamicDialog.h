#if !defined(AFX_CDXCDYNAMICDIALOG_H__E8F2A005_63C6_11D3_802B_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICDIALOG_H__E8F2A005_63C6_11D3_802B_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// cdxCDynamicDialog.h : header file
//

#include	"cdxCDynamicWndEx.h"

/*
 * cdxCDynamicDialog
 * =================
 * A new resizable dialog.
 * This should be the base-class for your normal dialogs.
 * This class supports:
 * - A sizing icon
 * - AutoPositioning (stores last position automatically and stuff)
 * - Anti-Flickering system.
 * - And of course, it provides
 *   the Dynamic child control system DcCS by codex design
 */

class cdxCDynamicDialog : public CDialog, public cdxCDynamicWndEx
{
	DECLARE_DYNAMIC(cdxCDynamicDialog);

public:
	enum { flDefault = flAntiFlicker|flSizeIcon };

public:
	cdxCDynamicDialog(UINT idd = 0, CWnd* pParent = NULL, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicDialog(LPCTSTR lpszTemplateName, CWnd* pParent = NULL, Freedom fd = fdAll, UINT nFlags = flDefault);
	virtual ~cdxCDynamicDialog() { DoOnDestroy(); }

public:
	virtual BOOL DestroyWindow();

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnDestroy();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP();
};

/*
 * cdxCDynamicChildDlg
 * ===================
 * Use this dialog class instead of cdxCDynamicDialog if
 * you create dialogs which you want to embedd as child
 * controls.
 * In that case, this dialog is far more straight forward.
 * This class provides:
 * - NO sizing icon
 * - NO auto anti-flickering (since the dialog itself won't be moved by hand)
 * - NO auto-positioning
 * - But of course, it provides
 *   the Dynamic child control system DcCS by codex design
 */

class cdxCDynamicChildDlg : public cdxCDynamicDialog
{
	DECLARE_DYNAMIC(cdxCDynamicChildDlg);

public:
	enum { flDefault = flAntiFlicker };

public:
	cdxCDynamicChildDlg(UINT idd = 0, CWnd* pParent = NULL, Freedom fd = fdAll, UINT nFlags = flDefault);
	cdxCDynamicChildDlg(LPCTSTR lpszTemplateName, CWnd* pParent = NULL, Freedom fd = fdAll, UINT nFlags = flDefault);
	virtual ~cdxCDynamicChildDlg() { DoOnDestroy(); }
};

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicDialog Inlines
/////////////////////////////////////////////////////////////////////////////

inline cdxCDynamicDialog::cdxCDynamicDialog(UINT idd, CWnd* pParent, Freedom fd, UINT nFlags)
:	CDialog(idd,pParent),
	cdxCDynamicWndEx(fd,nFlags)
{
	if(idd)
		ActivateAutoPos(idd);
}

inline cdxCDynamicDialog::cdxCDynamicDialog(LPCTSTR lpszTemplateName, CWnd* pParent, Freedom fd, UINT nFlags)
:	CDialog(lpszTemplateName,pParent),
	cdxCDynamicWndEx(fd,nFlags)
{
	if(lpszTemplateName && *lpszTemplateName)
		ActivateAutoPos(lpszTemplateName);
}

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicChildDlg Inlines
/////////////////////////////////////////////////////////////////////////////

inline cdxCDynamicChildDlg::cdxCDynamicChildDlg(UINT idd, CWnd* pParent, Freedom fd, UINT nFlags)
:	cdxCDynamicDialog(idd,pParent,fd,nFlags)
{
	m_bUseScrollPos	=	true;		// if you create scollbars I will use them ;)
	NoAutoPos();						// not in this case....
}

inline cdxCDynamicChildDlg::cdxCDynamicChildDlg(LPCTSTR lpszTemplateName, CWnd* pParent, Freedom fd, UINT nFlags)
:	cdxCDynamicDialog(lpszTemplateName,pParent,fd,nFlags)
{
	m_bUseScrollPos	=	true;		// if you create scollbars I will use them ;)
	NoAutoPos();						// not in this case....
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDXCDYNAMICDIALOG_H__E8F2A005_63C6_11D3_802B_000000000000__INCLUDED_)
