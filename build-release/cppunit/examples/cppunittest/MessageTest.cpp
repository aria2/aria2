#include "CoreSuite.h"
#include "MessageTest.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MessageTest,
                                       coreSuiteName() );


MessageTest::MessageTest()
{
}


MessageTest::~MessageTest()
{
}


void 
MessageTest::setUp()
{
  m_message = new CPPUNIT_NS::Message();
}


void 
MessageTest::tearDown()
{
  delete m_message;
}


void 
MessageTest::testDefaultConstructor()
{
  std::string empty;
  CPPUNIT_ASSERT_EQUAL( empty, m_message->shortDescription() );
  CPPUNIT_ASSERT_EQUAL( 0, m_message->detailCount() );
}


void 
MessageTest::testDetailAtThrowIfBadIndex()
{
  m_message->detailAt( -1 );
}



void 
MessageTest::testDetailAtThrowIfBadIndex2()
{
  m_message->detailAt( 0 );
}


void 
MessageTest::testAddDetail()
{
  std::string expected( "first" );
  m_message->addDetail( expected );
  CPPUNIT_ASSERT_EQUAL( 1, m_message->detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected, m_message->detailAt(0) );
}


void 
MessageTest::testAddDetail2()
{
  std::string expected1( "first" );
  std::string expected2( "second" );
  m_message->addDetail( expected1, expected2 );
  CPPUNIT_ASSERT_EQUAL( 2, m_message->detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, m_message->detailAt(0) );
  CPPUNIT_ASSERT_EQUAL( expected2, m_message->detailAt(1) );
}


void 
MessageTest::testAddDetail3()
{
  std::string expected1( "first" );
  std::string expected2( "second" );
  std::string expected3( "third" );
  m_message->addDetail( expected1, expected2, expected3 );
  CPPUNIT_ASSERT_EQUAL( 3, m_message->detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, m_message->detailAt(0) );
  CPPUNIT_ASSERT_EQUAL( expected2, m_message->detailAt(1) );
  CPPUNIT_ASSERT_EQUAL( expected3, m_message->detailAt(2) );
}


void 
MessageTest::testAddDetailEmptyMessage()
{
  m_message->addDetail( CPPUNIT_NS::Message() );
  CPPUNIT_ASSERT_EQUAL( 0, m_message->detailCount() );
}


void 
MessageTest::testAddDetailMessage()
{
  std::string expected1( "first" );
  std::string expected2( "second" );
  m_message->addDetail( CPPUNIT_NS::Message( "shortDesc", expected1, expected2 ) );
  CPPUNIT_ASSERT_EQUAL( 2, m_message->detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, m_message->detailAt(0) );
  CPPUNIT_ASSERT_EQUAL( expected2, m_message->detailAt(1) );
}


void 
MessageTest::testSetShortDescription()
{
  std::string expected( "shortDesc" );
  m_message->setShortDescription( expected );
  CPPUNIT_ASSERT_EQUAL( expected, m_message->shortDescription() );
}


void 
MessageTest::testClearDetails()
{
  m_message->addDetail( "detail1" );
  m_message->clearDetails();
  CPPUNIT_ASSERT_EQUAL( 0, m_message->detailCount() );
}


void 
MessageTest::testConstructor()
{
  std::string expected( "short" );
  CPPUNIT_NS::Message message( expected );
  
  CPPUNIT_ASSERT_EQUAL( expected, message.shortDescription() );
  CPPUNIT_ASSERT_EQUAL( 0, message.detailCount() );
}


void 
MessageTest::testConstructorDetail1()
{
  std::string expected( "short" );
  std::string expected1( "detail-1" );
  CPPUNIT_NS::Message message( expected, expected1 );
  
  CPPUNIT_ASSERT_EQUAL( expected, message.shortDescription() );
  CPPUNIT_ASSERT_EQUAL( 1, message.detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, message.detailAt(0) );
}


void 
MessageTest::testConstructorDetail2()
{
  std::string expected( "short" );
  std::string expected1( "detail-1" );
  std::string expected2( "detail-2" );
  CPPUNIT_NS::Message message( expected, expected1, expected2 );
  
  CPPUNIT_ASSERT_EQUAL( expected, message.shortDescription() );
  CPPUNIT_ASSERT_EQUAL( 2, message.detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, message.detailAt(0) );
  CPPUNIT_ASSERT_EQUAL( expected2, message.detailAt(1) );
}


void 
MessageTest::testConstructorDetail3()
{
  std::string expected( "short" );
  std::string expected1( "detail-1" );
  std::string expected2( "detail-2" );
  std::string expected3( "detail-3" );
  CPPUNIT_NS::Message message( expected, expected1, expected2, expected3 );
  
  CPPUNIT_ASSERT_EQUAL( expected, message.shortDescription() );
  CPPUNIT_ASSERT_EQUAL( 3, message.detailCount() );
  CPPUNIT_ASSERT_EQUAL( expected1, message.detailAt(0) );
  CPPUNIT_ASSERT_EQUAL( expected2, message.detailAt(1) );
  CPPUNIT_ASSERT_EQUAL( expected3, message.detailAt(2) );
}


void 
MessageTest::testDetailsNone()
{
  CPPUNIT_ASSERT_MESSAGE("012345678901234",true);
  std::string empty;
  CPPUNIT_ASSERT_EQUAL( empty, m_message->details() );
}


void 
MessageTest::testDetailsSome()
{
  m_message->addDetail( "Expected: 1", "Actual:   7", "Info: number" );
  std::string expected( "- Expected: 1\n- Actual:   7\n- Info: number\n" );
  std::string actual = m_message->details();
  CPPUNIT_ASSERT_EQUAL( expected, actual );
}


void 
MessageTest::testEqual()
{
  CPPUNIT_ASSERT( *m_message == CPPUNIT_NS::Message() );
  
  CPPUNIT_NS::Message message1( "short", "det1", "det2", "det3" );
  CPPUNIT_NS::Message message2( message1 );
  CPPUNIT_ASSERT( message1 == message2 );

  CPPUNIT_ASSERT( !(*m_message == message1) );

  CPPUNIT_NS::Message message3( "short" );
  CPPUNIT_ASSERT( !(message3 == message1) );

  CPPUNIT_NS::Message message4( "long" );
  CPPUNIT_ASSERT( !(message3 == message4) );

  CPPUNIT_NS::Message message5( "short", "det1", "det-2", "det3" );
  CPPUNIT_ASSERT( !(message1 == message5) );
}


void 
MessageTest::testNotEqual()
{
  CPPUNIT_NS::Message message1( "short", "det1", "det2", "det3" );
  CPPUNIT_NS::Message message2( "short", "det1", "det-2", "det3" );
  CPPUNIT_ASSERT( message1 != message2 );
  CPPUNIT_ASSERT( !(message1 != message1) );
}
