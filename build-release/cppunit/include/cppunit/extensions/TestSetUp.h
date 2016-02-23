#ifndef CPPUNIT_EXTENSIONS_TESTSETUP_H
#define CPPUNIT_EXTENSIONS_TESTSETUP_H

#include <cppunit/extensions/TestDecorator.h>

CPPUNIT_NS_BEGIN


class Test;
class TestResult;

/*! \brief Decorates a test by providing a specific setUp() and tearDown().
 */
class CPPUNIT_API TestSetUp : public TestDecorator 
{
public:
  TestSetUp( Test *test );

  void run( TestResult *result );

protected:
  virtual void setUp();
  virtual void tearDown();

private:
  TestSetUp( const TestSetUp & );
  void operator =( const TestSetUp & );
};


CPPUNIT_NS_END

#endif // CPPUNIT_EXTENSIONS_TESTSETUP_H

