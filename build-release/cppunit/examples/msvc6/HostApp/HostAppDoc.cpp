// HostAppDoc.cpp : implementation of the CHostAppDoc class
//

#include "stdafx.h"
#include "HostApp.h"

#include "HostAppDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHostAppDoc

IMPLEMENT_DYNCREATE(CHostAppDoc, CDocument)

BEGIN_MESSAGE_MAP(CHostAppDoc, CDocument)
    //{{AFX_MSG_MAP(CHostAppDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHostAppDoc construction/destruction

CHostAppDoc::CHostAppDoc()
{
    // TODO: add one-time construction code here
}

CHostAppDoc::~CHostAppDoc()
{
}



BOOL CHostAppDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CHostAppDoc serialization

void CHostAppDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CHostAppDoc diagnostics

#ifdef _DEBUG
void CHostAppDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CHostAppDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHostAppDoc commands
