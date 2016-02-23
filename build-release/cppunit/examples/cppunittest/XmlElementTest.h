#ifndef CPPUNITEST_XMLELEMENTTEST_H
#define CPPUNITEST_XMLELEMENTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <stdexcept>


/*! Unit tests for XmlElement.
 */
class XmlElementTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( XmlElementTest );
  CPPUNIT_TEST( testStringContentConstructor );
  CPPUNIT_TEST( testNumericContentConstructor );
  CPPUNIT_TEST( testSetName );
  CPPUNIT_TEST( testSetStringContent );
  CPPUNIT_TEST( testSetNumericContent );
  CPPUNIT_TEST( testElementCount );
  CPPUNIT_TEST_EXCEPTION( testElementAtNegativeIndexThrow, std::invalid_argument );
  CPPUNIT_TEST_EXCEPTION( testElementAtTooLargeIndexThrow, std::invalid_argument );
  CPPUNIT_TEST( testElementAt );
  CPPUNIT_TEST_EXCEPTION( testElementForThrow, std::invalid_argument );
  CPPUNIT_TEST( testElementFor );

  CPPUNIT_TEST( testEmptyNodeToString );
  CPPUNIT_TEST( testElementWithAttributesToString );
  CPPUNIT_TEST( testEscapedAttributeValueToString );
  CPPUNIT_TEST( testElementToStringEscapeContent );
  CPPUNIT_TEST( testElementWithChildrenToString );
  CPPUNIT_TEST( testElementWithContentToString );
  CPPUNIT_TEST( testElementWithNumericContentToString );
  CPPUNIT_TEST( testElementWithContentAndChildToString );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a XmlElementTest object.
   */
  XmlElementTest();

  /// Destructor.
  virtual ~XmlElementTest();

  void setUp();
  void tearDown();

  void testStringContentConstructor();
  void testNumericContentConstructor();
  void testSetName();
  void testSetStringContent();
  void testSetNumericContent();
  void testElementCount();
  void testElementAtNegativeIndexThrow();
  void testElementAtTooLargeIndexThrow();
  void testElementAt();
  void testElementForThrow();
  void testElementFor();

  void testEmptyNodeToString();
  void testElementWithAttributesToString();
  void testEscapedAttributeValueToString();
  void testElementToStringEscapeContent();
  void testElementWithChildrenToString();
  void testElementWithContentToString();
  void testElementWithNumericContentToString();
  void testElementWithContentAndChildToString();

private:
  /// Prevents the use of the copy constructor.
  XmlElementTest( const XmlElementTest &copy );

  /// Prevents the use of the copy operator.
  void operator =( const XmlElementTest &copy );
};



#endif  // CPPUNITEST_XMLELEMENTTEST_H
