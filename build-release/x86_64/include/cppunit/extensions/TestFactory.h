#ifndef CPPUNIT_EXTENSIONS_TESTFACTORY_H
#define CPPUNIT_EXTENSIONS_TESTFACTORY_H

#include <cppunit/Portability.h>

CPPUNIT_NS_BEGIN


class Test;

/*! \brief Abstract Test factory.
 */
class CPPUNIT_API TestFactory 
{
public:
  virtual ~TestFactory() {}

  /*! Makes a new test.
   * \return A new Test.
   */
  virtual Test* makeTest() = 0;
};


CPPUNIT_NS_END

#endif  // CPPUNIT_EXTENSIONS_TESTFACTORY_H
