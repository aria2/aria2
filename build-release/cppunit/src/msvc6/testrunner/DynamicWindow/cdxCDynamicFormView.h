#if !defined(AFX_CDXCDYNAMICFORMVIEW_H__82427295_6456_11D3_802D_000000000000__INCLUDED_)
#define AFX_CDXCDYNAMICFORMVIEW_H__82427295_6456_11D3_802D_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// cdxCDynamicFormView.h : header file
//

#include	"cdxCDynamicWnd.h"

/*
 * cdxCDynamicFormView
 * ===================
 * My dynamic form view.
 */

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class cdxCDynamicFormView : public CFormView, public cdxCDynamicWnd
{
	DECLARE_DYNCREATE(cdxCDynamicFormView);

	enum { flDefault = flAntiFlicker };

public:
	cdxCDynamicFormView(UINT idd = 0, Freedom fd = fdAll, UINT nFlags = flDefault) : CFormView(idd), cdxCDynamicWnd(fd,nFlags) { m_bUseScrollPos = true; }
	cdxCDynamicFormView(LPCTSTR lpszTemplateName, Freedom fd = fdAll, UINT nFlags = flDefault) : CFormView(lpszTemplateName), cdxCDynamicWnd(fd,nFlags) { m_bUseScrollPos = true; }
	virtual ~cdxCDynamicFormView() { DoOnDestroy(); }

public:
	virtual void OnInitialUpdate();
	virtual BOOL DestroyWindow();

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDXCDYNAMICFORMVIEW_H__82427295_6456_11D3_802D_000000000000__INCLUDED_)
