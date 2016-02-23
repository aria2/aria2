#include "TrackedTestCase.h"

Tracker *TrackedTestCase::ms_tracker = NULL;

TrackedTestCase::TrackedTestCase()
: CPPUNIT_NS::TestCase( "" )
{
  if ( ms_tracker != NULL )
    ms_tracker->onConstructor();
}


TrackedTestCase::~TrackedTestCase()
{
  if ( ms_tracker != NULL )
    ms_tracker->onDestructor();
}


void 
TrackedTestCase::setUp()
{
  if ( ms_tracker != NULL )
    ms_tracker->onSetUp();
}


void 
TrackedTestCase::tearDown()
{
  if ( ms_tracker != NULL )
    ms_tracker->onTearDown();
}


void 
TrackedTestCase::test()
{
  if ( ms_tracker != NULL )
    ms_tracker->onTest();
}


void 
TrackedTestCase::setTracker( Tracker *tracker )
{
  ms_tracker = tracker;
}


void 
TrackedTestCase::removeTracker()
{
  ms_tracker = NULL;
}
