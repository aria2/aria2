#include <cppunit/config/SourcePrefix.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/portability/CppUnitMap.h>
#include <cppunit/TestSuite.h>
#include <assert.h>


CPPUNIT_NS_BEGIN

/*! \brief (INTERNAL) List of all TestFactoryRegistry.
 */
class TestFactoryRegistryList
{
private:
  typedef CppUnitMap<std::string, TestFactoryRegistry *, std::less<std::string> > Registries;
  Registries m_registries;

  enum State {
    doNotChange =0,
    notCreated,
    exist,
    destroyed
  };

  static State stateFlag( State newState = doNotChange )
  {
    static State state = notCreated;
    if ( newState != doNotChange )
      state = newState;
    return state;
  }

  static TestFactoryRegistryList *getInstance()
  {
    static TestFactoryRegistryList list;
    return &list;
  }

  TestFactoryRegistry *getInternalRegistry( const std::string &name )
  {
    Registries::const_iterator foundIt = m_registries.find( name );
    if ( foundIt == m_registries.end() )
    {
      TestFactoryRegistry *factory = new TestFactoryRegistry( name );
      m_registries.insert( std::pair<const std::string, TestFactoryRegistry*>( name, factory ) );
      return factory;
    }
    return (*foundIt).second;
  }

public:
  TestFactoryRegistryList()
  {
    stateFlag( exist );
  }

  ~TestFactoryRegistryList()
  {
    for ( Registries::iterator it = m_registries.begin(); it != m_registries.end(); ++it )
      delete (*it).second;

    stateFlag( destroyed );
  }

  static TestFactoryRegistry *getRegistry( const std::string &name )
  {
    // If the following assertion failed, then TestFactoryRegistry::getRegistry() 
    // was called during static variable destruction without checking the registry 
    // validity beforehand using TestFactoryRegistry::isValid() beforehand.
    assert( isValid() );
    if ( !isValid() )         // release mode
      return NULL;            // => force CRASH

    return getInstance()->getInternalRegistry( name );
  }

  static bool isValid()
  {
    return stateFlag() != destroyed;
  }
};



TestFactoryRegistry::TestFactoryRegistry( std::string name ) :
    m_name( name )
{
}


TestFactoryRegistry::~TestFactoryRegistry()
{
}


TestFactoryRegistry &
TestFactoryRegistry::getRegistry( const std::string &name )
{
  return *TestFactoryRegistryList::getRegistry( name );
}


void 
TestFactoryRegistry::registerFactory( const std::string &name,
                                      TestFactory *factory )
{
  registerFactory( factory );
}


void 
TestFactoryRegistry::registerFactory( TestFactory *factory )
{
  m_factories.insert( factory );
}


void 
TestFactoryRegistry::unregisterFactory( TestFactory *factory )
{
  m_factories.erase( factory );
}


void 
TestFactoryRegistry::addRegistry( const std::string &name )
{
  registerFactory( &getRegistry( name ) );
}


Test *
TestFactoryRegistry::makeTest()
{
  TestSuite *suite = new TestSuite( m_name );
  addTestToSuite( suite );
  return suite;
}


void 
TestFactoryRegistry::addTestToSuite( TestSuite *suite )
{
  for ( Factories::iterator it = m_factories.begin(); 
        it != m_factories.end(); 
        ++it )
  {
    TestFactory *factory = *it;
    suite->addTest( factory->makeTest() );
  }
}


bool 
TestFactoryRegistry::isValid()
{
  return TestFactoryRegistryList::isValid();
}


CPPUNIT_NS_END
