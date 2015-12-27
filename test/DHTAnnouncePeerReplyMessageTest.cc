#include "DHTAnnouncePeerReplyMessage.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include "bencode2.h"

namespace aria2 {

class DHTAnnouncePeerReplyMessageTest : public CppUnit::TestFixture {

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
  std::shared_ptr<DHTNode> localNode(new DHTNode());
  std::shared_ptr<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  util::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTAnnouncePeerReplyMessage msg(localNode, remoteNode, transactionID);
  msg.setVersion("A200");
  std::string msgbody = msg.getBencodedMessage();

  Dict dict;
  dict.put("t", transactionID);
  dict.put("v", "A200");
  dict.put("y", "r");
  auto rDict = Dict::g();
  rDict->put("id", String::g(localNode->getID(), DHT_ID_LENGTH));
  dict.put("r", std::move(rDict));

  CPPUNIT_ASSERT_EQUAL(bencode2::encode(&dict), msgbody);
}

} // namespace aria2
