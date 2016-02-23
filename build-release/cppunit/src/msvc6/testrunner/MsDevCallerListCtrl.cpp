// MsDevCallerListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <atlbase.h>

#include "MsDevCallerListCtrl.h"
#include <msvc6/testrunner/TestRunner.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// VC6 IDE Handler
// //////////////////////////////////////////////////////////////////
#if _MSC_VER == 1200    // VC++ 6

#include <msvc6/DSPlugin/TestRunnerDSPluginVC6.h>


namespace VC6IdeHandler {
   static bool initialize()
   {
      return SUCCEEDED( CoInitialize(NULL) );
   }

   static void uninitialize( bool initialized )
   {
      if ( initialized )
         CoUninitialize();
   }

   static void goToLineInSourceCode( CString fileName, int line )
   {

      CComPtr< ITestRunnerDSPlugin> pIDSPlugin;
      HRESULT hr = CoCreateInstance( CLSID_DSAddIn, 
                                     NULL, 
                                     CLSCTX_LOCAL_SERVER, 
                                     IID_ITestRunnerDSPlugin, 
                                     reinterpret_cast< void**>(&pIDSPlugin) );
      if ( SUCCEEDED( hr ) )
      {
         pIDSPlugin->goToLineInSourceCode( CComBSTR( fileName ), 
                                           line );
      }
   }
} // namespace VC6IdeHandler

namespace IDEHandler = VC6IdeHandler;


// VC7 IDE Handler
// //////////////////////////////////////////////////////////////////

#elif _MSC_VER >= 1300     // VC++ 7 or more

#include <initguid.h>
#include <assert.h>

#pragma warning( disable : 4278 )
#pragma warning( disable : 4146 )
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("7.0") lcid("0") raw_interfaces_only named_guids
#pragma warning( default : 4146 )
#pragma warning( default : 4278 )



namespace VC7IdeHandler {
   static bool initialize()
   {
      return true;
   }

   static void uninitialize( bool initialized )
   {
   }

   static void goToLineInSourceCode( CString fileName, int line )
   {
      USES_CONVERSION;

      CComPtr< IRunningObjectTable > pIRunningObjectTable;
      HRESULT hr = ::GetRunningObjectTable( 0, &pIRunningObjectTable );

      CComPtr< IEnumMoniker > pIEnumMoniker;
      hr = pIRunningObjectTable->EnumRunning( &pIEnumMoniker );

      CComPtr< EnvDTE::_DTE > pIEnvDTE;
      while ( true )
      {
         ULONG celtFetched;
         CComPtr< IMoniker > pIMoniker;
         if ( S_OK != pIEnumMoniker->Next( 1, &pIMoniker, &celtFetched ) )
            break;

         CComPtr< IBindCtx > pIBindCtx; 
         hr = ::CreateBindCtx( NULL, &pIBindCtx ); 

         LPOLESTR pszDisplayName;
         pIMoniker->GetDisplayName( pIBindCtx, NULL, &pszDisplayName );

         TRACE( "Moniker %s\n", W2A( pszDisplayName ) );

         CString strDisplayName( pszDisplayName );
         CComPtr< IMalloc > pIMalloc;
         ::CoGetMalloc( 1, &pIMalloc );
         pIMalloc->Free( pszDisplayName );

         if ( strDisplayName.Right( 4 ) == _T(".sln") 
              || strDisplayName.Find( _T("VisualStudio.DTE") ) >= 0 )
         {
            CComPtr< IUnknown > pIUnknown;
            pIRunningObjectTable->GetObject( pIMoniker, &pIUnknown );
            pIUnknown->QueryInterface( &pIEnvDTE );
            if( pIEnvDTE.p )
               break;
         }
      }

      if ( pIEnvDTE.p )
      {
         CComPtr< EnvDTE::Documents > pIDocuments;
         HRESULT result = pIEnvDTE->get_Documents( &pIDocuments );
         if ( !SUCCEEDED( result ) )
            return;
         
         assert( pIDocuments.p );
         
         CComPtr< EnvDTE::Document > pIDocument;
         CComBSTR bstrFileName( fileName );
         CComVariant type=_T("Text");
         CComVariant read=_T("False");
         result = pIDocuments->Open( bstrFileName, 
                                     type.bstrVal,
                                     read.bVal, 
                                     &pIDocument );
         if ( !SUCCEEDED( result ) )
            return;
         
         assert( pIDocument.p );
         
         CComPtr< IDispatch > pIDispatch;
         result = pIDocument->get_Selection( &pIDispatch );
         if ( !SUCCEEDED( result ) )
            return;
         
         CComPtr< EnvDTE::TextSelection > pITextSelection;
         pIDispatch->QueryInterface( &pITextSelection );
         
         assert( pITextSelection.p );
         
         result = pITextSelection->GotoLine( line, TRUE );
         if ( !SUCCEEDED( result ) )
            return;
      }
   }

} // namespace VC7IdeHandler


namespace IDEHandler = VC7IdeHandler;


#else
#error Unsupported VC++ version.
#endif





/////////////////////////////////////////////////////////////////////////////
// MsDevCallerListCtrl



MsDevCallerListCtrl::MsDevCallerListCtrl()
    : m_lineNumberSubItem( 3 )
    , m_fileNameSubItem( 4 )
{
   m_initialized = IDEHandler::initialize();
}

MsDevCallerListCtrl::~MsDevCallerListCtrl()
{
   IDEHandler::uninitialize( m_initialized );
}


void 
MsDevCallerListCtrl::setLineNumberSubItem( int subItemIndex )
{
  m_lineNumberSubItem = subItemIndex;
}


void 
MsDevCallerListCtrl::setFileNameSubItem( int fileNameItemIndex )
{
  m_fileNameSubItem = fileNameItemIndex;
}


BEGIN_MESSAGE_MAP(MsDevCallerListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(MsDevCallerListCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MsDevCallerListCtrl message handlers

void MsDevCallerListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
   // get index of selected item
   POSITION pos = GetFirstSelectedItemPosition();
   int hotItem = GetNextSelectedItem(pos);

   CString lineNumber = GetItemText( hotItem, m_lineNumberSubItem);
   CString fileName = GetItemText( hotItem, m_fileNameSubItem);

   IDEHandler::goToLineInSourceCode( fileName, _ttoi( lineNumber) );

   *pResult = 0;
}