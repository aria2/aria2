#include "CookieBoxFactory.h"
#include <iostream>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#ifdef HAVE_LIBSSL
# include <openssl/err.h>
# include <openssl/ssl.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

using aria2::SharedHandle;
using aria2::SingletonHolder;

int main(int argc, char* argv[]) {
  CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
  
  // setup

#ifdef HAVE_LIBSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  gnutls_global_init();
#endif // HAVE_LIBGNUTLS

  SharedHandle<aria2::CookieBoxFactory> cookieBoxFactory = new aria2::CookieBoxFactory();
  SingletonHolder<SharedHandle<aria2::CookieBoxFactory> >::instance(cookieBoxFactory);

  // Run the tests.
  bool successfull = runner.run();

#ifdef HAVE_LIBGNUTLS
  gnutls_global_deinit();
#endif // HAVE_LIBGNUTLS

  return successfull ? 0 : 1;
}
