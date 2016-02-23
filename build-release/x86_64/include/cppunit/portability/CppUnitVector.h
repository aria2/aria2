#ifndef CPPUNIT_PORTABILITY_CPPUNITVECTOR_H
#define CPPUNIT_PORTABILITY_CPPUNITVECTOR_H

// The technic used is similar to the wrapper of STLPort.
 
#include <cppunit/Portability.h>
#include <vector>


#if CPPUNIT_STD_NEED_ALLOCATOR

template<class T>
class CppUnitVector : public std::vector<T,CPPUNIT_STD_ALLOCATOR>
{
public:
};

#else // CPPUNIT_STD_NEED_ALLOCATOR

#define CppUnitVector std::vector

#endif

#endif // CPPUNIT_PORTABILITY_CPPUNITVECTOR_H

