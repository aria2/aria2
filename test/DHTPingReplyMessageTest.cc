#include "DHTPingReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "bencode2.h"

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
  msg.setVersion("A200");
  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  SharedHandle<Dict> rDict = Dict::g();
  rDict->put("id", String::g(id, DHT_ID_LENGTH));
  dict.put("r", rDict);

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
}

} // namespace aria2
