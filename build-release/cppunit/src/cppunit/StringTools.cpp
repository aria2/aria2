#include <cppunit/tools/StringTools.h>
#include <cppunit/portability/Stream.h>
#include <algorithm>


CPPUNIT_NS_BEGIN


std::string 
StringTools::toString( int value )
{
  OStringStream stream;
  stream << value;
  return stream.str();
}


std::string 
StringTools::toString( double value )
{
  OStringStream stream;
  stream << value;
  return stream.str();
}


StringTools::Strings
StringTools::split( const std::string &text, 
                    char separator )
{
  Strings splittedText;

  std::string::const_iterator itStart = text.begin();
  while ( !text.empty() )
  {
    std::string::const_iterator itSeparator = std::find( itStart, 
                                                         text.end(), 
                                                         separator );
    splittedText.push_back( text.substr( itStart - text.begin(),
                                         itSeparator - itStart ) );
    if ( itSeparator == text.end() )
      break;
    itStart = itSeparator +1;
  }

  return splittedText;
}


std::string 
StringTools::wrap( const std::string &text,
                   int wrapColumn )
{
  const char lineBreak = '\n';
  Strings lines = split( text, lineBreak );

  std::string wrapped;
  for ( Strings::const_iterator it = lines.begin(); it != lines.end(); ++it )
  {
    if ( it != lines.begin() )
      wrapped += lineBreak;

    const std::string &line = *it;
    unsigned int index =0;
    while ( index < line.length() )
    {
      std::string lineSlice( line.substr( index, wrapColumn ) );
      wrapped += lineSlice;
      index += wrapColumn;
      if ( index < line.length() )
        wrapped += lineBreak;
    }
  }

  return wrapped;
}


CPPUNIT_NS_END

