#include "SingletonHolder.h"
#include "SharedHandle.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class SingletonHolderTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SingletonHolderTest);
  CPPUNIT_TEST(testInstance);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testInstance();
};


CPPUNIT_TEST_SUITE_REGISTRATION( SingletonHolderTest );

class M {
private:
  std::string _greeting;
public:
  M(const std::string& greeting):_greeting(greeting) {}

  const std::string& greeting() const { return _greeting; }

  void greeting(const std::string& greeting) {
    _greeting = greeting;
  }
};

typedef SharedHandle<M> MHandle;
typedef SharedHandle<int> IntHandle;

void SingletonHolderTest::testInstance()
{
  MHandle m = new M("Hello world.");
  SingletonHolder<MHandle>::instance(m);

  std::cerr << SingletonHolder<MHandle>::instance()->greeting() << std::endl;

  SingletonHolder<MHandle>::instance()->greeting("Yes, it worked!");

  std::cerr << SingletonHolder<MHandle>::instance()->greeting() << std::endl;

  IntHandle i = new int(100);
  SingletonHolder<IntHandle>::instance(i);
  std::cerr << SingletonHolder<IntHandle>::instance() << std::endl;

  std::cerr << SingletonHolder<MHandle>::instance()->greeting() << std::endl;

}

} // namespace aria2
