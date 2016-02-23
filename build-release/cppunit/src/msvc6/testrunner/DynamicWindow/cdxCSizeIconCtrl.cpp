// cdxCSizeIconCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "cdxCSizeIconCtrl.h"

#include	<winuser.h>

#ifndef OBM_SIZE
#define	OBM_SIZE		32766
#pragma message("*** NOTE[cdxCSizeIconCtrl.cpp]: Please define OEMRESOURCE in your project settings !")
// taken from WinresRc.h
// if not used for any reason
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 4100)

/////////////////////////////////////////////////////////////////////////////
// cdxCSizeIconCtrl::AutoOEMImageList
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// construction
/////////////////////////////////////////////////////////////////////////////

/*
 * one-step construction for my image list
 * (allows to use the AutoOEMImageList as static member)
 */

cdxCSizeIconCtrl::AutoOEMImageList::AutoOEMImageList(UINT nBitmapID, COLORREF crMask)
{
	CBitmap	cbmp;
	BITMAP	bmp;
	VERIFY( cbmp.LoadOEMBitmap(nBitmapID) );
	VERIFY( cbmp.GetBitmap(&bmp) );

	m_szImage.cx	=	bmp.bmWidth;
	m_szImage.cy	=	bmp.bmHeight;
  
	InitCommonControls();

	VERIFY( Create(bmp.bmWidth,bmp.bmHeight,ILC_COLOR16|ILC_MASK,0,1) );
	int	i	=	Add(&cbmp,crMask);
	ASSERT(i == 0);
}

/////////////////////////////////////////////////////////////////////////////
// cdxCSizeIconCtrl
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(cdxCSizeIconCtrl,CScrollBar);

/////////////////////////////////////////////////////////////////////////////

cdxCSizeIconCtrl::AutoOEMImageList	cdxCSizeIconCtrl::M_ilImage(OBM_SIZE,::GetSysColor(COLOR_BTNFACE));
HCURSOR										cdxCSizeIconCtrl::M_hcSize	=	::LoadCursor(NULL,IDC_SIZENWSE);

/////////////////////////////////////////////////////////////////////////////
// construction
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCSizeIconCtrl, CScrollBar)
	//{{AFX_MSG_MAP(cdxCSizeIconCtrl)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCSizeIconCtrl inlines
/////////////////////////////////////////////////////////////////////////////

/*
 * create short-cut
 */

BOOL cdxCSizeIconCtrl::Create(CWnd *pParent, UINT id)
{
	ASSERT(pParent != NULL);
	CRect	rect;pParent->GetClientRect(&rect);
	if(!CScrollBar::Create(		SBS_SIZEBOX|SBS_SIZEBOXBOTTOMRIGHTALIGN|
										WS_CHILD,
										rect,
										pParent,id))
		return FALSE;

	VERIFY( ModifyStyleEx(0,WS_EX_TRANSPARENT) );
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// cdxCSizeIconCtrl message handlers
/////////////////////////////////////////////////////////////////////////////

/*
 * draw icon
 */

void cdxCSizeIconCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if(GetParent() && (!GetParent()->IsZoomed() || !m_bReflectParentState))
	{
		CRect	rect;GetClientRect(&rect);
		CSize	sz	=	M_ilImage.Size();

		VERIFY( M_ilImage.Draw(	&dc,
								0,
								CPoint(rect.right - sz.cx,rect.bottom - sz.cy),
								ILD_NORMAL|ILD_TRANSPARENT) );
	}
}

/////////////////////////////////////////////////////////////////////////////
// cdxCSizeIconCtrl Cursor
/////////////////////////////////////////////////////////////////////////////

/*
 * set the cursor.
 */

BOOL cdxCSizeIconCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if(GetParent() && (!GetParent()->IsZoomed() || !m_bReflectParentState))
		::SetCursor((nHitTest == HTCLIENT) ? M_hcSize : NULL);
	return TRUE;
}

/*
 * catch Doubleclick - if you don't do that,
 * the window will be maximized if you double-blick
 * the control.
 * Don't know why, but it's annoying.
 */

void cdxCSizeIconCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
//	CScrollBar::OnLButtonDblClk(nFlags, point);
}
