#ifndef CPPUNIT_ACTIVETEST_H
#define CPPUNIT_ACTIVETEST_H

#include <afxmt.h>

#ifndef CPPUNIT_TESTDECORATOR_H
#include <cppunit/extensions/TestDecorator.h>
#endif


/* A Microsoft-specific active test
 *
 * An active test manages its own
 * thread of execution.  This one 
 * is very simple and only sufficient
 * for the limited use we put it through
 * in the TestRunner.  It spawns a thread
 * on run (TestResult *) and signals
 * completion of the test.
 *
 * We assume that only one thread 
 * will be active at once for each
 * instance.
 *
 */

class ActiveTest : public CPPUNIT_NS::TestDecorator
{
public:
  ActiveTest( CPPUNIT_NS::Test *test );
  ~ActiveTest();

  void run( CPPUNIT_NS::TestResult *result );

protected:
  HANDLE m_threadHandle;
  CEvent m_runCompleted;
  CPPUNIT_NS::TestResult *m_currentTestResult;

  void run();
  void setTestResult( CPPUNIT_NS::TestResult *result );
  static UINT threadFunction( LPVOID thisInstance );
};

#endif


