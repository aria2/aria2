// //////////////////////////////////////////////////////////////////////////
// Implementation file Timer.cpp for class Timer
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/04/19
// //////////////////////////////////////////////////////////////////////////

#include "Timer.h"


void 
Timer::start()
{
  m_beginTime = clock();
}


void 
Timer::finish()
{
  m_elapsedTime = double(clock() - m_beginTime) / CLOCKS_PER_SEC;
}


double 
Timer::elapsedTime() const
{
  return m_elapsedTime;
}
