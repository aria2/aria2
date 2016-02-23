#include "stdafx.h"

#include "ResourceLoaders.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



extern HINSTANCE g_testRunnerResource;

CString loadCString(UINT stringId)
{
  CString string;
  
  HRSRC stringRes = ::FindResource( g_testRunnerResource, 
                                   MAKEINTRESOURCE( (stringId>>4) + 1),
                                   RT_STRING );
  if ( stringRes )
  {
    int stringLen = ::SizeofResource( g_testRunnerResource, 
                                      stringRes) / sizeof(TCHAR);
    if (stringLen > 0)
    {
      LPTSTR stringBuffer = string.GetBuffer( stringLen+2 );
      int realStringLen = ::LoadString( g_testRunnerResource, 
                                        stringId, 
                                        stringBuffer, 
                                        (stringLen+1)*2 );
      string.ReleaseBuffer( realStringLen );

      ASSERT(realStringLen > 0);
    }
  }

  ASSERT( !string.IsEmpty() );
  return string;
}

