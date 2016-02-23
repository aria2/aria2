#ifndef CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H
#define CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251)  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/portability/CppUnitSet.h>
#include <cppunit/extensions/TestFactory.h>
#include <string>

CPPUNIT_NS_BEGIN


class TestSuite;

#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::set<TestFactory *>;
#endif


/*! \brief Registry for TestFactory.
 * \ingroup CreatingTestSuite
 *
 * Notes that the registry \b DON'T assumes lifetime control for any registered tests
 * anymore.
 *
 * The <em>default</em> registry is the registry returned by getRegistry() with the 
 * default name parameter value.
 *
 * To register tests, use the macros:
 * - CPPUNIT_TEST_SUITE_REGISTRATION(): to add tests in the default registry.
 * - CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(): to add tests in a named registry.
 *
 * Example 1: retreiving a suite that contains all the test registered with
 * CPPUNIT_TEST_SUITE_REGISTRATION().
 * \code
 * CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
 * CppUnit::TestSuite *suite = registry.makeTest();
 * \endcode
 *
 * Example 2: retreiving a suite that contains all the test registered with
 * \link CPPUNIT_TEST_SUITE_NAMED_REGISTRATION() CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Math" )\endlink.
 * \code
 * CppUnit::TestFactoryRegistry &mathRegistry = CppUnit::TestFactoryRegistry::getRegistry( "Math" );
 * CppUnit::TestSuite *mathSuite = mathRegistry.makeTest();
 * \endcode
 *
 * Example 3: creating a test suite hierarchy composed of unnamed registration and
 * named registration:
 * - All Tests
 *   - tests registered with CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Graph" )
 *   - tests registered with CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Math" )
 *   - tests registered with CPPUNIT_TEST_SUITE_REGISTRATION
 *
 * \code
 * CppUnit::TestSuite *rootSuite = new CppUnit::TestSuite( "All tests" );
 * rootSuite->addTest( CppUnit::TestFactoryRegistry::getRegistry( "Graph" ).makeTest() );
 * rootSuite->addTest( CppUnit::TestFactoryRegistry::getRegistry( "Math" ).makeTest() );
 * CppUnit::TestFactoryRegistry::getRegistry().addTestToSuite( rootSuite );
 * \endcode
 *
 * The same result can be obtained with:
 * \code
 * CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
 * registry.addRegistry( "Graph" );
 * registry.addRegistry( "Math" );
 * CppUnit::TestSuite *suite = registry.makeTest();
 * \endcode
 *
 * Since a TestFactoryRegistry is a TestFactory, the named registries can be 
 * registered in the unnamed registry, creating the hierarchy links.
 *
 * \see TestSuiteFactory, AutoRegisterSuite
 * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_SUITE_NAMED_REGISTRATION
 */
class CPPUNIT_API TestFactoryRegistry : public TestFactory
{
public:
  /** Constructs the registry with the specified name.
   * \param name Name of the registry. It is the name of TestSuite returned by
   *             makeTest().
   */
  TestFactoryRegistry( std::string name );

  /// Destructor.
  virtual ~TestFactoryRegistry();

  /** Returns a new TestSuite that contains the registered test.
   * \return A new TestSuite which contains all the test added using 
   * registerFactory(TestFactory *).
   */
  virtual Test *makeTest();

  /** Returns a named registry.
   *
   * If the \a name is left to its default value, then the registry that is returned is
   * the one used by CPPUNIT_TEST_SUITE_REGISTRATION(): the 'top' level registry.
   *
   * \param name Name of the registry to return.
   * \return Registry. If the registry does not exist, it is created with the
   *         specified name.
   */
  static TestFactoryRegistry &getRegistry( const std::string &name = "All Tests" );

  /** Adds the registered tests to the specified suite.
   * \param suite Suite the tests are added to.
   */
  void addTestToSuite( TestSuite *suite );

  /** Adds the specified TestFactory to the registry.
   *
   * \param factory Factory to register. 
   */
  void registerFactory( TestFactory *factory );

  /*! Removes the specified TestFactory from the registry.
   * 
   * The specified factory is not destroyed.
   * \param factory Factory to remove from the registry.
   * \todo Address case when trying to remove a TestRegistryFactory.
   */
  void unregisterFactory( TestFactory *factory );

  /*! Adds a registry to the registry.
   * 
   * Convenience method to help create test hierarchy. See TestFactoryRegistry detail
   * for examples of use. Calling this method is equivalent to:
   * \code
   * this->registerFactory( TestFactoryRegistry::getRegistry( name ) );
   * \endcode
   *
   * \param name Name of the registry to add.
   */
  void addRegistry( const std::string &name );

  /*! Tests if the registry is valid.
   *
   * This method should be used when unregistering test factory on static variable 
   * destruction to ensure that the registry has not been already destroyed (in 
   * that case there is no need to unregister the test factory).
   *
   * You should not concern yourself with this method unless you are writing a class
   * like AutoRegisterSuite.
   *
   * \return \c true if the specified registry has not been destroyed, 
   *         otherwise returns \c false.
   * \see AutoRegisterSuite.
   */
  static bool isValid();

  /** Adds the specified TestFactory with a specific name (DEPRECATED).
   * \param name Name associated to the factory.
   * \param factory Factory to register. 
   * \deprecated Use registerFactory( TestFactory *) instead.
   */
  void registerFactory( const std::string &name,
                        TestFactory *factory );

private:
  TestFactoryRegistry( const TestFactoryRegistry &copy );
  void operator =( const TestFactoryRegistry &copy );

private:
  typedef CppUnitSet<TestFactory *, std::less<TestFactory*> > Factories;
  Factories m_factories;

  std::string m_name;
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


#endif  // CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H
