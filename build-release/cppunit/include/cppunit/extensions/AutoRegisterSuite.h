#ifndef CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H
#define CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H

#include <cppunit/extensions/TestSuiteFactory.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <string>

CPPUNIT_NS_BEGIN


/*! \brief (Implementation) Automatically register the test suite of the specified type.
 *
 * You should not use this class directly. Instead, use the following macros:
 * - CPPUNIT_TEST_SUITE_REGISTRATION()
 * - CPPUNIT_TEST_SUITE_NAMED_REGISTRATION()
 *
 * This object will register the test returned by TestCaseType::suite()
 * when constructed to the test registry.
 *
 * This object is intented to be used as a static variable.
 *
 *
 * \param TestCaseType Type of the test case which suite is registered.
 * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_SUITE_NAMED_REGISTRATION
 * \see CppUnit::TestFactoryRegistry.
 */
template<class TestCaseType>
class AutoRegisterSuite
{
public:
  /** Auto-register the suite factory in the global registry.
   */
  AutoRegisterSuite()
      : m_registry( &TestFactoryRegistry::getRegistry() )
  {
    m_registry->registerFactory( &m_factory );
  }

  /** Auto-register the suite factory in the specified registry.
   * \param name Name of the registry.
   */
  AutoRegisterSuite( const std::string &name )
      : m_registry( &TestFactoryRegistry::getRegistry( name ) )
  {
    m_registry->registerFactory( &m_factory );
  }

  ~AutoRegisterSuite()
  {
    if ( TestFactoryRegistry::isValid() )
      m_registry->unregisterFactory( &m_factory );
  }

private:
  TestFactoryRegistry *m_registry;
  TestSuiteFactory<TestCaseType> m_factory;
};


/*! \brief (Implementation) Automatically adds a registry into another registry.
 *
 * Don't use this class. Use the macros CPPUNIT_REGISTRY_ADD() and
 * CPPUNIT_REGISTRY_ADD_TO_DEFAULT() instead.
 */
class AutoRegisterRegistry
{
public:
  AutoRegisterRegistry( const std::string &which,
                        const std::string &to )
  {
    TestFactoryRegistry::getRegistry( to ).addRegistry( which );
  }

  AutoRegisterRegistry( const std::string &which )
  {
    TestFactoryRegistry::getRegistry().addRegistry( which );
  }
};


CPPUNIT_NS_END

#endif  // CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H
