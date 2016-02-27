#include "SingletonHolder.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "a2functional.h"

namespace aria2 {

class SingletonHolderTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SingletonHolderTest);
  CPPUNIT_TEST(testInstance);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testInstance();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SingletonHolderTest);

class M {
private:
  std::string greeting_;

public:
  M(const std::string& greeting) : greeting_(greeting) {}

  const std::string& greeting() const { return greeting_; }

  void greeting(const std::string& greeting) { greeting_ = greeting; }
};

void SingletonHolderTest::testInstance()
{
  SingletonHolder<M>::instance(make_unique<M>("Hello world."));
  CPPUNIT_ASSERT_EQUAL(std::string("Hello world."),
                       SingletonHolder<M>::instance()->greeting());

  SingletonHolder<M>::instance()->greeting("Yes, it worked!");
  CPPUNIT_ASSERT_EQUAL(std::string("Yes, it worked!"),
                       SingletonHolder<M>::instance()->greeting());
}

} // namespace aria2
