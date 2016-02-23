#ifndef CPPUNIT_TEXTTESTRESULT_H
#define CPPUNIT_TEXTTESTRESULT_H

#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/portability/Stream.h>

CPPUNIT_NS_BEGIN


class SourceLine;
class Exception;
class Test;

/*! \brief Holds printable test result (DEPRECATED).
 * \ingroup TrackingTestExecution
 * 
 * deprecated Use class TextTestProgressListener and TextOutputter instead.
 */
class CPPUNIT_API TextTestResult : public TestResult,
                                   public TestResultCollector
{
public:
  TextTestResult();

  virtual void addFailure( const TestFailure &failure );
  virtual void startTest( Test *test );
  virtual void print( OStream &stream );
};

/** insertion operator for easy output */
CPPUNIT_API OStream &operator <<( OStream &stream, 
                                  TextTestResult &result );

CPPUNIT_NS_END

#endif // CPPUNIT_TEXTTESTRESULT_H


