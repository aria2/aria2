#include "FtpConnection.h"
#include "Exception.h"
#include "Util.h"
#include "SocketCore.h"
#include "Request.h"
#include "Option.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class FtpConnectionTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FtpConnectionTest);
  CPPUNIT_TEST(testSendMdtm);
  CPPUNIT_TEST(testReceiveMdtmResponse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<SocketCore> _serverSocket;
  uint16_t _listenPort;
  SharedHandle<FtpConnection> _ftp;
  Option _option;
public:
  void setUp()
  {
    //_ftpServerSocket.reset(new SocketCore());
    SharedHandle<SocketCore> listenSocket(new SocketCore());
    listenSocket->bind(0);
    listenSocket->beginListen();
    std::pair<std::string, uint16_t> addrinfo;
    listenSocket->getAddrInfo(addrinfo);
    _listenPort = addrinfo.second;

    SharedHandle<Request> req(new Request());
    req->setUrl("ftp://localhost/dir/file.img");

    SharedHandle<SocketCore> clientSocket(new SocketCore());
    clientSocket->establishConnection("127.0.0.1", _listenPort);

    while(!clientSocket->isWritable(0));
    clientSocket->setBlockingMode();

    _serverSocket.reset(listenSocket->acceptConnection());
    _ftp.reset(new FtpConnection(1, clientSocket, req, &_option));
  }

  void tearDown() {}

  void testSendMdtm();
  void testReceiveMdtmResponse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FtpConnectionTest);

void FtpConnectionTest::testSendMdtm()
{
  _ftp->sendMdtm();
  char data[32];
  size_t len = sizeof(data);
  _serverSocket->readData(data, len);
  CPPUNIT_ASSERT_EQUAL((size_t)15, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("MDTM file.img\r\n"), std::string(data));
}

void FtpConnectionTest::testReceiveMdtmResponse()
{
  {
    Time t;
    _serverSocket->writeData("213 20080908124312");
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveMdtmResponse(t));
    _serverSocket->writeData("\r\n");
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTime());
  }
  {
    // see milli second part is ignored
    Time t;
    _serverSocket->writeData("213 20080908124312.014\r\n");
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTime());
  }
  {
    // hhmmss part is missing
    Time t;
    _serverSocket->writeData("213 20080908\r\n");
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)-1, t.getTime());
  }
  {
    // invalid month: 19
    Time t;
    _serverSocket->writeData("213 20081908124312\r\n");
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)-1, t.getTime());
  }
  {
    Time t;
    _serverSocket->writeData("550 File Not Found\r\n");
    CPPUNIT_ASSERT_EQUAL((unsigned int)550, _ftp->receiveMdtmResponse(t));
  }
}

} // namespace aria2
