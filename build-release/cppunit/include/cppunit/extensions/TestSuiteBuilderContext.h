#ifndef CPPUNIT_HELPER_TESTSUITEBUILDERCONTEXT_H
#define CPPUNIT_HELPER_TESTSUITEBUILDERCONTEXT_H

#include <cppunit/Portability.h>
#include <cppunit/portability/CppUnitMap.h>
#include <string>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif


CPPUNIT_NS_BEGIN

class TestSuite;
class TestFixture;
class TestFixtureFactory;
class TestNamer;

/*! \brief Context used when creating test suite in HelperMacros.
 *
 * Base class for all context used when creating test suite. The
 * actual context type during test suite creation is TestSuiteBuilderContext.
 *
 * \sa CPPUNIT_TEST_SUITE, CPPUNIT_TEST_SUITE_ADD_TEST, 
 *     CPPUNIT_TEST_SUITE_ADD_CUSTOM_TESTS.
 */
class CPPUNIT_API TestSuiteBuilderContextBase
{
public:
  /*! \brief Constructs a new context.
   *
   * You should not use this. The context is created in 
   * CPPUNIT_TEST_SUITE().
   */
  TestSuiteBuilderContextBase( TestSuite &suite,
                               const TestNamer &namer,
                               TestFixtureFactory &factory );

  virtual ~TestSuiteBuilderContextBase();

  /*! \brief Adds a test to the fixture suite.
   *
   * \param test Test to add to the fixture suite. Must not be \c NULL.
   */
  void addTest( Test *test );

  /*! \brief Returns the fixture name.
   * \return Fixture name. It is the name used to name the fixture
   *         suite.
   */
  std::string getFixtureName() const;

  /*! \brief Returns the name of the test for the specified method.
   *
   * \param testMethodName Name of the method that implements a test.
   * \return A string that is the concatenation of the test fixture name 
   *         (returned by getFixtureName()) and\a testMethodName, 
   *         separated using '::'. This provides a fairly unique name for a given
   *         test.
   */
  std::string getTestNameFor( const std::string &testMethodName ) const;

  /*! \brief Adds property pair.
   * \param key   PropertyKey string to add.
   * \param value PropertyValue string to add.
   */
  void addProperty( const std::string &key, 
                    const std::string &value );
  
  /*! \brief Returns property value assigned to param key.
   * \param key PropertyKey string.
   */
  const std::string getStringProperty( const std::string &key ) const;

protected:
  TestFixture *makeTestFixture() const;

  // Notes: we use a vector here instead of a map to work-around the
  // shared std::map in dll bug in VC6.
  // See http://www.dinkumware.com/vc_fixes.html for detail.
  typedef std::pair<std::string,std::string> Property;
  typedef CppUnitVector<Property> Properties;

  TestSuite &m_suite;
  const TestNamer &m_namer;
  TestFixtureFactory &m_factory;

private:
  Properties m_properties;
};


/*! \brief Type-sage context used when creating test suite in HelperMacros.
 * 
 * \sa TestSuiteBuilderContextBase.
 */
template<class Fixture>
class TestSuiteBuilderContext : public TestSuiteBuilderContextBase
{
public:
  typedef Fixture FixtureType;

  TestSuiteBuilderContext( TestSuiteBuilderContextBase &contextBase )
      : TestSuiteBuilderContextBase( contextBase )
  {
  }

  /*! \brief Returns a new TestFixture instance.
   * \return A new fixture instance. The fixture instance is returned by
   *         the TestFixtureFactory passed on construction. The actual type 
   *         is that of the fixture on which the static method suite() 
   *         was called.
   */
  FixtureType *makeFixture() const
  {
    return CPPUNIT_STATIC_CAST( FixtureType *, 
                                TestSuiteBuilderContextBase::makeTestFixture() );
  }
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // CPPUNIT_HELPER_TESTSUITEBUILDERCONTEXT_H

