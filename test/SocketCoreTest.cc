#include "SocketCore.h"

#include <cstring>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "a2functional.h"
#include "Exception.h"

namespace aria2 {

class SocketCoreTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SocketCoreTest);
  CPPUNIT_TEST(testWriteAndReadDatagram);
  CPPUNIT_TEST(testGetSocketError);
  CPPUNIT_TEST(testInetNtop);
  CPPUNIT_TEST(testInetPton);
  CPPUNIT_TEST(testGetBinAddr);
  CPPUNIT_TEST(testVerifyHostname);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testWriteAndReadDatagram();
  void testGetSocketError();
  void testInetNtop();
  void testInetPton();
  void testGetBinAddr();
  void testVerifyHostname();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SocketCoreTest);

void SocketCoreTest::testWriteAndReadDatagram()
{
  try {
    SocketCore s(SOCK_DGRAM);
    s.bind(0);
    SocketCore c(SOCK_DGRAM);
    c.bind(0);

    std::pair<std::string, uint16_t> svaddr;
    s.getAddrInfo(svaddr);

    std::string message1 = "hello world.";
    c.writeData(message1.c_str(), message1.size(), "localhost", svaddr.second);
    std::string message2 = "chocolate coated pie";
    c.writeData(message2.c_str(), message2.size(), "localhost", svaddr.second);

    char readbuffer[100];
    std::pair<std::string, uint16_t> peer;
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer), peer);
      // commented out because ip address may vary
      //CPPUNIT_ASSERT_EQUAL(std::std::string("127.0.0.1"), peer.first);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message1.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message1, std::string(readbuffer));
    }
    {
      ssize_t rlength = s.readDataFrom(readbuffer, sizeof(readbuffer), peer);
      CPPUNIT_ASSERT_EQUAL((ssize_t)message2.size(), rlength);
      readbuffer[rlength] = '\0';
      CPPUNIT_ASSERT_EQUAL(message2, std::string(readbuffer));
    }
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
    CPPUNIT_FAIL("exception thrown");
  }
}

void SocketCoreTest::testGetSocketError()
{
  SocketCore s;
  s.bind(0);
  // See there is no error at this point
  CPPUNIT_ASSERT_EQUAL(std::string(""), s.getSocketError());
}

void SocketCoreTest::testInetNtop()
{
  char dest[NI_MAXHOST];
  {
    std::string s = "192.168.0.1";
    addrinfo* res;
    CPPUNIT_ASSERT_EQUAL(0, callGetaddrinfo(&res, s.c_str(), 0, AF_INET,
                                            SOCK_STREAM, 0, 0));
    WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
    sockaddr_in addr;
    memcpy(&addr, res->ai_addr, sizeof(addr));
    CPPUNIT_ASSERT_EQUAL(0, inetNtop(AF_INET, &addr.sin_addr,
                                     dest, sizeof(dest)));
    CPPUNIT_ASSERT_EQUAL(s, std::string(dest));
  }
  {
    std::string s = "2001:db8::2:1";
    addrinfo* res;
    CPPUNIT_ASSERT_EQUAL(0, callGetaddrinfo(&res, s.c_str(), 0, AF_INET6,
                                            SOCK_STREAM, 0, 0));
    WSAAPI_AUTO_DELETE<struct addrinfo*> resDeleter(res, freeaddrinfo);
    sockaddr_in6 addr;
    memcpy(&addr, res->ai_addr, sizeof(addr));
    CPPUNIT_ASSERT_EQUAL(0, inetNtop(AF_INET6, &addr.sin6_addr,
                                     dest, sizeof(dest)));
    CPPUNIT_ASSERT_EQUAL(s, std::string(dest));
  }
}

void SocketCoreTest::testInetPton()
{
  {
    const char ipaddr[] = "192.168.0.1";
    in_addr ans;
    CPPUNIT_ASSERT_EQUAL((size_t)4, net::getBinAddr(&ans, ipaddr));
    in_addr dest;
    CPPUNIT_ASSERT_EQUAL(0, inetPton(AF_INET, ipaddr, &dest));
    CPPUNIT_ASSERT(memcmp(&ans, &dest, sizeof(ans)) == 0);
  }
  {
    const char ipaddr[] = "2001:db8::2:1";
    in6_addr ans;
    CPPUNIT_ASSERT_EQUAL((size_t)16, net::getBinAddr(&ans, ipaddr));
    in6_addr dest;
    CPPUNIT_ASSERT_EQUAL(0, inetPton(AF_INET6, ipaddr, &dest));
    CPPUNIT_ASSERT(memcmp(&ans, &dest, sizeof(ans)) == 0);
  }
  unsigned char dest[16];
  CPPUNIT_ASSERT_EQUAL(-1, inetPton(AF_INET, "localhost", &dest));
  CPPUNIT_ASSERT_EQUAL(-1, inetPton(AF_INET6, "localhost", &dest));
}

void SocketCoreTest::testGetBinAddr()
{
  unsigned char dest[16];
  unsigned char ans1[] = { 192, 168, 0, 1 };
  CPPUNIT_ASSERT_EQUAL((size_t)4, net::getBinAddr(dest, "192.168.0.1"));
  CPPUNIT_ASSERT(std::equal(&dest[0], &dest[4], &ans1[0]));

  unsigned char ans2[] = { 0x20u, 0x01u, 0x0du, 0xb8u,
                           0x00u, 0x00u, 0x00u, 0x00u,
                           0x00u, 0x00u, 0x00u, 0x00u,
                           0x00u, 0x02u, 0x00u, 0x01u };
  CPPUNIT_ASSERT_EQUAL((size_t)16, net::getBinAddr(dest, "2001:db8::2:1"));
  CPPUNIT_ASSERT(std::equal(&dest[0], &dest[16], &ans2[0]));

  CPPUNIT_ASSERT_EQUAL((size_t)0, net::getBinAddr(dest, "localhost"));
}

void SocketCoreTest::testVerifyHostname()
{
  {
    std::vector<std::string> dnsNames, ipAddrs;
    std::string commonName;
    CPPUNIT_ASSERT(!net::verifyHostname("example.org",
                                        dnsNames, ipAddrs, commonName));
  }
  {
    // Only commonName is provided
    std::vector<std::string> dnsNames, ipAddrs;
    std::string commonName = "example.org";
    CPPUNIT_ASSERT(net::verifyHostname("example.org",
                                       dnsNames, ipAddrs, commonName));
  }
  {
    // Match against dNSName in subjectAltName
    std::vector<std::string> dnsNames, ipAddrs;
    dnsNames.push_back("foo");
    dnsNames.push_back("example.org");
    std::string commonName = "exampleX.org";
    CPPUNIT_ASSERT(net::verifyHostname("example.org",
                                       dnsNames, ipAddrs, commonName));
  }
  {
    // If dNsName is provided, don't match with commonName
    std::vector<std::string> dnsNames, ipAddrs;
    dnsNames.push_back("foo");
    dnsNames.push_back("exampleX.org");
    ipAddrs.push_back("example.org");
    std::string commonName = "example.org";
    CPPUNIT_ASSERT(!net::verifyHostname("example.org",
                                        dnsNames, ipAddrs, commonName));
  }
  {
    // IPAddress in dnsName don't match.
    std::vector<std::string> dnsNames, ipAddrs;
    dnsNames.push_back("192.168.0.1");
    std::string commonName = "example.org";
    CPPUNIT_ASSERT(!net::verifyHostname("192.168.0.1",
                                        dnsNames, ipAddrs, commonName));
  }
  {
    // IPAddress string match with commonName
    std::vector<std::string> dnsNames, ipAddrs;
    std::string commonName = "192.168.0.1";
    CPPUNIT_ASSERT(net::verifyHostname("192.168.0.1",
                                       dnsNames, ipAddrs, commonName));
  }
  {
    // Match against iPAddress in subjectAltName
    std::vector<std::string> dnsNames, ipAddrs;
    unsigned char binAddr[16];
    size_t len;
    len = net::getBinAddr(binAddr, "192.168.0.1");
    ipAddrs.push_back(std::string(binAddr, binAddr+len));
    std::string commonName = "example.org";
    CPPUNIT_ASSERT(net::verifyHostname("192.168.0.1",
                                       dnsNames, ipAddrs, commonName));
  }
  {
    // Match against iPAddress (ipv6) in subjectAltName
    std::vector<std::string> dnsNames, ipAddrs;
    unsigned char binAddr[16];
    size_t len;
    len = net::getBinAddr(binAddr, "::1");
    ipAddrs.push_back(std::string(binAddr, binAddr+len));
    std::string commonName = "example.org";
    CPPUNIT_ASSERT(net::verifyHostname("::1",
                                       dnsNames, ipAddrs, commonName));
  }
  {
    // If iPAddress is privided, don't match with commonName
    std::vector<std::string> dnsNames, ipAddrs;
    unsigned char binAddr[16];
    size_t len;
    len = net::getBinAddr(binAddr, "192.168.0.2");
    ipAddrs.push_back(std::string(binAddr, binAddr+len));
    std::string commonName = "192.168.0.1";
    CPPUNIT_ASSERT(!net::verifyHostname("192.168.0.1",
                                        dnsNames, ipAddrs, commonName));
  }
}

} // namespace aria2
