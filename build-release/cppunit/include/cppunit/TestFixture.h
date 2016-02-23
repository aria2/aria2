#ifndef CPPUNIT_TESTFIXTURE_H    // -*- C++ -*-
#define CPPUNIT_TESTFIXTURE_H

#include <cppunit/Portability.h>

CPPUNIT_NS_BEGIN


/*! \brief Wraps a test case with setUp and tearDown methods.
 * \ingroup WritingTestFixture
 *
 * A TestFixture is used to provide a common environment for a set
 * of test cases.
 *
 * To define a test fixture, do the following:
 * - implement a subclass of TestCase 
 * - the fixture is defined by instance variables 
 * - initialize the fixture state by overriding setUp
 *   (i.e. construct the instance variables of the fixture)
 * - clean-up after a test by overriding tearDown.
 *
 * Each test runs in its own fixture so there
 * can be no side effects among test runs.
 * Here is an example:
 * 
 * \code
 * class MathTest : public CppUnit::TestFixture {
 * protected:
 *   int m_value1, m_value2;
 *
 * public:
 *   MathTest() {}
 *
 *   void setUp () {
 *     m_value1 = 2;
 *     m_value2 = 3;
 *   }
 * }
 * \endcode
 *
 * For each test implement a method which interacts
 * with the fixture. Verify the expected results with assertions specified
 * by calling CPPUNIT_ASSERT on the expression you want to test:
 * 
 * \code
 * public: 
 *   void testAdd () {
 *     int result = m_value1 + m_value2;
 *     CPPUNIT_ASSERT( result == 5 );
 *   }
 * \endcode
 * 
 * Once the methods are defined you can run them. To do this, use
 * a TestCaller.
 *
 * \code
 * CppUnit::Test *test = new CppUnit::TestCaller<MathTest>( "testAdd", 
 *                                                          &MathTest::testAdd );
 * test->run();
 * \endcode
 *
 *
 * The tests to be run can be collected into a TestSuite. 
 * 
 * \code
 * public: 
 *   static CppUnit::TestSuite *MathTest::suite () {
 *      CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite;
 *      suiteOfTests->addTest(new CppUnit::TestCaller<MathTest>(
 *                              "testAdd", &MathTest::testAdd));
 *      suiteOfTests->addTest(new CppUnit::TestCaller<MathTest>(
 *                              "testDivideByZero", &MathTest::testDivideByZero));
 *      return suiteOfTests;
 *  }
 * \endcode
 * 
 * A set of macros have been created for convenience. They are located in HelperMacros.h.
 *
 * \see TestResult, TestSuite, TestCaller,
 * \see CPPUNIT_TEST_SUB_SUITE, CPPUNIT_TEST, CPPUNIT_TEST_SUITE_END, 
 * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_EXCEPTION, CPPUNIT_TEST_FAIL.
 */
class CPPUNIT_API TestFixture
{
public:
  virtual ~TestFixture() {};

  //! \brief Set up context before running a test.
  virtual void setUp() {};

  //! Clean up after the test run.
  virtual void tearDown() {};
};


CPPUNIT_NS_END


#endif
