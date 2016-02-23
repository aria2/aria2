#ifndef ASSERTIONTRAITSTEST_H
#define ASSERTIONTRAITSTEST_H

#include <cppunit/extensions/HelperMacros.h>

class assertion_traitsTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( assertion_traitsTest );
  CPPUNIT_TEST( test_toString );
  CPPUNIT_TEST_SUITE_END();

public:

  assertion_traitsTest();

  void test_toString();


private:
  assertion_traitsTest( const assertion_traitsTest &copy );
  void operator =( const assertion_traitsTest &copy );

private:
};

#endif

