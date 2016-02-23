#ifndef SUBCLASSEDTESTCASE_H
#define SUBCLASSEDTESTCASE_H

#include "BaseTestCase.h"


class SubclassedTestCase : public BaseTestCase
{
  CPPUNIT_TEST_SUB_SUITE( SubclassedTestCase, BaseTestCase );
  CPPUNIT_TEST( testSubclassing );
  CPPUNIT_TEST_SUITE_END();

public:
  SubclassedTestCase();
  virtual ~SubclassedTestCase();

  virtual void setUp();
  virtual void tearDown();

  // Another test to ensure the subclassed test case are in the suite .
  void testSubclassing();

protected:
  // We overload this method to ensure that the testUsingCheckIt in the
  // parent class will fail.
  virtual void checkIt();

private:
  SubclassedTestCase( const SubclassedTestCase &copy );
  void operator =( const SubclassedTestCase &copy );
};



#endif  // SUBCLASSEDTESTCASE_H
