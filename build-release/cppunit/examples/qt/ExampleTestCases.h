#ifndef CPP_UNIT_EXAMPLETESTCASES_H
#define CPP_UNIT_EXAMPLETESTCASES_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

/* 
 * A test case that is designed to produce
 * example errors and failures.
 *
 */

class ExampleTestCases : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ExampleTestCases );
  CPPUNIT_TEST( example );
  CPPUNIT_TEST( anotherExample );
  CPPUNIT_TEST( testAdd );
  CPPUNIT_TEST( testEquals );
  CPPUNIT_TEST_SUITE_END();

protected:

	double m_value1;
	double m_value2;
public:

	void setUp ();

protected:

	void example ();
	void anotherExample ();
	void testAdd ();
	void testEquals ();
};

#endif
