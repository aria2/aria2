#include "AsyncNameResolver.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "SocketCore.h"

namespace aria2 {

class AsyncNameResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AsyncNameResolverTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(AsyncNameResolverTest);

} // namespace aria2
