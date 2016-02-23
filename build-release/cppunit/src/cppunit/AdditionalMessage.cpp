#include <cppunit/AdditionalMessage.h>


CPPUNIT_NS_BEGIN


AdditionalMessage::AdditionalMessage()
{
}


AdditionalMessage::AdditionalMessage( const std::string &detail1 )
{
  if ( !detail1.empty() )
    addDetail( detail1 );
}


AdditionalMessage::AdditionalMessage( const char *detail1 )
{
  if ( detail1  &&  !std::string( detail1 ).empty() )
    addDetail( std::string(detail1) );
}


AdditionalMessage::AdditionalMessage( const Message &other )
    : SuperClass( other )
{
}


AdditionalMessage &
AdditionalMessage::operator =( const Message &other )
{
  SuperClass::operator =( other );

  return *this;
}


CPPUNIT_NS_END
