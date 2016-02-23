#include <cppunit/Exception.h>


CPPUNIT_NS_BEGIN


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
/*!
 * \deprecated Use SourceLine::isValid() instead.
 */
const std::string Exception::UNKNOWNFILENAME = "<unknown>";

/*!
 * \deprecated Use SourceLine::isValid() instead.
 */
const long Exception::UNKNOWNLINENUMBER = -1;
#endif


Exception::Exception( const Exception &other )
   : std::exception( other )
{ 
  m_message = other.m_message; 
  m_sourceLine = other.m_sourceLine;
} 


Exception::Exception( const Message &message, 
                      const SourceLine &sourceLine )
    : m_message( message )
    , m_sourceLine( sourceLine )
{
}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
Exception::Exception( std::string message, 
                      long lineNumber, 
                      std::string fileName )
    : m_message( message )
    , m_sourceLine( fileName, lineNumber )
{
}
#endif


Exception::~Exception() throw()
{
}


Exception & 
Exception::operator =( const Exception& other )
{ 
// Don't call superclass operator =(). VC++ STL implementation
// has a bug. It calls the destructor and copy constructor of 
// std::exception() which reset the virtual table to std::exception.
//  SuperClass::operator =(other);

  if ( &other != this )
  {
    m_message = other.m_message; 
    m_sourceLine = other.m_sourceLine;
  }

  return *this; 
}


const char*
Exception::what() const throw()
{
  Exception *mutableThis = CPPUNIT_CONST_CAST( Exception *, this );
  mutableThis->m_whatMessage = m_message.shortDescription() + "\n" + 
                               m_message.details();
  return m_whatMessage.c_str();
}


SourceLine 
Exception::sourceLine() const
{
  return m_sourceLine;
}


Message 
Exception::message() const
{
  return m_message;
}


void 
Exception::setMessage( const Message &message )
{
  m_message = message;
}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
long 
Exception::lineNumber() const
{ 
  return m_sourceLine.isValid() ? m_sourceLine.lineNumber() : 
                                  UNKNOWNLINENUMBER; 
}


std::string 
Exception::fileName() const
{ 
  return m_sourceLine.isValid() ? m_sourceLine.fileName() : 
                                  UNKNOWNFILENAME;
}
#endif


Exception *
Exception::clone() const
{
  return new Exception( *this );
}


CPPUNIT_NS_END
