#ifndef MESSAGETEST_H
#define MESSAGETEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Message.h>
#include <stdexcept>


/// Unit tests for MessageTest
class MessageTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( MessageTest );
  CPPUNIT_TEST( testDefaultConstructor );
  CPPUNIT_TEST_EXCEPTION( testDetailAtThrowIfBadIndex, std::invalid_argument );
  CPPUNIT_TEST_EXCEPTION( testDetailAtThrowIfBadIndex2, std::invalid_argument );
  CPPUNIT_TEST( testAddDetail );
  CPPUNIT_TEST( testAddDetail2 );
  CPPUNIT_TEST( testAddDetail3 );
  CPPUNIT_TEST( testAddDetailEmptyMessage );
  CPPUNIT_TEST( testAddDetailMessage );
  CPPUNIT_TEST( testSetShortDescription );
  CPPUNIT_TEST( testClearDetails );
  CPPUNIT_TEST( testConstructor );
  CPPUNIT_TEST( testConstructorDetail1 );
  CPPUNIT_TEST( testConstructorDetail2 );
  CPPUNIT_TEST( testConstructorDetail3 );
  CPPUNIT_TEST( testDetailsNone );
  CPPUNIT_TEST( testDetailsSome );
  CPPUNIT_TEST( testEqual );
  CPPUNIT_TEST( testNotEqual );
  CPPUNIT_TEST_SUITE_END();

public:
  MessageTest();

  virtual ~MessageTest();

  void setUp();
  void tearDown();

  void testDefaultConstructor();
  void testDetailAtThrowIfBadIndex();
  void testDetailAtThrowIfBadIndex2();
  void testAddDetail();
  void testAddDetail2();
  void testAddDetail3();
  void testAddDetailEmptyMessage();
  void testAddDetailMessage();
  void testSetShortDescription();
  void testClearDetails();

  void testConstructor();
  void testConstructorDetail1();
  void testConstructorDetail2();
  void testConstructorDetail3();

  void testDetailsNone();
  void testDetailsSome();

  void testEqual();
  void testNotEqual();

private:
  /// Prevents the use of the copy constructor.
  MessageTest( const MessageTest &other );

  /// Prevents the use of the copy operator.
  void operator =( const MessageTest &other );

private:
  CPPUNIT_NS::Message *m_message;
};



#endif  // MESSAGETEST_H
