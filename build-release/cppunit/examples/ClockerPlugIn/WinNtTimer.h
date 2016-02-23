// //////////////////////////////////////////////////////////////////////////
// Header file WinNtTimer.h for class WinNtTimer
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////
#ifndef WINNTTIMER_H
#define WINNTTIMER_H

#include <windows.h>
#include <winnt.h>
#include <winbase.h>


/// A Timer.
class WinNtTimer
{
public:
  void start();
  void finish();

  double elapsedTime() const;

private:
  LONGLONG m_beginTime;
  double m_elapsedTime;
  bool m_isValid;
};



// Inlines methods for Timer:
// --------------------------



#endif  // WINNTTIMER_H
