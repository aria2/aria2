#ifndef EXCEPTIONTEST_H
#define EXCEPTIONTEST_H

#include <cppunit/extensions/HelperMacros.h>


class ExceptionTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( ExceptionTest );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testDefaultConstructor );
  CPPUNIT_TEST( testCopyConstructor );
  CPPUNIT_TEST( testAssignment );
  CPPUNIT_TEST( testClone );
  CPPUNIT_TEST_SUITE_END();

public:
  ExceptionTest();
  virtual ~ExceptionTest();

  virtual void setUp();
  virtual void tearDown();

  void testConstructor();
  void testDefaultConstructor();
  void testCopyConstructor();
  void testAssignment();
  void testClone();

private:
  ExceptionTest( const ExceptionTest &copy );
  void operator =( const ExceptionTest &copy );
  void checkIsSame( CPPUNIT_NS::Exception &e, 
                    CPPUNIT_NS::Exception &other );

private:
};



#endif  // EXCEPTIONTEST_H
