#if !defined(AFX_MSDEVCALLERLISTCTRL_H__4B5EE0C1_D251_45CF_BBD1_D5003C80B238__INCLUDED_)
#define AFX_MSDEVCALLERLISTCTRL_H__4B5EE0C1_D251_45CF_BBD1_D5003C80B238__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsDevCallerListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MsDevCallerListCtrl window

class MsDevCallerListCtrl : public CListCtrl
{
// Construction
public:
	MsDevCallerListCtrl();

  void setLineNumberSubItem( int subItemIndex );
  void setFileNameSubItem( int fileNameItemIndex );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MsDevCallerListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~MsDevCallerListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(MsDevCallerListCtrl)
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
  int m_lineNumberSubItem;
  int m_fileNameSubItem;
  bool m_initialized;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSDEVCALLERLISTCTRL_H__4B5EE0C1_D251_45CF_BBD1_D5003C80B238__INCLUDED_)
