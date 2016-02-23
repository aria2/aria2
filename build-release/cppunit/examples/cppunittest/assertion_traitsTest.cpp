#include <cppunit/TestAssert.h>
#include "CoreSuite.h"
#include "assertion_traitsTest.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( assertion_traitsTest,
                                       coreSuiteName() );

assertion_traitsTest::assertion_traitsTest()
{
}


void
assertion_traitsTest::test_toString()
{
    CPPUNIT_ASSERT_EQUAL( std::string( "abc" ), 
			  CPPUNIT_NS::assertion_traits<char*>::toString( "abc" ) );

    CPPUNIT_ASSERT_EQUAL( std::string( "33" ), 
			  CPPUNIT_NS::assertion_traits<int>::toString( 33 ) );

    // Test that assertion_traits<double>::toString() produces 
    // more than the standard 6 digits of precision.
    CPPUNIT_ASSERT_EQUAL( std::string( "33.1" ), 
			  CPPUNIT_NS::assertion_traits<double>::toString( 33.1 ) );
    CPPUNIT_ASSERT_EQUAL( std::string( "33.001" ), 
			  CPPUNIT_NS::assertion_traits<double>::toString( 33.001 ) );
    CPPUNIT_ASSERT_EQUAL( std::string( "33.00001" ), 
			  CPPUNIT_NS::assertion_traits<double>::toString( 33.00001 ) );
    CPPUNIT_ASSERT_EQUAL( std::string( "33.0000001" ), 
			  CPPUNIT_NS::assertion_traits<double>::toString( 33.0000001 ) );
    CPPUNIT_ASSERT_EQUAL( std::string( "33.0000000001" ), 
			  CPPUNIT_NS::assertion_traits<double>::toString( 33.0000000001 ) );
}
