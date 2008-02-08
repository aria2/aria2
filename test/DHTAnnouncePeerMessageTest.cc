#include "DHTAnnouncePeerMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTAnnouncePeerMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTAnnouncePeerMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTAnnouncePeerMessageTest);

void DHTAnnouncePeerMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(infoHash, DHT_ID_LENGTH);

  std::string token = "token";
  uint16_t port = 6881;

  DHTAnnouncePeerMessage msg(localNode, remoteNode, infoHash, port, token, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("q"));
  cm->put("q", new Data("announce_peer"));
  Dictionary* a = new Dictionary();
  cm->put("a", a);
  a->put("id", new Data(reinterpret_cast<const char*>(localNode->getID()), DHT_ID_LENGTH));
  a->put("info_hash", new Data(infoHash, DHT_ID_LENGTH));
  a->put("port", new Data(Util::uitos(port), true));
  a->put("token", new Data(token));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
		       Util::urlencode(msgbody));
}

} // namespace aria2
