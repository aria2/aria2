#ifndef CPPUNIT_EXTENSIONS_TESTDECORATOR_H
#define CPPUNIT_EXTENSIONS_TESTDECORATOR_H

#include <cppunit/Portability.h>
#include <cppunit/Test.h>

CPPUNIT_NS_BEGIN


class TestResult;


/*! \brief  Decorator for Tests.
 *
 * TestDecorator provides an alternate means to extend functionality
 * of a test class without subclassing the test.  Instead, one can
 * subclass the decorater and use it to wrap the test class.
 *
 * Does not assume ownership of the test it decorates
 */ 
class CPPUNIT_API TestDecorator : public Test
{
public:
  TestDecorator( Test *test );
  ~TestDecorator();

  int countTestCases() const;

  std::string getName() const;

  void run( TestResult *result );

  int getChildTestCount() const;

protected:
  Test *doGetChildTestAt( int index ) const;

  Test *m_test;

private:
  TestDecorator( const TestDecorator &);
  void operator =( const TestDecorator & );
};


CPPUNIT_NS_END

#endif

