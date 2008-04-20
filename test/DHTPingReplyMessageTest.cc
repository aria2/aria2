#include "DHTPingReplyMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

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
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  unsigned char id[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(id, DHT_ID_LENGTH);

  DHTPingReplyMessage msg(localNode, remoteNode, id, transactionID);

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm(new Dictionary());
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  cm->put("r", r);
  r->put("id", new Data(id, DHT_ID_LENGTH));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(v.getBencodedData(), msgbody);
}

} // namespace aria2
