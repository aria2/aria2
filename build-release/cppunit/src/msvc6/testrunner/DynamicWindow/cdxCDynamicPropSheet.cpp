// cdxCDynamicPropSheet.cpp : implementation file
//

#include "stdafx.h"
#include "cdxCDynamicPropSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 4706)


/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicPropSheet
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(cdxCDynamicPropSheet, CPropertySheet)

/////////////////////////////////////////////////////////////////////////////
// maps
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(cdxCDynamicPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(cdxCDynamicPropSheet)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*
 * we map the controls by our new dynamic map feature :)
 */

BEGIN_DYNAMIC_MAP(cdxCDynamicPropSheet,cdxCDynamicWnd)
	DYNAMIC_MAP_ENTRY(	ID_WIZNEXT,				mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	ID_WIZFINISH,			mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	ID_WIZBACK,				mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	IDOK,						mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	IDCANCEL,				mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	ID_WIZNEXT,				mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	ID_APPLY_NOW,			mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	IDHELP,					mdRepos,mdRepos	)
	DYNAMIC_MAP_ENTRY(	AFX_IDC_TAB_CONTROL,	mdResize,mdResize	)
	DYNAMIC_MAP_ENTRY(	ID_WIZFINISH+1,		mdResize,mdRepos	)
END_DYNAMIC_MAP()

/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicPropSheet message handlers
/////////////////////////////////////////////////////////////////////////////

/*
 * initialize window
 */

BOOL cdxCDynamicPropSheet::OnInitDialog() 
{
	// initialize window & dynamic manager

	BOOL	b	=	CPropertySheet::OnInitDialog();
	DoInitWindow(*this);

	ModifyStyle(0,WS_CLIPSIBLINGS);

	ASSERT(GetPageCount() > 0);			// NO pages ??
	cdxCDynamicPropPage	*pActive	=	(cdxCDynamicPropPage *)GetActivePage();

	ASSERT(pActive && pActive->IsKindOf(RUNTIME_CLASS(cdxCDynamicPropPage)));
	AddSzControl(*pActive,mdResize,mdResize);
	VERIFY( GetControlPosition(*pActive,m_PagePos) );
	m_bHasPos	=	true;

	return b;
}

void cdxCDynamicPropSheet::AddPage( cdxCDynamicPropPage & rPage )
{
	ASSERT(rPage.m_pSheet == NULL);
	rPage.m_pSheet	=	this;
	CPropertySheet::AddPage(&rPage);
}

void cdxCDynamicPropSheet::RemovePage( cdxCDynamicPropPage & rPage )
{
	ASSERT(rPage.m_pSheet == this);
	rPage.m_pSheet	=	NULL;
}

void cdxCDynamicPropSheet::OnInitPage(cdxCDynamicPropPage & rPage)
{
	ASSERT(::IsWindow(rPage));

	if(m_bHasPos)
		AddSzControl(rPage,m_PagePos);
}

/////////////////////////////////////////////////////////////////////////////

/*
 * map WM_CLOSE to IDCANCEL if it is a modal sheet
 */

void cdxCDynamicPropSheet::OnClose() 
{
	if(!PressButton(PSBTN_CANCEL))
		CPropertySheet::OnClose();
}

/*
 * give us a resizable border
 */

int cdxCDynamicPropSheet::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ModifyStyle(0,WS_THICKFRAME|WS_SYSMENU);
	ModifyStyleEx(0,WS_CLIPCHILDREN);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

BOOL cdxCDynamicPropSheet::DestroyWindow() 
{
	DoOnDestroy();
	return CPropertySheet::DestroyWindow();
}

void cdxCDynamicPropSheet::OnDestroy() 
{
	DoOnDestroy();
	CPropertySheet::OnDestroy();
}

void cdxCDynamicPropSheet::OnSize(UINT nType, int cx, int cy) 
{
	CPropertySheet::OnSize(nType, cx, cy);
	DoOnSize(nType, cx, cy);
}

void cdxCDynamicPropSheet::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CPropertySheet::OnSizing(fwSide, pRect);
	DoOnSizing(fwSide, pRect);
}

void cdxCDynamicPropSheet::OnTimer(UINT nIDEvent) 
{
	CPropertySheet::OnTimer(nIDEvent);
	DoOnTimer(nIDEvent);
}

void cdxCDynamicPropSheet::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CPropertySheet::OnGetMinMaxInfo(lpMMI);
	DoOnGetMinMaxInfo(lpMMI);
}








/////////////////////////////////////////////////////////////////////////////
// cdxCDynamicPropSheet message handlers
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(cdxCDynamicPropPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// creation
/////////////////////////////////////////////////////////////////////////////

void cdxCDynamicPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cdxCDynamicPropPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(cdxCDynamicPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(cdxCDynamicPropPage)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_PARENTNOTIFY()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// active/inactive stuff
/////////////////////////////////////////////////////////////////////////////

BOOL cdxCDynamicPropPage::OnInitDialog() 
{
	ASSERT(m_pSheet != NULL);

	BOOL	b	=	CPropertyPage::OnInitDialog();
	DoInitWindow(*this);
	
	return b;
}

BOOL cdxCDynamicPropPage::OnSetActive() 
{
	BOOL	bGetsActive	=	CPropertyPage::OnSetActive();
	if(bGetsActive && !m_bFirstHit)
	{
		m_pSheet->OnInitPage(*this);
		m_bFirstHit	=	true;
	}
	if(m_pSheet)
		m_pSheet->OnSetActive(*this,bGetsActive);
	return bGetsActive;
}

BOOL cdxCDynamicPropPage::OnKillActive() 
{
	BOOL	bGetsKilled	=	CPropertyPage::OnKillActive();
	if(m_pSheet)
		m_pSheet->OnKillActive(*this,bGetsKilled);
	return bGetsKilled;
}

void cdxCDynamicPropPage::OnSize(UINT nType, int cx, int cy) 
{
	CPropertyPage::OnSize(nType, cx, cy);
	DoOnSize(nType, cx, cy);
}

void cdxCDynamicPropPage::OnTimer(UINT nIDEvent) 
{
	CPropertyPage::OnTimer(nIDEvent);
	DoOnTimer(nIDEvent);
}

void cdxCDynamicPropPage::OnDestroy() 
{
	DoOnDestroy();
	CPropertyPage::OnDestroy();
}

void cdxCDynamicPropPage::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CPropertyPage::OnSizing(fwSide, pRect);
	DoOnSizing(fwSide, pRect);
}

void cdxCDynamicPropPage::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CPropertyPage::OnGetMinMaxInfo(lpMMI);
	DoOnGetMinMaxInfo(lpMMI);
}

void cdxCDynamicPropPage::OnParentNotify(UINT message, LPARAM lParam) 
{
	CPropertyPage::OnParentNotify(message, lParam);
	DoOnParentNotify(message, lParam);
}

