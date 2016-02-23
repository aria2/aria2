#if !defined(AFX_CDXCSIZEICONCTRL_H__9B4AD1C3_8AA5_11D2_BE9C_000000000000__INCLUDED_)
#define AFX_CDXCSIZEICONCTRL_H__9B4AD1C3_8AA5_11D2_BE9C_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// cdxCSizeIconCtrl.h : header file
//

//
// cdxCSizeIconCtrl.h : header file
// -----------------------------------------------------------------------
// Author:  Hans Bühler (hb@codex-design.de)
//          codex design (http://www.codex-design.de)
// Version: 1.3
// -----------------------------------------------------------------------
// Changes for 1.1:
// - cdxCSizeIconCtrl catches left-mb-doubleclick what caused the window
//	  to get maximized for any reason. 
// Changes for 1.2:
// - Ability to check parent's state: If it is zoomed, the control won't
//   draw a sizing icon.
// Changes for 1.3:
// - Icon now has proper colors.
// -----------------------------------------------------------------------
// Comments welcome.
//

/*
 * cdxCSizeIconCtrl
 * ================
 * A simple class that is a size-icon.
 *
 * (w)Nov.1998 mailto:hans.buehler@student.hu-berlin.de,
 *    codex design
 */

class cdxCSizeIconCtrl : public CScrollBar
{
	DECLARE_DYNAMIC(cdxCSizeIconCtrl);

public:
	class AutoOEMImageList : public CImageList
	{
	private:
		CSize	m_szImage;

	public:
		AutoOEMImageList(UINT nBitmapID, COLORREF crMask);
		virtual ~AutoOEMImageList() {}

		const CSize & Size() const { return m_szImage; }
	};

private:
	bool	m_bCapture;
public:
	bool	m_bReflectParentState;

public:
	cdxCSizeIconCtrl(bool bReflectParentState = true) : m_bCapture(false), m_bReflectParentState(bReflectParentState) {}
	virtual ~cdxCSizeIconCtrl() {}

	virtual BOOL Create(CWnd *pParent, UINT id = AFX_IDW_SIZE_BOX);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cdxCSizeIconCtrl)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(cdxCSizeIconCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();

	//
	// static members
	//
public:
	static AutoOEMImageList	M_ilImage;
	static HCURSOR				M_hcSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDXCSIZEICONCTRL_H__9B4AD1C3_8AA5_11D2_BE9C_000000000000__INCLUDED_)
