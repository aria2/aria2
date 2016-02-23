#ifndef CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H
#define CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H

#include <cppunit/extensions/TestFactory.h>

CPPUNIT_NS_BEGIN


  class Test;

  /*! \brief TestFactory for TestFixture that implements a static suite() method.
   * \see AutoRegisterSuite.
   */
  template<class TestCaseType>
  class TestSuiteFactory : public TestFactory
  {
  public:
    virtual Test *makeTest()
    {
      return TestCaseType::suite();
    }
  };


CPPUNIT_NS_END

#endif  // CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H
