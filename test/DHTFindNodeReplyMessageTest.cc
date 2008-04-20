#include "DHTFindNodeReplyMessage.h"
#include "DHTNode.h"
#include "DHTUtil.h"
#include "BencodeVisitor.h"
#include "Dictionary.h"
#include "Data.h"
#include "Exception.h"
#include "Util.h"
#include "DHTBucket.h"
#include "PeerMessageUtil.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTFindNodeReplyMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTFindNodeReplyMessageTest);
  CPPUNIT_TEST(testGetBencodedMessage);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGetBencodedMessage();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTFindNodeReplyMessageTest);

void DHTFindNodeReplyMessageTest::testGetBencodedMessage()
{
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTNode> remoteNode(new DHTNode());

  unsigned char tid[DHT_TRANSACTION_ID_LENGTH];
  DHTUtil::generateRandomData(tid, DHT_TRANSACTION_ID_LENGTH);
  std::string transactionID(&tid[0], &tid[DHT_TRANSACTION_ID_LENGTH]);

  DHTFindNodeReplyMessage msg(localNode, remoteNode, transactionID);

  std::string compactNodeInfo;
  SharedHandle<DHTNode> nodes[8];
  for(size_t i = 0; i < DHTBucket::K; ++i) {
    nodes[i].reset(new DHTNode());
    nodes[i]->setIPAddress("192.168.0."+Util::uitos(i+1));
    nodes[i]->setPort(6881+i);

    unsigned char buf[6];
    CPPUNIT_ASSERT(PeerMessageUtil::createcompact(buf, nodes[i]->getIPAddress(), nodes[i]->getPort()));
    compactNodeInfo +=
      std::string(&nodes[i]->getID()[0], &nodes[i]->getID()[DHT_ID_LENGTH])+
      std::string(&buf[0], &buf[sizeof(buf)]);
  }
  msg.setClosestKNodes(std::deque<SharedHandle<DHTNode> >(&nodes[0], &nodes[DHTBucket::K]));

  std::string msgbody = msg.getBencodedMessage();

  SharedHandle<Dictionary> cm(new Dictionary());
  cm->put("t", new Data(transactionID));
  cm->put("y", new Data("r"));
  Dictionary* r = new Dictionary();
  cm->put("r", r);
  r->put("id", new Data(localNode->getID(), DHT_ID_LENGTH));
  r->put("nodes", new Data(compactNodeInfo));

  BencodeVisitor v;
  cm->accept(&v);

  CPPUNIT_ASSERT_EQUAL(v.getBencodedData(), msgbody);
}

} // namespace aria2
