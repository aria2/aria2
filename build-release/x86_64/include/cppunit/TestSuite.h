#ifndef CPPUNIT_TESTSUITE_H    // -*- C++ -*-
#define CPPUNIT_TESTSUITE_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/TestComposite.h>
#include <cppunit/portability/CppUnitVector.h>

CPPUNIT_NS_BEGIN


#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::vector<Test *>;
#endif


/*! \brief A Composite of Tests.
 * \ingroup CreatingTestSuite
 *
 * It runs a collection of test cases. Here is an example.
 * \code
 * CppUnit::TestSuite *suite= new CppUnit::TestSuite();
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testAdd", testAdd));
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testDivideByZero", testDivideByZero));
 * \endcode
 * Note that \link TestSuite TestSuites \endlink assume lifetime
 * control for any tests added to them.
 *
 * TestSuites do not register themselves in the TestRegistry.
 * \see Test 
 * \see TestCaller
 */
class CPPUNIT_API TestSuite : public TestComposite
{
public:
  /*! Constructs a test suite with the specified name.
   */
  TestSuite( std::string name = "" );

  ~TestSuite();

  /*! Adds the specified test to the suite.
   * \param test Test to add. Must not be \c NULL.
    */
  void addTest( Test *test );

  /*! Returns the list of the tests (DEPRECATED).
   * \deprecated Use getChildTestCount() & getChildTestAt() of the 
   *             TestComposite interface instead.
   * \return Reference on a vector that contains the tests of the suite.
   */
  const CppUnitVector<Test *> &getTests() const;

  /*! Destroys all the tests of the suite.
   */
  virtual void deleteContents();

  int getChildTestCount() const;

  Test *doGetChildTestAt( int index ) const;

private:
  CppUnitVector<Test *> m_tests;
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_TESTSUITE_H
