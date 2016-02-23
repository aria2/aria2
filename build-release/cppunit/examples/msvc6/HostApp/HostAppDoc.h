// HostAppDoc.h : interface of the CHostAppDoc class
//
/////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_HOSTAPPDOC_H)
#define AFX_HOSTAPPDOC_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CHostAppDoc : public CDocument
{
protected: // create from serialization only
    CHostAppDoc();
    DECLARE_DYNCREATE(CHostAppDoc)

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHostAppDoc)
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CHostAppDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    //{{AFX_MSG(CHostAppDoc)
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTAPPDOC_H)
