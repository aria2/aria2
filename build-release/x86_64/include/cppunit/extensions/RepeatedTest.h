#ifndef CPPUNIT_EXTENSIONS_REPEATEDTEST_H
#define CPPUNIT_EXTENSIONS_REPEATEDTEST_H

#include <cppunit/Portability.h>
#include <cppunit/extensions/TestDecorator.h>

CPPUNIT_NS_BEGIN


class Test;
class TestResult;


/*! \brief Decorator that runs a test repeatedly.
 *
 * Does not assume ownership of the test it decorates
 */
class CPPUNIT_API RepeatedTest : public TestDecorator 
{
public:
  RepeatedTest( Test *test, 
                int timesRepeat ) : 
      TestDecorator( test ), 
      m_timesRepeat(timesRepeat) 
  {
  }

  void run( TestResult *result );

  int countTestCases() const;

private:
  RepeatedTest( const RepeatedTest & );
  void operator=( const RepeatedTest & );

  const int m_timesRepeat;
};


CPPUNIT_NS_END


#endif // CPPUNIT_EXTENSIONS_REPEATEDTEST_H
