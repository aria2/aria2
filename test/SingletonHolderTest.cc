#include "SingletonHolder.h"
#include "SharedHandle.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

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
  string _greeting;
public:
  M(const string& greeting):_greeting(greeting) {}

  const string& greeting() const { return _greeting; }

  void greeting(const string& greeting) {
    _greeting = greeting;
  }
};

typedef SharedHandle<M> MHandle;
typedef SharedHandle<int> IntHandle;

void SingletonHolderTest::testInstance()
{
  MHandle m = new M("Hello world.");
  SingletonHolder<MHandle>::instance(m);

  cerr << SingletonHolder<MHandle>::instance()->greeting() << endl;

  SingletonHolder<MHandle>::instance()->greeting("Yes, it worked!");

  cerr << SingletonHolder<MHandle>::instance()->greeting() << endl;

  IntHandle i = new int(100);
  SingletonHolder<IntHandle>::instance(i);
  cerr << SingletonHolder<IntHandle>::instance() << endl;

  cerr << SingletonHolder<MHandle>::instance()->greeting() << endl;

}
