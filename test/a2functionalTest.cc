#include "a2functional.h"
#include <string>
#include <numeric>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class a2functionalTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(a2functionalTest);
  CPPUNIT_TEST(testMemFunSh);
  CPPUNIT_TEST(testAdopt2nd);
  CPPUNIT_TEST_SUITE_END();
public:
  void testMemFunSh();
  void testAdopt2nd();

  class Greeting {
  public:
    virtual ~Greeting() {}

    virtual string sayGreeting() = 0;

    virtual string sayGreetingConst() const = 0;
  };

  typedef SharedHandle<Greeting> GreetingHandle;

  class JapaneseGreeting:public Greeting
  {
    virtual string sayGreeting()
    {
      return "HAROO WAARUDO";
    }

    virtual string sayGreetingConst() const
    {
      return "HAROO WAARUDO";
    }

  };

};


CPPUNIT_TEST_SUITE_REGISTRATION(a2functionalTest);

void a2functionalTest::testMemFunSh()
{
  GreetingHandle greeting = new JapaneseGreeting();

  CPPUNIT_ASSERT_EQUAL(string("HAROO WAARUDO"), mem_fun_sh(&Greeting::sayGreeting)(greeting));

  CPPUNIT_ASSERT_EQUAL(string("HAROO WAARUDO"), mem_fun_sh(&Greeting::sayGreetingConst)(greeting));

}

void a2functionalTest::testAdopt2nd()
{
  GreetingHandle greeting = new JapaneseGreeting();

  CPPUNIT_ASSERT_EQUAL(string("A Japanese said:HAROO WAARUDO"),
		       adopt2nd(plus<string>(), mem_fun_sh(&Greeting::sayGreeting))("A Japanese said:", greeting));
}
