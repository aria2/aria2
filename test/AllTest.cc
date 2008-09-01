#include "Platform.h"
#include <iostream>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

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

  // Run the tests.
  bool successfull = runner.run();

  return successfull ? 0 : 1;
}
