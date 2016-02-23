#include <cppunit/tools/StringTools.h>
#include <cppunit/tools/XmlElement.h>
#include <stdexcept>


CPPUNIT_NS_BEGIN

  
XmlElement::XmlElement( std::string elementName,
                        std::string content ) 
  : m_name( elementName )
  , m_content( content )
{
}

    
XmlElement::XmlElement( std::string elementName,
                        int numericContent )
  : m_name( elementName )
{
  setContent( numericContent );
}


XmlElement::~XmlElement()
{
  Elements::iterator itNode = m_elements.begin();
  while ( itNode != m_elements.end() )
  {
    XmlElement *element = *itNode++;
    delete element;
  }
}


std::string 
XmlElement::name() const
{
  return m_name;
}


std::string 
XmlElement::content() const
{
  return m_content;
}


void 
XmlElement::setName( const std::string &name )
{
  m_name = name;
}


void 
XmlElement::setContent( const std::string &content )
{
  m_content = content;
}


void 
XmlElement::setContent( int numericContent )
{
  m_content = StringTools::toString( numericContent );
}


void 
XmlElement::addAttribute( std::string attributeName,
                          std::string value  )
{
  m_attributes.push_back( Attribute( attributeName, value ) );
}


void 
XmlElement::addAttribute( std::string attributeName,
                          int numericValue )
{
  addAttribute( attributeName, StringTools::toString( numericValue ) );
}


void 
XmlElement::addElement( XmlElement *node )
{
  m_elements.push_back( node );
}


int 
XmlElement::elementCount() const
{
  return m_elements.size();
}


XmlElement *
XmlElement::elementAt( int index ) const
{
  if ( index < 0  ||  index >= elementCount() )
    throw std::invalid_argument( "XmlElement::elementAt(), out of range index" );

  return m_elements[ index ];
}


XmlElement *
XmlElement::elementFor( const std::string &name ) const
{
  Elements::const_iterator itElement = m_elements.begin();
  for ( ; itElement != m_elements.end(); ++itElement )
  {
    if ( (*itElement)->name() == name )
      return *itElement;
  }

  throw std::invalid_argument( "XmlElement::elementFor(), not matching child element found" );
  return NULL;  // make some compilers happy.
}


std::string 
XmlElement::toString( const std::string &indent ) const
{
  std::string element( indent );
  element += "<";
  element += m_name;
  if ( !m_attributes.empty() )
  {
    element += " ";
    element += attributesAsString();
  }
  element += ">";

  if ( !m_elements.empty() )
  {
    element += "\n";

    std::string subNodeIndent( indent + "  " );
    Elements::const_iterator itNode = m_elements.begin();
    while ( itNode != m_elements.end() )
    {
      const XmlElement *node = *itNode++;
      element += node->toString( subNodeIndent );
    }

    element += indent;
  }

  if ( !m_content.empty() )
  {
    element += escape( m_content );
    if ( !m_elements.empty() )
    {
      element += "\n";
      element += indent;
    }
  }

  element += "</";
  element += m_name;
  element += ">\n";

  return element;
}


std::string 
XmlElement::attributesAsString() const
{
  std::string attributes;
  Attributes::const_iterator itAttribute = m_attributes.begin();
  while ( itAttribute != m_attributes.end() )
  {
    if ( !attributes.empty() )
       attributes += " ";

    const Attribute &attribute = *itAttribute++;
    attributes += attribute.first;
    attributes += "=\"";
    attributes += escape( attribute.second );
    attributes += "\"";
  }
  return attributes;
}


std::string 
XmlElement::escape( std::string value ) const
{
  std::string escaped;
  for ( unsigned int index =0; index < value.length(); ++index )
  {
    char c = value[index ];
    switch ( c )    // escape all predefined XML entity (safe?)
    {
    case '<': 
      escaped += "&lt;";
      break;
    case '>': 
      escaped += "&gt;";
      break;
    case '&': 
      escaped += "&amp;";
      break;
    case '\'': 
      escaped += "&apos;";
      break;
    case '"': 
      escaped += "&quot;";
      break;
    default:
      escaped += c;
    }
  }
  
  return escaped;
}


CPPUNIT_NS_END

