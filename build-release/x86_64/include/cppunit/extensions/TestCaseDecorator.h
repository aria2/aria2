#ifndef CPPUNIT_EXTENSIONS_TESTCASEDECORATOR_H
#define CPPUNIT_EXTENSIONS_TESTCASEDECORATOR_H

#include <cppunit/Portability.h>
#include <cppunit/TestCase.h>

CPPUNIT_NS_BEGIN


/*! \brief  Decorator for Test cases.
 *
 * TestCaseDecorator provides an alternate means to extend functionality
 * of a test class without subclassing the test.  Instead, one can
 * subclass the decorater and use it to wrap the test class.
 *
 * Does not assume ownership of the test it decorates
 */ 
class CPPUNIT_API TestCaseDecorator : public TestCase
{
public:
  TestCaseDecorator( TestCase *test );
  ~TestCaseDecorator();

  std::string getName() const;

  void setUp();

  void tearDown();

  void runTest();

protected:
  TestCase *m_test;
};


CPPUNIT_NS_END

#endif

