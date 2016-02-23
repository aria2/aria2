#if !defined(AFX_PROGRESSBAR_H__F2CB2DBB_467B_4978_829B_CAD101EA4B8A__INCLUDED_)
#define AFX_PROGRESSBAR_H__F2CB2DBB_467B_4978_829B_CAD101EA4B8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class ProgressBar : public CWnd
{
public:
	ProgressBar();
	virtual ~ProgressBar();

  void step( bool successful );

  int scale( int value );

  void reset();

  void start( int total );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgressBar)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(ProgressBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP();


protected:
  void paint( CDC &dc );

  void paintBackground( CDC &dc );

  void paintStatus( CDC &dc );

  COLORREF getStatusColor();

  void paintStep( int startX, 
                  int endX );

private:
  CRect m_bounds;
  bool m_error;
  int m_total;
  int m_progress;
  int m_progressX;
};



// Get the current color
inline COLORREF 
ProgressBar::getStatusColor ()
{ 
  return m_error ? RGB (255, 0, 0) : 
                   RGB (0, 255, 0); 
}


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSBAR_H__F2CB2DBB_467B_4978_829B_CAD101EA4B8A__INCLUDED_)
