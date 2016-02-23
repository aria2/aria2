#ifndef CPPUNITTEST_XMLUNIFORMISER_H
#define CPPUNITTEST_XMLUNIFORMISER_H

#include <cppunit/SourceLine.h>
#include <cppunit/TestAssert.h>
#include <string>


/*! Uniformise an XML string.
 *
 * Strips spaces between attribut in Element.
 * \warning Attribute values must be double-quoted (att="value").
 * No support for embedded DTD declaration
 */
class XmlUniformiser
{
public:
  XmlUniformiser( const std::string &xml );
  std::string stripped();

private:
  void skipSpaces();
  bool isValidIndex();
  void skipNext( int count =1 );
  void copyNext( int count =1 );
  void skipProcessed();
  void skipComment();
  void copyElement();
  void copyElementContent();
  bool isSpace( char c );
  bool isSpace();
  bool startsWith( std::string expected );
  void copyElementName();
  void copyElementAttributes();
  void copyAttributeName();
  bool isEndOfAttributeName();
  void copyAttributeValue();
  void copyUntilDoubleQuote();
  void removeTrailingSpaces();

private:
  unsigned int m_index;
  std::string m_xml;
  std::string m_stripped;
};




void 
checkXmlEqual( std::string expectedXml,
               std::string actualXml,
               CPPUNIT_NS::SourceLine sourceLine );


/// Asserts that two XML strings are equivalent.
#define CPPUNITTEST_ASSERT_XML_EQUAL( expected, actual ) \
    ::checkXmlEqual( expected, actual,      \
                     CPPUNIT_SOURCELINE() )



#endif  // XMLUNIFORMISER_H
