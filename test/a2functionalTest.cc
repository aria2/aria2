#include "a2functional.h"

#include <string>
#include <numeric>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class a2functionalTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(a2functionalTest);
  CPPUNIT_TEST(testMemFunSh);
  CPPUNIT_TEST(testAdopt2nd);
  CPPUNIT_TEST(testStrjoin);
  CPPUNIT_TEST(testLeastRecentAccess);
  CPPUNIT_TEST_SUITE_END();
public:
  void testMemFunSh();
  void testAdopt2nd();
  void testStrjoin();
  void testLeastRecentAccess();

  class Greeting {
  public:
    virtual ~Greeting() {}

    virtual std::string sayGreeting() = 0;

    virtual std::string sayGreetingConst() const = 0;
  };

  typedef SharedHandle<Greeting> GreetingHandle;

  class JapaneseGreeting:public Greeting
  {
    virtual std::string sayGreeting()
    {
      return "HAROO WAARUDO";
    }

    virtual std::string sayGreetingConst() const
    {
      return "HAROO WAARUDO";
    }

  };

  struct LastAccess {
    time_t lastAccess_;
    LastAccess(time_t lastAccess):lastAccess_(lastAccess) {}

    time_t getLastAccessTime() const
    {
      return lastAccess_;
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(a2functionalTest);

void a2functionalTest::testMemFunSh()
{
  GreetingHandle greeting(new JapaneseGreeting());

  CPPUNIT_ASSERT_EQUAL(std::string("HAROO WAARUDO"), mem_fun_sh(&Greeting::sayGreeting)(greeting));

  CPPUNIT_ASSERT_EQUAL(std::string("HAROO WAARUDO"), mem_fun_sh(&Greeting::sayGreetingConst)(greeting));

}

void a2functionalTest::testAdopt2nd()
{
  GreetingHandle greeting(new JapaneseGreeting());

  CPPUNIT_ASSERT_EQUAL(std::string("A Japanese said:HAROO WAARUDO"),
                       adopt2nd(std::plus<std::string>(), mem_fun_sh(&Greeting::sayGreeting))("A Japanese said:", greeting));
}

void a2functionalTest::testStrjoin()
{
  std::vector<std::string> v;
  CPPUNIT_ASSERT_EQUAL(std::string(""), strjoin(v.begin(), v.end(), " "));

  v.push_back("A");

  CPPUNIT_ASSERT_EQUAL(std::string("A"), strjoin(v.begin(), v.end(), " "));

  v.push_back("hero");
  v.push_back("is");
  v.push_back("lonely");

  CPPUNIT_ASSERT_EQUAL(std::string("A hero is lonely"),
                       strjoin(v.begin(), v.end(), " "));
}

void a2functionalTest::testLeastRecentAccess()
{
  std::vector<LastAccess> v;
  for(int i = 99; i >= 0; --i) {
    v.push_back(LastAccess(i));
  }
  std::sort(v.begin(), v.end(), LeastRecentAccess<LastAccess>());
  for(int i = 0; i < 100; ++i) {
    CPPUNIT_ASSERT_EQUAL((time_t)i, v[i].lastAccess_);
  }
}

} // namespace aria2
