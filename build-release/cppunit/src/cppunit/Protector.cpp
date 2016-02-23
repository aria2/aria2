#include <cppunit/Exception.h>
#include <cppunit/Message.h>
#include <cppunit/Protector.h>
#include <cppunit/TestResult.h>
#include "ProtectorContext.h"
#include <memory>

CPPUNIT_NS_BEGIN

Functor::~Functor()
{
}


Protector::~Protector()
{
}


void 
Protector::reportError( const ProtectorContext &context,
                        const Exception &error ) const
{
  std::auto_ptr<Exception> actualError( error.clone() );
  actualError->setMessage( actualMessage( actualError->message(), context ) );
  context.m_result->addError( context.m_test, 
                              actualError.release() );
}



void 
Protector::reportError( const ProtectorContext &context,
                        const Message &message,
                        const SourceLine &sourceLine ) const
{
  reportError( context, Exception( message, sourceLine ) );
}


void 
Protector::reportFailure( const ProtectorContext &context,
                          const Exception &failure ) const
{
  std::auto_ptr<Exception> actualFailure( failure.clone() );
  actualFailure->setMessage( actualMessage( actualFailure->message(), context ) );
  context.m_result->addFailure( context.m_test, 
                                actualFailure.release() );
}


Message 
Protector::actualMessage( const Message &message,
                          const ProtectorContext &context ) const
{
  Message theActualMessage;
  if ( context.m_shortDescription.empty() )
    theActualMessage = message;
  else
  {
    theActualMessage = Message( context.m_shortDescription, 
                                message.shortDescription() );
    theActualMessage.addDetail( message );
  }

  return theActualMessage;
}




ProtectorGuard::ProtectorGuard( TestResult *result,
                                              Protector *protector )
    : m_result( result )
{
  m_result->pushProtector( protector );
}


ProtectorGuard::~ProtectorGuard()
{
  m_result->popProtector();
}


CPPUNIT_NS_END
