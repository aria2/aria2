#include "DHTPingReplyMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

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
  DHTNodeHandle localNode = new DHTNode();
  DHTNodeHandle remoteNode = new DHTNode();

  char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  char id[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(id, DHT_ID_LENGTH);

  DHTPingReplyMessage msg(localNode, remoteNode, (const unsigned char*)id, transactionID);

  string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm = new Dictionary();
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  cm->put("r", r);
  r->put("id", new Data(id, DHT_ID_LENGTH));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(v.getBencodedData(), msgbody);
}
