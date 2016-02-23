// AddInMod.cpp : implementation file
//

#include "stdafx.h"

#include "DSPlugIn.h"
#include "COMHelper.h"
#include "DSAddIn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


COMUtility::COMExceptionThrower CDSAddIn::cex_;

CDSAddIn::~CDSAddIn( void)
{
}

// This is called when the user first loads the add-in, and on start-up
//  of each subsequent Developer Studio session
STDMETHODIMP CDSAddIn::OnConnection(IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection)
{
  HRESULT result = S_OK;

  try
  {
    CComPtr< IUnknown> pIUnk;

	  AFX_MANAGE_STATE(AfxGetStaticModuleState());

    cex_ = _Module.GetClassObject( GetObjectCLSID(), IID_IUnknown, reinterpret_cast<void**>(&pIUnk));

    cex_ = CoRegisterClassObject( 
      GetObjectCLSID(),
      pIUnk,
      CLSCTX_LOCAL_SERVER,
      REGCLS_MULTIPLEUSE,
      &classRegistrationId_
      );

    pIApp_ = pApp;

    m_dwCookie = dwCookie;
    *OnConnection = VARIANT_TRUE;
  }
  catch( const std::bad_cast&)
  {
    *OnConnection = VARIANT_FALSE;
  }
  catch( const _com_error&)
  {
    *OnConnection = VARIANT_FALSE;
  }

  return result;
}

// This is called on shut-down, and also when the user unloads the add-in
STDMETHODIMP CDSAddIn::OnDisconnection(VARIANT_BOOL bLastTime)
{
  pIApp_.Release();
  CoRevokeClassObject( classRegistrationId_);

	return S_OK;
}


// ITestRunnerDSPlugin
STDMETHODIMP CDSAddIn::goToLineInSourceCode( BSTR fileName, int lineNumber)
{
  HRESULT result = S_OK;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

  try
  {
    CComPtr< IDispatch> tmp;
    CComPtr< IDocuments> pIDocuments;
    CComPtr< ITextDocument> pITextDocu;
    CComPtr< ITextSelection> pITextSel;

    cex_ = pIApp_->get_Documents( &tmp);
    pIDocuments.Attach( COMUtility::interface_cast<IDocuments>( tmp.p));
    tmp.Release();
    cex_ = pIDocuments->Open( fileName,
                             CComVariant(),
                             CComVariant(),
                             &tmp);
    pITextDocu.Attach( COMUtility::interface_cast< ITextDocument>( tmp.p));
    tmp.Release();
    cex_ = pITextDocu->get_Selection( &tmp);
    pITextSel.Attach( COMUtility::interface_cast< ITextSelection>( tmp.p));
    cex_ = pITextSel->GoToLine( lineNumber, CComVariant( 1));
  }
  catch( const std::bad_cast&)
  {
    result = E_FAIL;
  }
  catch( const _com_error&)
  {
    result = E_FAIL;
  }
  
  return result;
}
