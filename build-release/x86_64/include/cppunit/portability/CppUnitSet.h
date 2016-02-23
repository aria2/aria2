#ifndef CPPUNIT_PORTABILITY_CPPUNITSET_H
#define CPPUNIT_PORTABILITY_CPPUNITSET_H

// The technic used is similar to the wrapper of STLPort.
 
#include <cppunit/Portability.h>
#include <functional>
#include <set>


#if CPPUNIT_STD_NEED_ALLOCATOR

template<class T>
class CppUnitSet : public std::set<T
                                  ,std::less<T>
                                  ,CPPUNIT_STD_ALLOCATOR>
{
public:
};

#else // CPPUNIT_STD_NEED_ALLOCATOR

#define CppUnitSet std::set

#endif

#endif // CPPUNIT_PORTABILITY_CPPUNITSET_H

