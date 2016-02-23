/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Apr 13 11:47:16 2002
 */
/* Compiler settings for G:\prg\vc\Lib\cppunit\src\msvc6\DSPlugIn\TestRunnerDSPlugin.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __TestRunnerDSPluginVC6_h__
#define __TestRunnerDSPluginVC6_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ITestRunnerDSPlugin_FWD_DEFINED__
#define __ITestRunnerDSPlugin_FWD_DEFINED__
typedef interface ITestRunnerDSPlugin ITestRunnerDSPlugin;
#endif 	/* __ITestRunnerDSPlugin_FWD_DEFINED__ */


#ifndef __DSAddIn_FWD_DEFINED__
#define __DSAddIn_FWD_DEFINED__

#ifdef __cplusplus
typedef class DSAddIn DSAddIn;
#else
typedef struct DSAddIn DSAddIn;
#endif /* __cplusplus */

#endif 	/* __DSAddIn_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ITestRunnerDSPlugin_INTERFACE_DEFINED__
#define __ITestRunnerDSPlugin_INTERFACE_DEFINED__

/* interface ITestRunnerDSPlugin */
/* [oleautomation][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ITestRunnerDSPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3ADE0E37-5A56-4a68-BD8D-67E9E7502971")
    ITestRunnerDSPlugin : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE goToLineInSourceCode( 
            /* [in] */ BSTR fileName,
            /* [in] */ int lineNumber) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITestRunnerDSPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITestRunnerDSPlugin __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITestRunnerDSPlugin __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITestRunnerDSPlugin __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *goToLineInSourceCode )( 
            ITestRunnerDSPlugin __RPC_FAR * This,
            /* [in] */ BSTR fileName,
            /* [in] */ int lineNumber);
        
        END_INTERFACE
    } ITestRunnerDSPluginVtbl;

    interface ITestRunnerDSPlugin
    {
        CONST_VTBL struct ITestRunnerDSPluginVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITestRunnerDSPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITestRunnerDSPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITestRunnerDSPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITestRunnerDSPlugin_goToLineInSourceCode(This,fileName,lineNumber)	\
    (This)->lpVtbl -> goToLineInSourceCode(This,fileName,lineNumber)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ITestRunnerDSPlugin_goToLineInSourceCode_Proxy( 
    ITestRunnerDSPlugin __RPC_FAR * This,
    /* [in] */ BSTR fileName,
    /* [in] */ int lineNumber);


void __RPC_STUB ITestRunnerDSPlugin_goToLineInSourceCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITestRunnerDSPlugin_INTERFACE_DEFINED__ */



#ifndef __TestRunnerDSPluginLib_LIBRARY_DEFINED__
#define __TestRunnerDSPluginLib_LIBRARY_DEFINED__

/* library TestRunnerDSPluginLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_TestRunnerDSPluginLib;

EXTERN_C const CLSID CLSID_DSAddIn;

#ifdef __cplusplus

class DECLSPEC_UUID("F193CE54-716C-41CB-80B2-FA74CA3EE2AC")
DSAddIn;
#endif
#endif /* __TestRunnerDSPluginLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
