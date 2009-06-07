#include "a2functional.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <numeric>

namespace aria2 {

class a2functionalTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(a2functionalTest);
  CPPUNIT_TEST(testMemFunSh);
  CPPUNIT_TEST(testAdopt2nd);
  CPPUNIT_TEST(testStrjoin);
  CPPUNIT_TEST(testStrconcat);
  CPPUNIT_TEST(testStrappend);
  CPPUNIT_TEST_SUITE_END();
public:
  void testMemFunSh();
  void testAdopt2nd();
  void testStrjoin();
  void testStrconcat();
  void testStrappend();
  
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

void a2functionalTest::testStrconcat()
{
  CPPUNIT_ASSERT_EQUAL(std::string("X=3"), strconcat("X=", "3"));
}

void a2functionalTest::testStrappend()
{
  std::string str = "X=";
  strappend(str, "3", ",Y=", "5");
  CPPUNIT_ASSERT_EQUAL(std::string("X=3,Y=5"), str);
}

} // namespace aria2
