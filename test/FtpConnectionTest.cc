#include "FtpConnection.h"

#include <iostream>
#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "SocketCore.h"
#include "Request.h"
#include "Option.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

class FtpConnectionTest : public CppUnit::TestFixture {

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
  CPPUNIT_TEST(testSendSize);
  CPPUNIT_TEST(testReceiveSizeResponse);
  CPPUNIT_TEST(testSendRetr);
  CPPUNIT_TEST(testReceiveEpsvResponse);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<SocketCore> serverSocket_;
  uint16_t listenPort_;
  std::shared_ptr<SocketCore> clientSocket_;
  std::shared_ptr<FtpConnection> ftp_;
  std::shared_ptr<Option> option_;
  std::shared_ptr<AuthConfigFactory> authConfigFactory_;
  std::shared_ptr<Request> req_;

public:
  void setUp()
  {
    option_.reset(new Option());
    authConfigFactory_.reset(new AuthConfigFactory());

    //_ftpServerSocket.reset(new SocketCore());
    std::shared_ptr<SocketCore> listenSocket(new SocketCore());
    listenSocket->bind(0);
    listenSocket->beginListen();
    listenSocket->setBlockingMode();
    listenPort_ = listenSocket->getAddrInfo().port;

    req_.reset(new Request());
    req_->setUri("ftp://localhost/dir%20sp/hello%20world.img");

    clientSocket_.reset(new SocketCore());
    clientSocket_->establishConnection("localhost", listenPort_);

    while (!clientSocket_->isWritable(0))
      ;

    serverSocket_ = listenSocket->acceptConnection();
    serverSocket_->setBlockingMode();
    ftp_.reset(new FtpConnection(
        1, clientSocket_, req_,
        authConfigFactory_->createAuthConfig(req_, option_.get()),
        option_.get()));
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
  void testSendSize();
  void testReceiveSizeResponse();
  void testSendRetr();
  void testReceiveEpsvResponse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FtpConnectionTest);

namespace {
void waitRead(const std::shared_ptr<SocketCore>& socket)
{
  while (!socket->isReadable(0))
    ;
}
} // namespace

void FtpConnectionTest::testReceiveResponse()
{
  serverSocket_->writeData("100");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData(" single line response");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData("\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(100, ftp_->receiveResponse());
  // 2 responses in the buffer
  serverSocket_->writeData("101 single1\r\n"
                           "102 single2\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(101, ftp_->receiveResponse());
  CPPUNIT_ASSERT_EQUAL(102, ftp_->receiveResponse());

  serverSocket_->writeData("103-multi line response\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData("103-line2\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData("103");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData(" ");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  serverSocket_->writeData("last\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(103, ftp_->receiveResponse());

  serverSocket_->writeData("104-multi\r\n"
                           "104 \r\n"
                           "105-multi\r\n"
                           "105 \r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(104, ftp_->receiveResponse());
  CPPUNIT_ASSERT_EQUAL(105, ftp_->receiveResponse());
}

void FtpConnectionTest::testSendMdtm()
{
  ftp_->sendMdtm();
  char data[32];
  size_t len = sizeof(data);
  serverSocket_->readData(data, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("MDTM hello world.img\r\n"),
                       std::string(data));
}

void FtpConnectionTest::testReceiveMdtmResponse()
{
  {
    Time t;
    serverSocket_->writeData("213 20080908124312");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveMdtmResponse(t));
    serverSocket_->writeData("\r\n");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(213, ftp_->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTimeFromEpoch());
  }
  {
    // see milli second part is ignored
    Time t;
    serverSocket_->writeData("213 20080908124312.014\r\n");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(213, ftp_->receiveMdtmResponse(t));
    CPPUNIT_ASSERT_EQUAL((time_t)1220877792, t.getTimeFromEpoch());
  }
  {
    // hhmmss part is missing
    Time t;
    serverSocket_->writeData("213 20080908\r\n");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(213, ftp_->receiveMdtmResponse(t));
    CPPUNIT_ASSERT(t.bad());
  }
  {
    // invalid month: 19
    Time t;
    serverSocket_->writeData("213 20081908124312\r\n");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(213, ftp_->receiveMdtmResponse(t));
#ifdef HAVE_TIMEGM
    // Time will be normalized. Wed Jul 8 12:43:12 2009
    CPPUNIT_ASSERT_EQUAL((time_t)1247056992, t.getTimeFromEpoch());
#else  // !HAVE_TIMEGM
    // The replacement timegm does not normalize.
    CPPUNIT_ASSERT_EQUAL((time_t)-1, t.getTimeFromEpoch());
#endif // !HAVE_TIMEGM
  }
  {
    Time t;
    serverSocket_->writeData("550 File Not Found\r\n");
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(550, ftp_->receiveMdtmResponse(t));
  }
}

void FtpConnectionTest::testReceiveResponse_overflow()
{
  char data[1_k];
  memset(data, 0, sizeof(data));
  memcpy(data, "213 ", 4);
  for (int i = 0; i < 64; ++i) {
    serverSocket_->writeData(data, sizeof(data));
    waitRead(clientSocket_);
    CPPUNIT_ASSERT_EQUAL(0, ftp_->receiveResponse());
  }
  serverSocket_->writeData(data, sizeof(data));
  waitRead(clientSocket_);
  try {
    ftp_->receiveResponse();
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlRetryEx& e) {
    // success
  }
}

void FtpConnectionTest::testSendPwd()
{
  ftp_->sendPwd();
  char data[32];
  size_t len = sizeof(data);
  serverSocket_->readData(data, len);
  CPPUNIT_ASSERT_EQUAL((size_t)5, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("PWD\r\n"), std::string(data));
}

void FtpConnectionTest::testReceivePwdResponse()
{
  std::string pwd;
  serverSocket_->writeData("257 ");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(0, ftp_->receivePwdResponse(pwd));
  CPPUNIT_ASSERT(pwd.empty());
  serverSocket_->writeData("\"/dir/to\" is your directory.\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(257, ftp_->receivePwdResponse(pwd));
  CPPUNIT_ASSERT_EQUAL(std::string("/dir/to"), pwd);
}

void FtpConnectionTest::testReceivePwdResponse_unquotedResponse()
{
  std::string pwd;
  serverSocket_->writeData("257 /dir/to\r\n");
  waitRead(clientSocket_);
  try {
    ftp_->receivePwdResponse(pwd);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (DlAbortEx& e) {
    // success
  }
}

void FtpConnectionTest::testReceivePwdResponse_badStatus()
{
  std::string pwd;
  serverSocket_->writeData("500 failed\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(500, ftp_->receivePwdResponse(pwd));
  CPPUNIT_ASSERT(pwd.empty());
}

void FtpConnectionTest::testSendCwd()
{
  ftp_->sendCwd("%2Fdir%20sp");
  char data[32];
  size_t len = sizeof(data);
  serverSocket_->readData(data, len);
  data[len] = '\0';
  CPPUNIT_ASSERT_EQUAL(std::string("CWD /dir sp\r\n"), std::string(data));
}

void FtpConnectionTest::testSendSize()
{
  ftp_->sendSize();
  char data[32];
  size_t len = sizeof(data);
  serverSocket_->readData(data, len);
  CPPUNIT_ASSERT_EQUAL(std::string("SIZE hello world.img\r\n"),
                       std::string(&data[0], &data[len]));
}

void FtpConnectionTest::testReceiveSizeResponse()
{
  serverSocket_->writeData("213 4294967296\r\n");
  waitRead(clientSocket_);
  int64_t size;
  CPPUNIT_ASSERT_EQUAL(213, ftp_->receiveSizeResponse(size));
  CPPUNIT_ASSERT_EQUAL((int64_t)4294967296LL, size);
}

void FtpConnectionTest::testSendRetr()
{
  ftp_->sendRetr();
  char data[32];
  size_t len = sizeof(data);
  serverSocket_->readData(data, len);
  CPPUNIT_ASSERT_EQUAL(std::string("RETR hello world.img\r\n"),
                       std::string(&data[0], &data[len]));
}

void FtpConnectionTest::testReceiveEpsvResponse()
{
  serverSocket_->writeData("229 Success (|||12000|)\r\n");
  waitRead(clientSocket_);
  uint16_t port = 0;
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)12000, port);

  serverSocket_->writeData("229 Success |||12000|)\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, port);

  serverSocket_->writeData("229 Success (|||12000|\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, port);

  serverSocket_->writeData("229 Success ()|||12000|\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, port);

  serverSocket_->writeData("229 Success )(|||12000|)\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, port);

  serverSocket_->writeData("229 Success )(||12000|)\r\n");
  waitRead(clientSocket_);
  CPPUNIT_ASSERT_EQUAL(229, ftp_->receiveEpsvResponse(port));
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, port);
}

} // namespace aria2
