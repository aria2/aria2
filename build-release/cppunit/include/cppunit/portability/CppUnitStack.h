#ifndef CPPUNIT_PORTABILITY_CPPUNITSTACK_H
#define CPPUNIT_PORTABILITY_CPPUNITSTACK_H

// The technic used is similar to the wrapper of STLPort.
 
#include <cppunit/Portability.h>
#include <deque>
#include <stack>


#if CPPUNIT_STD_NEED_ALLOCATOR

template<class T>
class CppUnitStack : public std::stack<T
                                      ,std::deque<T,CPPUNIT_STD_ALLOCATOR> >
{
public:
};

#else // CPPUNIT_STD_NEED_ALLOCATOR

#define CppUnitStack std::stack

#endif

#endif // CPPUNIT_PORTABILITY_CPPUNITSTACK_H