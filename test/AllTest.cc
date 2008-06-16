#include "Platform.h"
#include "CookieBoxFactory.h"
#include <iostream>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

using aria2::SharedHandle;
using aria2::SingletonHolder;

int main(int argc, char* argv[]) {
  aria2::Platform platform;

#ifdef ENABLE_NLS
  // Set locale to C to prevent the messages to be localized.
  setlocale (LC_CTYPE, "C");
  setlocale (LC_MESSAGES, "C");
#endif // ENABLE_NLS

  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  
  // setup

  SharedHandle<aria2::CookieBoxFactory> cookieBoxFactory
    (new aria2::CookieBoxFactory());
  SingletonHolder<SharedHandle<aria2::CookieBoxFactory> >::instance(cookieBoxFactory);

  // Run the tests.
  bool successfull = runner.run();

  return successfull ? 0 : 1;
}
