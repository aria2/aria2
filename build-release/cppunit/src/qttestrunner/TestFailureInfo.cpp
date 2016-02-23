// //////////////////////////////////////////////////////////////////////////
// Implementation file TestFailureInfo.cpp for class TestFailureInfo
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////

#include "TestFailureInfo.h"
#include <cppunit/Exception.h>


TestFailureInfo::TestFailureInfo( CPPUNIT_NS::Test *failedTest, 
                                  CPPUNIT_NS::Exception *thrownException,
                                  bool isError ) : 
    CPPUNIT_NS::TestFailure( failedTest, thrownException->clone(), isError )
{
}


TestFailureInfo::~TestFailureInfo()
{
}
