#ifndef CPPUNIT_EXTENSIONS_TESTFIXTUREFACTORY_H
#define CPPUNIT_EXTENSIONS_TESTFIXTUREFACTORY_H

#include <cppunit/Portability.h>


CPPUNIT_NS_BEGIN


class TestFixture;

/*! \brief Abstract TestFixture factory (Implementation).
 *
 * Implementation detail. Use by HelperMacros to handle TestFixture hierarchy.
 */
class TestFixtureFactory
{
public:
  //! Creates a new TestFixture instance.
  virtual TestFixture *makeFixture() =0;

  virtual ~TestFixtureFactory() {}
};


/*! \brief Concret TestFixture factory (Implementation).
 *
 * Implementation detail. Use by HelperMacros to handle TestFixture hierarchy.
 */
template<class TestFixtureType>
class ConcretTestFixtureFactory : public CPPUNIT_NS::TestFixtureFactory
{
  /*! \brief Returns a new TestFixture instance.
   * \return A new fixture instance. The fixture instance is returned by
   *         the TestFixtureFactory passed on construction. The actual type 
   *         is that of the fixture on which the static method suite() 
   *         was called.
   */
  TestFixture *makeFixture()
  {
    return new TestFixtureType();
  }
};


CPPUNIT_NS_END


#endif // CPPUNIT_EXTENSIONS_TESTFIXTUREFACTORY_H

