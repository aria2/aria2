#include "ExampleTestCases.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ExampleTestCases );

void ExampleTestCases::example ()
{
   CPPUNIT_ASSERT_DOUBLES_EQUAL (1.0, 1.1, 0.05);
   CPPUNIT_ASSERT (1 == 0);
   CPPUNIT_ASSERT (1 == 1);
}

void ExampleTestCases::anotherExample ()
{
   CPPUNIT_ASSERT (1 == 2);
}

void ExampleTestCases::setUp ()
{
   m_value1 = 2.0;
   m_value2 = 3.0;
}

void ExampleTestCases::testAdd ()
{
   double result = m_value1 + m_value2;
   CPPUNIT_ASSERT (result == 6.0);
}

void ExampleTestCases::testEquals ()
{
   std::auto_ptr<long>	l1 (new long (12));
   std::auto_ptr<long>	l2 (new long (12));
   
   CPPUNIT_ASSERT_EQUAL (12, 12);
   CPPUNIT_ASSERT_EQUAL (12L, 12L);
   CPPUNIT_ASSERT_EQUAL (*l1, *l2);
   
   CPPUNIT_ASSERT (12L == 12L);
   CPPUNIT_ASSERT_EQUAL (12, 13);
   CPPUNIT_ASSERT_DOUBLES_EQUAL (12.0, 11.99, 0.5);
}
