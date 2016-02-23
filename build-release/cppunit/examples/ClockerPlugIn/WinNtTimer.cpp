// //////////////////////////////////////////////////////////////////////////
// Implementation file WinNtTimer.cpp for class WinNtTimer
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////

#include "WinNtTimer.h"


/*! Returns time spent in the thread.
 * @param rquadTime Receive the time spent in the thread (user+kernel time) 
 *                  in unit of 100 nano-seconds.
 *                  In pratice, the effective resolution is 10ms !!!
 *
 * @return \c true if sucess, \c false otherwise.
 */
static bool 
GetThreadSpentTime( LONGLONG &rquadTime )
{
  FILETIME timeCreation;
  FILETIME timeExit;
  FILETIME timeKernel;
  FILETIME timeUser;
  if ( !::GetThreadTimes( ::GetCurrentThread(),
                          &timeCreation,
                          &timeExit,
                          &timeKernel,
                          &timeUser) )
  {
    rquadTime = 0;
    return false;
  }

  LARGE_INTEGER lintKernel;
  lintKernel.LowPart = timeKernel.dwLowDateTime;
  lintKernel.HighPart = timeKernel.dwHighDateTime;

  LARGE_INTEGER lintUser;
  lintUser.LowPart = timeUser.dwLowDateTime;
  lintUser.HighPart = timeUser.dwHighDateTime;

  rquadTime = lintKernel.QuadPart + lintUser.QuadPart;

  return true;
}



void 
WinNtTimer::start()
{
  m_isValid = GetThreadSpentTime( m_beginTime );

}


void 
WinNtTimer::finish()
{
  LONGLONG quadTimeEnd;
  LONGLONG quadProcessedElapse;
  m_isValid = m_isValid  && GetThreadSpentTime( quadTimeEnd );
  if ( m_isValid )
  {
    quadProcessedElapse = quadTimeEnd - m_beginTime;
    m_elapsedTime = double(quadProcessedElapse) / 10000000;
  }
  else
    m_elapsedTime = -1;
}


double 
WinNtTimer::elapsedTime() const
{
  return m_elapsedTime;
}
