#include <cppunit/config/SourcePrefix.h>
#include "SubclassedTestCase.h"


SubclassedTestCase::SubclassedTestCase()
{
}


SubclassedTestCase::~SubclassedTestCase()
{
}


void 
SubclassedTestCase::setUp()
{
}


void 
SubclassedTestCase::tearDown()
{
}


void 
SubclassedTestCase::checkIt()
{
  CPPUNIT_ASSERT( false );
}


void 
SubclassedTestCase::testSubclassing()
{
}
