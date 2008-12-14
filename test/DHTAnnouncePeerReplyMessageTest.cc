#include "DHTAnnouncePeerReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "DHTUtil.h"
#include "Exception.h"
#include "Util.h"
#include "bencode.h"

namespace aria2 {

class DHTAnnouncePeerReplyMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTAnnouncePeerReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTAnnouncePeerReplyMessageTest);

void DHTAnnouncePeerReplyMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTAnnouncePeerReplyMessage msg(localNode, remoteNode, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  bencode::BDE dict = bencode::BDE::dict();
  dict["t"] = transactionID;
  dict["y"] = bencode::BDE("r");
  bencode::BDE rDict = bencode::BDE::dict();
  rDict["id"] = bencode::BDE(localNode->getID(), DHT_ID_LENGTH);
  dict["r"] = rDict;

  CPPUNIT_ASSERT_EQUAL(bencode::encode(dict), msgbody);
}

} // namespace aria2
