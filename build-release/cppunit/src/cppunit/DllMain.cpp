#define WIN32_LEAN_AND_MEAN 
#define NOGDI
#define NOUSER
#define NOKERNEL
#define NOSOUND
#define BLENDFUNCTION void    // for mingw & gcc

#include <windows.h>

BOOL APIENTRY 
DllMain( HANDLE hModule, 
         DWORD  ul_reason_for_call, 
         LPVOID lpReserved )
{
  return TRUE;
}
