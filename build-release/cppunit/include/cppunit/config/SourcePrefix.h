#ifndef CPPUNIT_CONFIG_H_INCLUDED
#define CPPUNIT_CONFIG_H_INCLUDED

#include <cppunit/Portability.h>

#ifdef _MSC_VER
#pragma warning(disable: 4018 4284 4146)
#if _MSC_VER >= 1400
#pragma warning(disable: 4996)		// sprintf is deprecated
#endif
#endif


#endif // CPPUNIT_CONFIG_H_INCLUDED
