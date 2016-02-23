#ifndef MOCKTESTCASE_H
#define MOCKTESTCASE_H

#include <cppunit/TestCase.h>


/*! \class MockTestCase
 * \brief This class represents a mock test case.
 */
class MockTestCase : public CPPUNIT_NS::TestCase
{
public:
  typedef CPPUNIT_NS::TestCase SuperClass;   // work around VC++ call to super class method.

  /*! Constructs a MockTestCase object.
   */
  MockTestCase( std::string name );

  /// Destructor.
  virtual ~MockTestCase();

  void setExpectedSetUpCall( int callCount = 1 );
  void setExpectedTearDownCall( int callCount = 1 );
  void setExpectedRunTestCall( int callCount = 1 );
  void setExpectedCountTestCasesCall( int callCount = 1 );
  
  void makeSetUpThrow();
  void makeTearDownThrow();
  void makeRunTestThrow();
  void makeFindTestPathPassFor( const CPPUNIT_NS::Test *testFound );
  
  void verify();

protected:
  int countTestCases() const;
  void setUp();
  void tearDown();
  void runTest();
//  bool findTestPath( const CPPUNIT_NS::Test *test,
//                     CPPUNIT_NS::TestPath &testPath );

private:
  /// Prevents the use of the copy constructor.
  MockTestCase( const MockTestCase &copy );

  /// Prevents the use of the copy operator.
  void operator =( const MockTestCase &copy );

private:
  bool m_hasSetUpExpectation;
  int m_expectedSetUpCall;
  int m_actualSetUpCall;

  bool m_hasTearDownExpectation;
  int m_expectedTearDownCall;
  int m_actualTearDownCall;

  bool m_expectRunTestCall;
  int m_expectedRunTestCallCount;
  int m_actualRunTestCallCount;
  bool m_expectCountTestCasesCall;
  int m_expectedCountTestCasesCallCount;
  int m_actualCountTestCasesCallCount;

  bool m_setUpThrow;
  bool m_tearDownThrow;
  bool m_runTestThrow;
  const CPPUNIT_NS::Test *m_passingTest;
};





#endif  // MOCKTESTCASE_H
