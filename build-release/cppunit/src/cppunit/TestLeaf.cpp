#include <cppunit/TestLeaf.h>


CPPUNIT_NS_BEGIN


int 
TestLeaf::countTestCases() const
{
  return 1;
}


int 
TestLeaf::getChildTestCount() const
{
  return 0;
}


Test *
TestLeaf::doGetChildTestAt( int index ) const
{
  checkIsValidIndex( index );
  return NULL;    // never called, checkIsValidIndex() always throw.
}

CPPUNIT_NS_END
