#include "DHTGetPeersMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTGetPeersMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTGetPeersMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTGetPeersMessageTest);

void DHTGetPeersMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode = new DHTNode();
  SharedHandle<DHTNode> remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char infoHash[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(infoHash, DHT_ID_LENGTH);

  DHTGetPeersMessage msg(localNode, remoteNode, infoHash, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("q"));
  cm->put("q", new Data("get_peers"));
  Dictionary* a = new Dictionary();
  cm->put("a", a);
  a->put("id", new Data(reinterpret_cast<const char*>(localNode->getID()), DHT_ID_LENGTH));
  a->put("info_hash", new Data(infoHash, DHT_ID_LENGTH));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(Util::urlencode(v.getBencodedData()),
		       Util::urlencode(msgbody));
}

} // namespace aria2
