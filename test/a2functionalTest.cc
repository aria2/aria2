#include "a2functional.h"
#include <string>
#include <numeric>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

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

} // namespace aria2
