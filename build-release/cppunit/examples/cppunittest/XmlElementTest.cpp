#include <cppunit/config/SourcePrefix.h>
#include <cppunit/tools/XmlElement.h>
#include "ToolsSuite.h"
#include "XmlElementTest.h"
#include "XmlUniformiser.h"


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( XmlElementTest, 
                                       toolsSuiteName() );


XmlElementTest::XmlElementTest()
{
}


XmlElementTest::~XmlElementTest()
{
}


void 
XmlElementTest::setUp()
{
}


void 
XmlElementTest::tearDown()
{
}


void 
XmlElementTest::testStringContentConstructor()
{
  CPPUNIT_NS::XmlElement element( "aName", "someContent" );
  CPPUNIT_ASSERT_EQUAL( std::string("aName"), element.name() );
  CPPUNIT_ASSERT_EQUAL( std::string("someContent"), element.content() );
}


void 
XmlElementTest::testNumericContentConstructor()
{
  CPPUNIT_NS::XmlElement element( "numericName", 123456789 );
  CPPUNIT_ASSERT_EQUAL( std::string("numericName"), element.name() );
  CPPUNIT_ASSERT_EQUAL( std::string("123456789"), element.content() );
}


void 
XmlElementTest::testSetName()
{
  CPPUNIT_NS::XmlElement element( "aName" );
  element.setName( "anotherName" );
  CPPUNIT_ASSERT_EQUAL( std::string("anotherName"), element.name() );
}


void 
XmlElementTest::testSetStringContent()
{
  CPPUNIT_NS::XmlElement element( "aName", "someContent" );
  element.setContent( "other" );
  CPPUNIT_ASSERT_EQUAL( std::string("other"), element.content() );
}


void 
XmlElementTest::testSetNumericContent()
{
  CPPUNIT_NS::XmlElement element( "aName", "someContent" );
  element.setContent( 87654321 );
  CPPUNIT_ASSERT_EQUAL( std::string("87654321"), element.content() );
}


void 
XmlElementTest::testElementCount()
{
  CPPUNIT_NS::XmlElement node( "element", "content" );
  CPPUNIT_ASSERT_EQUAL( 0, node.elementCount() );

  node.addElement( new CPPUNIT_NS::XmlElement( "child1" ) );
  node.addElement( new CPPUNIT_NS::XmlElement( "child2" ) );
  CPPUNIT_ASSERT_EQUAL( 2, node.elementCount() );
}


void 
XmlElementTest::testElementAtNegativeIndexThrow()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.elementAt( -1 );
}


void 
XmlElementTest::testElementAtTooLargeIndexThrow()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.elementAt( 0 );
}


void 
XmlElementTest::testElementAt()
{
  CPPUNIT_NS::XmlElement node( "element" );
  CPPUNIT_NS::XmlElement *element1 = new CPPUNIT_NS::XmlElement( "element1" );
  CPPUNIT_NS::XmlElement *element2 = new CPPUNIT_NS::XmlElement( "element2" );
  node.addElement( element1 );
  node.addElement( element2 );

  CPPUNIT_ASSERT( element1 == node.elementAt(0) );
  CPPUNIT_ASSERT( element2 == node.elementAt(1) );
}


void 
XmlElementTest::testElementForThrow()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.addElement( new CPPUNIT_NS::XmlElement( "element1" ) );
  node.elementFor( "name2" );
}


void 
XmlElementTest::testElementFor()
{
  CPPUNIT_NS::XmlElement node( "element" );
  CPPUNIT_NS::XmlElement *element1 = new CPPUNIT_NS::XmlElement( "element1" );
  CPPUNIT_NS::XmlElement *element2 = new CPPUNIT_NS::XmlElement( "element2" );
  node.addElement( element1 );
  node.addElement( element2 );

  CPPUNIT_ASSERT( element2 == node.elementFor( "element2" ) );
  CPPUNIT_ASSERT( element1 == node.elementFor( "element1" ) );
}


void 
XmlElementTest::testEmptyNodeToString()
{
  CPPUNIT_NS::XmlElement node( "element" );
  std::string expectedXml = "<element></element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementWithAttributesToString()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.addAttribute( "id", 17 );
  node.addAttribute( "date-format", "iso-8901" );
  std::string expectedXml = "<element id=\"17\" "
                            "date-format=\"iso-8901\">"
                            "</element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testEscapedAttributeValueToString()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.addAttribute( "escaped", "&<>\"'" );
  std::string expectedXml = "<element escaped=\""
                            "&amp;&lt;&gt;&quot;&apos;"
                            "\"></element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementToStringEscapeContent()
{
  CPPUNIT_NS::XmlElement node( "element", "ChessTest<class Chess>" );
  std::string expectedXml = "<element>"
                            "ChessTest&lt;class Chess&gt;"
                            "</element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementWithChildrenToString()
{
  CPPUNIT_NS::XmlElement node( "element" );
  node.addElement( new CPPUNIT_NS::XmlElement( "child1" ) );
  node.addElement( new CPPUNIT_NS::XmlElement( "child2" ) );
  std::string expectedXml = "<element><child1></child1>"
                            "<child2></child2></element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementWithContentToString()
{
  CPPUNIT_NS::XmlElement node( "element", "content\nline2" );
  std::string expectedXml = "<element>content\nline2</element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementWithNumericContentToString()
{
  CPPUNIT_NS::XmlElement node( "element", 123456789 );
  std::string expectedXml = "<element>123456789</element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}


void 
XmlElementTest::testElementWithContentAndChildToString()
{
  CPPUNIT_NS::XmlElement node( "element", "content" );
  node.addElement( new CPPUNIT_NS::XmlElement( "child1" ) );
  std::string expectedXml = "<element><child1></child1>content</element>";
  CPPUNITTEST_ASSERT_XML_EQUAL( expectedXml, node.toString() );
}
