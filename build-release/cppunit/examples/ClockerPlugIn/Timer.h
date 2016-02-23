// //////////////////////////////////////////////////////////////////////////
// Header file Timer.h for class Timer
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////
#ifndef TIMER_H
#define TIMER_H

#include <time.h>

/// A Timer.
class Timer
{
public:
  void start();
  void finish();

  double elapsedTime() const;

private:
  clock_t m_beginTime;
  double m_elapsedTime;
};



// Inlines methods for Timer:
// --------------------------



#endif  // TIMER_H
