#include "Socks5Handshake.h"

#include <memory>
#include <string>
#include <vector>

#include <cppunit/extensions/HelperMacros.h>

#include "SocketCore.h"
#include "RecoverableException.h"

namespace aria2 {

namespace {
using Bytes = std::vector<unsigned char>;

Bytes bytes(const std::string& s) { return Bytes(s.begin(), s.end()); }

Bytes concat(Bytes a, const Bytes& b)
{
  a.insert(a.end(), b.begin(), b.end());
  return a;
}
} // namespace

class Socks5HandshakeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Socks5HandshakeTest);
  CPPUNIT_TEST(testNoAuthDomain);
  CPPUNIT_TEST(testNumericHostIsSentAsAddress);
  CPPUNIT_TEST(testAuthIPv4);
  CPPUNIT_TEST(testIPv6);
  CPPUNIT_TEST(testAuthFailure);
  CPPUNIT_TEST(testConnectFailure);
  CPPUNIT_TEST(testUnsupportedAuth);
  CPPUNIT_TEST(testEOF);
  CPPUNIT_TEST(testHostnameTooLong);
  CPPUNIT_TEST_SUITE_END();

  std::shared_ptr<SocketCore> client_;
  // Plays the proxy.
  std::shared_ptr<SocketCore> proxy_;

public:
  void setUp()
  {
    SocketCore listener;
    listener.bind(0);
    listener.beginListen();
    client_ = std::make_shared<SocketCore>();
    client_->establishConnection("127.0.0.1", listener.getAddrInfo().port);
    CPPUNIT_ASSERT(listener.isReadable(5));
    proxy_ = listener.acceptConnection();
    proxy_->setBlockingMode();
  }

  void tearDown()
  {
    client_.reset();
    proxy_.reset();
  }

  // Advances the handshake until it needs to read or is done.
  Socks5Handshake::Status pump(Socks5Handshake& h)
  {
    for (int i = 0; i < 100; ++i) {
      auto st = h.advance(*client_);
      if (st == Socks5Handshake::WANT_WRITE) {
        client_->isWritable(1);
        continue;
      }
      return st;
    }
    CPPUNIT_FAIL("handshake did not make progress");
    return Socks5Handshake::DONE;
  }

  // Receives the next n bytes sent by the client.
  Bytes proxyRecv(size_t n)
  {
    Bytes res;
    while (res.size() < n) {
      CPPUNIT_ASSERT(proxy_->isReadable(5));
      unsigned char buf[64];
      size_t len = std::min(sizeof(buf), n - res.size());
      proxy_->readData(buf, len);
      CPPUNIT_ASSERT(len > 0);
      res.insert(res.end(), buf, buf + len);
    }
    return res;
  }

  void proxySend(const Bytes& b)
  {
    CPPUNIT_ASSERT_EQUAL((ssize_t)b.size(),
                         proxy_->writeData(b.data(), b.size()));
    CPPUNIT_ASSERT(client_->isReadable(5));
  }

  // Runs the handshake up to the CONNECT request and returns it.
  // method is the method selected by the proxy.
  Bytes toConnect(Socks5Handshake& h, const Bytes& greeting, uint8_t method)
  {
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    CPPUNIT_ASSERT(greeting == proxyRecv(greeting.size()));
    proxySend({0x05, method});
    return Bytes();
  }

  void testNoAuthDomain()
  {
    Socks5Handshake h("", "", "example.org", "", 8080);
    toConnect(h, {0x05, 0x01, 0x00}, 0x00);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    CPPUNIT_ASSERT(concat({0x05, 0x01, 0x00, 0x03, 11},
                          concat(bytes("example.org"), {0x1f, 0x90})) ==
                   proxyRecv(4 + 1 + 11 + 2));
    proxySend({0x05, 0x00, 0x00, 0x01, 0, 0, 0, 0, 0, 0});
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::DONE, (int)pump(h));
  }

  void testNumericHostIsSentAsAddress()
  {
    Socks5Handshake h("", "", "127.0.0.1", "", 80);
    toConnect(h, {0x05, 0x01, 0x00}, 0x00);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    CPPUNIT_ASSERT((Bytes{0x05, 0x01, 0x00, 0x01, 127, 0, 0, 1, 0x00, 0x50}) ==
                   proxyRecv(10));
  }

  void testAuthIPv4()
  {
    Socks5Handshake h("test", "secret", "example.org", "192.0.2.1", 1080);
    toConnect(h, {0x05, 0x02, 0x00, 0x02}, 0x02);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    CPPUNIT_ASSERT(concat({0x01, 4}, concat(bytes("test"),
                                            concat({6}, bytes("secret")))) ==
                   proxyRecv(2 + 4 + 1 + 6));
    proxySend({0x01, 0x00});
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    // The locally resolved address is sent instead of the hostname.
    CPPUNIT_ASSERT((Bytes{0x05, 0x01, 0x00, 0x01, 192, 0, 2, 1, 0x04, 0x38}) ==
                   proxyRecv(10));
    // BND.ADDR as a domain name, followed by data of the target that
    // must not be consumed by the handshake.
    proxySend({0x05, 0x00, 0x00, 0x03, 3, 'a', 'b', 'c', 0, 80, 'X'});
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::DONE, (int)pump(h));
    char c;
    size_t len = 1;
    client_->readData(&c, len);
    CPPUNIT_ASSERT_EQUAL((size_t)1, len);
    CPPUNIT_ASSERT_EQUAL('X', c);
  }

  void testIPv6()
  {
    Socks5Handshake h("", "", "example.org", "::1", 443);
    toConnect(h, {0x05, 0x01, 0x00}, 0x00);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    Bytes expected{0x05, 0x01, 0x00, 0x04};
    expected.insert(expected.end(), 15, 0);
    expected.push_back(1);
    expected.push_back(0x01);
    expected.push_back(0xbb);
    CPPUNIT_ASSERT(expected == proxyRecv(4 + 16 + 2));
    Bytes reply{0x05, 0x00, 0x00, 0x04};
    reply.insert(reply.end(), 18, 0);
    proxySend(reply);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::DONE, (int)pump(h));
  }

  void testAuthFailure()
  {
    Socks5Handshake h("test", "bad", "example.org", "", 80);
    toConnect(h, {0x05, 0x02, 0x00, 0x02}, 0x02);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    proxyRecv(2 + 4 + 1 + 3);
    proxySend({0x01, 0x01});
    CPPUNIT_ASSERT_THROW(pump(h), RecoverableException);
  }

  void testConnectFailure()
  {
    Socks5Handshake h("", "", "example.org", "", 80);
    toConnect(h, {0x05, 0x01, 0x00}, 0x00);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    proxyRecv(4 + 1 + 11 + 2);
    // Connection refused
    proxySend({0x05, 0x05, 0x00, 0x01, 0, 0, 0, 0, 0, 0});
    CPPUNIT_ASSERT_THROW(pump(h), RecoverableException);
  }

  void testUnsupportedAuth()
  {
    Socks5Handshake h("", "", "example.org", "", 80);
    toConnect(h, {0x05, 0x01, 0x00}, 0xff);
    CPPUNIT_ASSERT_THROW(pump(h), RecoverableException);
  }

  void testEOF()
  {
    Socks5Handshake h("", "", "example.org", "", 80);
    CPPUNIT_ASSERT_EQUAL((int)Socks5Handshake::WANT_READ, (int)pump(h));
    proxyRecv(3);
    proxy_.reset();
    CPPUNIT_ASSERT(client_->isReadable(5));
    CPPUNIT_ASSERT_THROW(pump(h), RecoverableException);
  }

  void testHostnameTooLong()
  {
    CPPUNIT_ASSERT_THROW(Socks5Handshake("", "", std::string(256, 'a'), "", 80),
                         RecoverableException);
    // It is fine if the address is sent instead of the hostname.
    Socks5Handshake("", "", std::string(256, 'a'), "192.0.2.1", 80);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Socks5HandshakeTest);

} // namespace aria2
