// DSAddIn.h : header file
//

#if !defined(AFX_DSADDIN_H__3F8385DE_5079_4944_A01B_236F76A0E901__INCLUDED_)
#define AFX_DSADDIN_H__3F8385DE_5079_4944_A01B_236F76A0E901__INCLUDED_


#include "ToAddToDistribution/TestRunnerDSPluginVC6.h"
#include "COMHelper.h"

// {F193CE54-716C-41CB-80B2-FA74CA3EE2AC}
// DEFINE_GUID(CLSID_DSAddIn,
// 0xf193ce54, 0x716c, 0x41cb, 0x80, 0xb2, 0xfa, 0x74, 0xca, 0x3e, 0xe2, 0xac);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>,
	public IDSAddIn,
  public ITestRunnerDSPlugin
{
public:
  DECLARE_REGISTRY_RESOURCEID( IDR_DSADDIN)

  CDSAddIn(): classRegistrationId_( 0) {}
  ~CDSAddIn();

	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
    COM_INTERFACE_ENTRY(ITestRunnerDSPlugin)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

  DECLARE_CLASSFACTORY_SINGLETON( CDSAddIn)

// IDSAddIns
public:
	STDMETHOD( OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD( OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

// ITestRunnerDSPlugin
  STDMETHOD( goToLineInSourceCode)( BSTR fileName, int lineNumber);


protected:
  CComPtr< IApplication> pIApp_;
  DWORD classRegistrationId_;
	DWORD m_dwCookie;
  
  static COMUtility::COMExceptionThrower cex_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSADDIN_H__3F8385DE_5079_4944_A01B_236F76A0E901__INCLUDED)
