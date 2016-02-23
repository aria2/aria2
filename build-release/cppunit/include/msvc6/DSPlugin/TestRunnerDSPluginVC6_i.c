/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Apr 13 11:47:16 2002
 */
/* Compiler settings for G:\prg\vc\Lib\cppunit\src\msvc6\DSPlugIn\TestRunnerDSPlugin.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_ITestRunnerDSPlugin = {0x3ADE0E37,0x5A56,0x4a68,{0xBD,0x8D,0x67,0xE9,0xE7,0x50,0x29,0x71}};


const IID LIBID_TestRunnerDSPluginLib = {0x3ADE0E38,0x5A56,0x4a68,{0xBD,0x8D,0x67,0xE9,0xE7,0x50,0x29,0x71}};


const CLSID CLSID_DSAddIn = {0xF193CE54,0x716C,0x41CB,{0x80,0xB2,0xFA,0x74,0xCA,0x3E,0xE2,0xAC}};


#ifdef __cplusplus
}
#endif

