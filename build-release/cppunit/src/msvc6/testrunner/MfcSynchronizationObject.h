// //////////////////////////////////////////////////////////////////////////
// Header file LightweightSynchronizationObject.h for class LightweightSynchronizationObject
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2002/02/27
// //////////////////////////////////////////////////////////////////////////
#ifndef LIGHTWEIGHTSYNCHRONIZATIONOBJECT_H
#define LIGHTWEIGHTSYNCHRONIZATIONOBJECT_H

#include <cppunit/SynchronizedObject.h>


/*! \class LightweightSynchronizationObject
 * \brief This class represents a lock object for synchronized object.
 */
class MfcSynchronizationObject 
    : public CPPUNIT_NS::SynchronizedObject::SynchronizationObject
{
  CCriticalSection m_syncObject;

public:
    void lock()
    {
      m_syncObject.Lock();
    }
    
    void unlock()
    {
      m_syncObject.Unlock();
    }
};



// Inlines methods for LightweightSynchronizationObject:
// -----------------------------------------------------



#endif  // LIGHTWEIGHTSYNCHRONIZATIONOBJECT_H
