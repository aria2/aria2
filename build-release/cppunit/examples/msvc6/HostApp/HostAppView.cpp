// HostAppView.cpp : implementation of the CHostAppView class
//

#include "stdafx.h"
#include "HostApp.h"

#include "HostAppDoc.h"
#include "HostAppView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHostAppView

IMPLEMENT_DYNCREATE(CHostAppView, CView)

BEGIN_MESSAGE_MAP(CHostAppView, CView)
    //{{AFX_MSG_MAP(CHostAppView)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHostAppView construction/destruction

CHostAppView::CHostAppView()
{
    // TODO: add construction code here

}

CHostAppView::~CHostAppView()
{
}

BOOL CHostAppView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHostAppView drawing

void CHostAppView::OnDraw(CDC* pDC)
{
    CHostAppDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    // TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CHostAppView printing

BOOL CHostAppView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CHostAppView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void CHostAppView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CHostAppView diagnostics

#ifdef _DEBUG
void CHostAppView::AssertValid() const
{
    CView::AssertValid();
}

void CHostAppView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CHostAppDoc* CHostAppView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHostAppDoc)));
    return (CHostAppDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHostAppView message handlers
