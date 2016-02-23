#include "CoreSuite.h"
#include "ExceptionTest.h"
#include <cppunit/Exception.h>
#include <memory>


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ExceptionTest,
                                       coreSuiteName() );


ExceptionTest::ExceptionTest()
{
}


ExceptionTest::~ExceptionTest()
{
}


void 
ExceptionTest::setUp()
{
}


void 
ExceptionTest::tearDown()
{
}


void 
ExceptionTest::testConstructor()
{
  const CPPUNIT_NS::Message message( "a message" );
  const CPPUNIT_NS::SourceLine sourceLine( "dir/afile.cpp", 17 );
  
  CPPUNIT_NS::Exception e( message, sourceLine );

  CPPUNIT_ASSERT_EQUAL( message.shortDescription(), e.message().shortDescription() );
  CPPUNIT_ASSERT( sourceLine == e.sourceLine() );
}


void 
ExceptionTest::testDefaultConstructor()
{
  CPPUNIT_NS::Exception e;

  CPPUNIT_ASSERT( CPPUNIT_NS::Message() == e.message() );
  CPPUNIT_ASSERT( !e.sourceLine().isValid() );
}


void 
ExceptionTest::testCopyConstructor()
{
  CPPUNIT_NS::SourceLine sourceLine( "fileName.cpp", 123 );
  CPPUNIT_NS::Exception e( CPPUNIT_NS::Message("message"), sourceLine  );
  CPPUNIT_NS::Exception other( e );
  checkIsSame( e, other );
}


void 
ExceptionTest::testAssignment()
{
  CPPUNIT_NS::SourceLine sourceLine( "fileName.cpp", 123 );
  CPPUNIT_NS::Exception e( CPPUNIT_NS::Message("message"), sourceLine  );
  CPPUNIT_NS::Exception other;
  other = e;
  checkIsSame( e, other );
}


void 
ExceptionTest::testClone()
{
  CPPUNIT_NS::SourceLine sourceLine( "fileName.cpp", 123 );
  CPPUNIT_NS::Exception e( CPPUNIT_NS::Message("message"), sourceLine  );
  std::auto_ptr<CPPUNIT_NS::Exception> other( e.clone() );
  checkIsSame( e, *other.get() );
}


void 
ExceptionTest::checkIsSame( CPPUNIT_NS::Exception &e, 
                            CPPUNIT_NS::Exception &other )
{
  std::string eWhat( e.what() );
  std::string otherWhat( other.what() );
  CPPUNIT_ASSERT_EQUAL( eWhat, otherWhat );
  CPPUNIT_ASSERT( e.sourceLine() == other.sourceLine() );
}
