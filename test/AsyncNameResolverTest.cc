#include "AsyncNameResolver.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "SocketCore.h"

namespace aria2 {

class AsyncNameResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AsyncNameResolverTest);
  CPPUNIT_TEST(testParseAsyncDNSServers);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testParseAsyncDNSServers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AsyncNameResolverTest);

void AsyncNameResolverTest::testParseAsyncDNSServers()
{
#ifdef HAVE_ARES_ADDR_NODE
  in_addr ans4;
  CPPUNIT_ASSERT_EQUAL((size_t)4, net::getBinAddr(&ans4, "192.168.0.1"));
  in6_addr ans6;
  CPPUNIT_ASSERT_EQUAL((size_t)16, net::getBinAddr(&ans6, "2001:db8::2:1"));

  ares_addr_node* root;
  root = parseAsyncDNSServers("192.168.0.1,2001:db8::2:1");
  ares_addr_node* node = root;
  CPPUNIT_ASSERT(node);
  CPPUNIT_ASSERT_EQUAL(AF_INET, node->family);
  CPPUNIT_ASSERT(memcmp(&ans4, &node->addr, sizeof(ans4)) == 0);
  node = node->next;
  CPPUNIT_ASSERT(node);
  CPPUNIT_ASSERT_EQUAL(AF_INET6, node->family);
  CPPUNIT_ASSERT(memcmp(&ans6, &node->addr, sizeof(ans6)) == 0);
  for (node = root; node;) {
    ares_addr_node* nextNode = node->next;
    delete node;
    node = nextNode;
  }
#endif // HAVE_ARES_ADDR_NODE
}

} // namespace aria2
