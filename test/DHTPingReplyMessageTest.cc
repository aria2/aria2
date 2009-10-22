#include "DHTPingReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "bencode.h"

namespace aria2 {

class DHTPingReplyMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTPingReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTPingReplyMessageTest);

void DHTPingReplyMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char id[DHT_ID_LENGTH];
  util::generateRandomData(id, DHT_ID_LENGTH);

  DHTPingReplyMessage msg(localNode, remoteNode, id, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  BDE dict = BDE::dict();
  dict["t"] = transactionID;
  dict["y"] = BDE("r");
  BDE rDict = BDE::dict();
  rDict["id"] = BDE(id, DHT_ID_LENGTH);
  dict["r"] = rDict;

  CPPUNIT_ASSERT_EQUAL(bencode::encode(dict), msgbody);
}

} // namespace aria2
