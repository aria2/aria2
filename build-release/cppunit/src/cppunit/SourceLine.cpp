#include <cppunit/SourceLine.h>


CPPUNIT_NS_BEGIN


SourceLine::SourceLine() :
    m_lineNumber( -1 )
{
}


SourceLine::SourceLine( const SourceLine &other )
   : m_fileName( other.m_fileName.c_str() )
   , m_lineNumber( other.m_lineNumber )
{
}


SourceLine::SourceLine( const std::string &fileName,
                        int lineNumber )
   : m_fileName( fileName.c_str() )
   , m_lineNumber( lineNumber )
{
}


SourceLine &
SourceLine::operator =( const SourceLine &other )
{
   if ( this != &other )
   {
      m_fileName = other.m_fileName.c_str();
      m_lineNumber = other.m_lineNumber;
   }
   return *this;
}


SourceLine::~SourceLine()
{
}


bool 
SourceLine::isValid() const
{
  return !m_fileName.empty();
}


int 
SourceLine::lineNumber() const
{
  return m_lineNumber;
}


std::string 
SourceLine::fileName() const
{
  return m_fileName;
}


bool 
SourceLine::operator ==( const SourceLine &other ) const
{
  return m_fileName == other.m_fileName  &&
         m_lineNumber == other.m_lineNumber;
}


bool 
SourceLine::operator !=( const SourceLine &other ) const
{
  return !( *this == other );
}


CPPUNIT_NS_END
