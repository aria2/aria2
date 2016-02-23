#include "stdafx.h"
#include "ActiveTest.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// Construct the active test
ActiveTest::ActiveTest( CPPUNIT_NS::Test *test )
    : TestDecorator( test )
    , m_runCompleted() 
{ 
  m_currentTestResult = NULL; 
  m_threadHandle = INVALID_HANDLE_VALUE; 
}


// Pend until the test has completed
ActiveTest::~ActiveTest()
{ 
  CSingleLock( &m_runCompleted, TRUE );
  m_test = NULL;
}


// Set the test result that we are to run
void 
ActiveTest::setTestResult( CPPUNIT_NS::TestResult *result )
{ 
  m_currentTestResult = result; 
}


// Run our test result
void 
ActiveTest::run()
{ 
  TestDecorator::run( m_currentTestResult );
}


// Spawn a thread to a test
void 
ActiveTest::run( CPPUNIT_NS::TestResult *result )
{
  CWinThread *thread;
  
  setTestResult( result );
  m_runCompleted.ResetEvent();

  thread = ::AfxBeginThread( threadFunction, 
                             this, 
                             THREAD_PRIORITY_NORMAL, 
                             0, 
                             CREATE_SUSPENDED);
  
  ::DuplicateHandle( GetCurrentProcess(), 
                     thread->m_hThread,
                     GetCurrentProcess(), 
                     &m_threadHandle, 
                     0, 
                     FALSE, 
                     DUPLICATE_SAME_ACCESS );

  thread->ResumeThread ();
}


// Simple execution thread.  Assuming that an ActiveTest instance
// only creates one of these at a time.
UINT 
ActiveTest::threadFunction( LPVOID thisInstance )
{
  ActiveTest *test = (ActiveTest *)thisInstance;

  test->run ();

  ::CloseHandle( test->m_threadHandle );
  test->m_threadHandle = INVALID_HANDLE_VALUE;

  test->m_runCompleted.SetEvent();

  return 0;
}

