#include <cppunit/config/SourcePrefix.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/XmlElement.h>


CPPUNIT_NS_BEGIN


XmlDocument::XmlDocument( const std::string &encoding,
                          const std::string &styleSheet )
  : m_styleSheet( styleSheet )
  , m_rootElement( new XmlElement( "DummyRoot" ) )
  , m_standalone( true )
{
  setEncoding( encoding );
}


XmlDocument::~XmlDocument()
{
  delete m_rootElement;
}



std::string 
XmlDocument::encoding() const
{
  return m_encoding;
}


void 
XmlDocument::setEncoding( const std::string &encoding )
{
  m_encoding = encoding.empty() ? std::string("ISO-8859-1") : encoding;
}


std::string 
XmlDocument::styleSheet() const
{
  return m_styleSheet;
}


void 
XmlDocument::setStyleSheet( const std::string &styleSheet )
{
  m_styleSheet = styleSheet;
}


bool
XmlDocument::standalone() const
{
  return m_standalone;
}


void
XmlDocument::setStandalone( bool standalone )
{
  m_standalone = standalone;
}


void 
XmlDocument::setRootElement( XmlElement *rootElement )
{
  if ( rootElement == m_rootElement )
    return;

  delete m_rootElement;
  m_rootElement = rootElement;
}


XmlElement &
XmlDocument::rootElement() const
{
  return *m_rootElement;
}


std::string 
XmlDocument::toString() const
{
  std::string asString = "<?xml version=\"1.0\" "
                         "encoding='" + m_encoding + "'";
  if ( m_standalone )
      asString += " standalone='yes'";

  asString += " ?>\n"; 

  if ( !m_styleSheet.empty() )
    asString += "<?xml-stylesheet type=\"text/xsl\" href=\"" + m_styleSheet + "\"?>\n";

  asString += m_rootElement->toString();

  return asString;
}


CPPUNIT_NS_END

