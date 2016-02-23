#ifndef CPPUNIT_PORTABILITY_FLOATINGPOINT_H_INCLUDED
#define CPPUNIT_PORTABILITY_FLOATINGPOINT_H_INCLUDED

#include <cppunit/Portability.h>
#include <math.h>

CPPUNIT_NS_BEGIN

/// \brief Tests if a floating-point is a NaN.
// According to IEEE-754 floating point standard, 
// (see e.g. page 8 of
// http://www.cs.berkeley.edu/~wkahan/ieee754status/ieee754.ps) 
// all comparisons with NaN are false except "x != x", which is true.
//
// At least Microsoft Visual Studio 6 is known not to implement this test correctly.
// It emits the following code to test equality:
//  fcomp       qword ptr [nan]
//  fnstsw      ax                        // copie fp (floating-point) status register to ax
//  test        ah,40h                    // test bit 14 of ax (0x4000) => C3 of fp status register
// According to the following documentation on the x86 floating point status register,
// the C2 bit should be tested to test for NaN value. 
// http://webster.cs.ucr.edu/AoA/Windows/HTML/RealArithmetic.html#1000117
// In Microsoft Visual Studio 2003 & 2005, the test is implemented with:
//  test        ah,44h         // Visual Studio 2005 test both C2 & C3...
//
// To work around this, a NaN is assumed to be detected if no strict ordering is found.
inline bool floatingPointIsUnordered( double x )
{
   // x != x will detect a NaN on conformant platform
   // (2.0 < x  &&  x < 1.0) will detect a NaN on non conformant platform:
   // => no ordering can be found for x.
   return  (x != x) ||  (2.0 < x  &&  x < 1.0);
}


/// \brief Tests if a floating-point is finite.
/// @return \c true if x is neither a NaN, nor +inf, nor -inf, \c false otherwise.
inline int floatingPointIsFinite( double x )
{
#if defined(CPPUNIT_HAVE_ISFINITE)
   return isfinite( x );
#elif defined(CPPUNIT_HAVE_FINITE)
   return finite( x );
#elif defined(CPPUNIT_HAVE__FINITE)
   return _finite(x);
#else
   double testInf = x * 0.0;  // Produce 0.0 if x is finite, a NaN otherwise.
   return testInf == 0.0  &&  !floatingPointIsUnordered(testInf);
#endif
}

CPPUNIT_NS_END

#endif // CPPUNIT_PORTABILITY_FLOATINGPOINT_H_INCLUDED
