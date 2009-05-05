#include "FtpConnection.h"

#include <iostream>
#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "Util.h"
#include "SocketCore.h"
#include "Request.h"
#include "Option.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

class FtpConnectionTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(FtpConnectionTest);
  CPPUNIT_TEST(testReceiveResponse);
  CPPUNIT_TEST(testReceiveResponse_overflow);
  CPPUNIT_TEST(testSendMdtm);
  CPPUNIT_TEST(testReceiveMdtmResponse);
  CPPUNIT_TEST(testSendPwd);
  CPPUNIT_TEST(testReceivePwdResponse);
  CPPUNIT_TEST(testReceivePwdResponse_unquotedResponse);
  CPPUNIT_TEST(testReceivePwdResponse_badStatus);
  CPPUNIT_TEST(testSendCwd);
  CPPUNIT_TEST(testSendCwd_baseWorkingDir);
  CPPUNIT_TEST(testReceiveSizeResponse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<SocketCore> _serverSocket;
  uint16_t _listenPort;
  SharedHandle<SocketCore> _clientSocket;
  SharedHandle<FtpConnection> _ftp;
  SharedHandle<Option> _option;
  SharedHandle<AuthConfigFactory> _authConfigFactory;
public:
  void setUp()
  {
    _option.reset(new Option());
    _authConfigFactory.reset(new AuthConfigFactory(_option.get()));

    //_ftpServerSocket.reset(new SocketCore());
    SharedHandle<SocketCore> listenSocket(new SocketCore());
    listenSocket->bind(0);
    listenSocket->beginListen();
    std::pair<std::string, uint16_t> addrinfo;
    listenSocket->getAddrInfo(addrinfo);
    _listenPort = addrinfo.second;

    SharedHandle<Request> req(new Request());
    req->setUrl("ftp://localhost/dir/file.img");

    _clientSocket.reset(new SocketCore());
    _clientSocket->establishConnection("localhost", _listenPort);

    while(!_clientSocket->isWritable(0));
    _clientSocket->setBlockingMode();

    _serverSocket.reset(listenSocket->acceptConnection());
    _ftp.reset(new FtpConnection(1, _clientSocket, req,
				 _authConfigFactory->createAuthConfig(req),
				 _option.get()));
  }

  void tearDown() {}

  void testSendMdtm();
  void testReceiveMdtmResponse();
  void testReceiveResponse();
  void testReceiveResponse_overflow();
  void testSendPwd();
  void testReceivePwdResponse();
  void testReceivePwdResponse_unquotedResponse();
  void testReceivePwdResponse_badStatus();
  void testSendCwd();
  void testSendCwd_baseWorkingDir();
  void testReceiveSizeResponse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(FtpConnectionTest);

static void waitRead(const SharedHandle<SocketCore>& socket)
{
  while(!socket->isReadable(0));
}

void FtpConnectionTest::testReceiveResponse()
{
  _serverSocket->writeData("100");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData(" single line response");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData("\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)100, _ftp->receiveResponse());
  // 2 responses in the buffer
  _serverSocket->writeData("101 single1\r\n"
			   "102 single2\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)101, _ftp->receiveResponse());
  CPPUNIT_ASSERT_EQUAL((unsigned int)102, _ftp->receiveResponse());

  _serverSocket->writeData("103-multi line response\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData("103-line2\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData("103");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData(" ");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  _serverSocket->writeData("last\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)103, _ftp->receiveResponse());

  _serverSocket->writeData("104-multi\r\n"
			   "104 \r\n"
			   "105-multi\r\n"
			   "105 \r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)104, _ftp->receiveResponse());
  CPPUNIT_ASSERT_EQUAL((unsigned int)105, _ftp->receiveResponse());
}

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
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveMdtmResponse(t));
    _serverSocket->writeData("\r\n");
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTime());
  }
  {
    // see milli second part is ignored
    Time t;
    _serverSocket->writeData("213 20080908124312.014\r\n");
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTime());
  }
  {
    // hhmmss part is missing
    Time t;
    _serverSocket->writeData("213 20080908\r\n");
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    CPPUNIT_ASSERT(t.bad());
  }
  {
    // invalid month: 19, we don't care about invalid month..
    Time t;
    _serverSocket->writeData("213 20081908124312\r\n");
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveMdtmResponse(t));
    // Wed Jul 8 12:43:12 2009
    CPPUNIT_ASSERT_EQUAL((time_t)1247056992, t.getTime());
  }
  {
    Time t;
    _serverSocket->writeData("550 File Not Found\r\n");
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)550, _ftp->receiveMdtmResponse(t));
  }
}

void FtpConnectionTest::testReceiveResponse_overflow()
{
  char data[1024];
  memset(data, 0, sizeof(data));
  memcpy(data, "213 ", 4);
  for(int i = 0; i < 64; ++i) {
    _serverSocket->writeData(data, sizeof(data));
    waitRead(_clientSocket);
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receiveResponse());
  }
  _serverSocket->writeData(data, sizeof(data));
  waitRead(_clientSocket);
  try {
    _ftp->receiveResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlRetryEx& e) {
    // success
  }
}

void FtpConnectionTest::testSendPwd()
{
  _ftp->sendPwd();
  char data[32];
  size_t len = sizeof(data);
  _serverSocket->readData(data, len);
  CPPUNIT_ASSERT_EQUAL((size_t)5, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("PWD\r\n"), std::string(data));
}

void FtpConnectionTest::testReceivePwdResponse()
{
  std::string pwd;
  _serverSocket->writeData("257 ");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)0, _ftp->receivePwdResponse(pwd));
  CPPUNIT_ASSERT(pwd.empty());
  _serverSocket->writeData("\"/dir/to\" is your directory.\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)257, _ftp->receivePwdResponse(pwd));
  CPPUNIT_ASSERT_EQUAL(std::string("/dir/to"), pwd);
}

void FtpConnectionTest::testReceivePwdResponse_unquotedResponse()
{
  std::string pwd;
  _serverSocket->writeData("257 /dir/to\r\n");
  waitRead(_clientSocket);
  try {
    _ftp->receivePwdResponse(pwd);
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    // success
  }
}

void FtpConnectionTest::testReceivePwdResponse_badStatus()
{
  std::string pwd;
  _serverSocket->writeData("500 failed\r\n");
  waitRead(_clientSocket);
  CPPUNIT_ASSERT_EQUAL((unsigned int)500, _ftp->receivePwdResponse(pwd));
  CPPUNIT_ASSERT(pwd.empty());
}

void FtpConnectionTest::testSendCwd()
{
  _ftp->sendCwd();
  char data[32];
  size_t len = sizeof(data);
  _serverSocket->readData(data, len);
  CPPUNIT_ASSERT_EQUAL((size_t)10, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("CWD /dir\r\n"), std::string(data));
}

void FtpConnectionTest::testSendCwd_baseWorkingDir()
{
  _ftp->setBaseWorkingDir("/base");
  _ftp->sendCwd();
  char data[32];
  size_t len = sizeof(data);
  _serverSocket->readData(data, len);
  CPPUNIT_ASSERT_EQUAL((size_t)15, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("CWD /base/dir\r\n"), std::string(data));
}

void FtpConnectionTest::testReceiveSizeResponse()
{
  _serverSocket->writeData("213 4294967296\r\n");
  waitRead(_clientSocket);
  uint64_t size;
  CPPUNIT_ASSERT_EQUAL((unsigned int)213, _ftp->receiveSizeResponse(size));
  CPPUNIT_ASSERT_EQUAL((uint64_t)4294967296LL, size);
}

} // namespace aria2
