#include "CookieBoxFactory.h"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(int argc, char* argv[]) {
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  
  // setup
  CookieBoxFactoryHandle cookieBoxFactory = new CookieBoxFactory();
  CookieBoxFactorySingletonHolder::instance(cookieBoxFactory);

  // Run the tests.
  bool successfull = runner.run();

  return successfull ? 0 : 1;
}
