#ifndef XMLUNIFORMISERTEST_H
#define XMLUNIFORMISERTEST_H

#include <cppunit/extensions/HelperMacros.h>


/*! \class XmlUniformiserTest
 * \brief Unit test for XmlUniformiser.
 */
class XmlUniformiserTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( XmlUniformiserTest );
  CPPUNIT_TEST( testEmpty );
  CPPUNIT_TEST( testSkipProcessed );
  CPPUNIT_TEST( testOpenElementWithoutAttributeButSomeSpaces );
  CPPUNIT_TEST( testOpenCloseElement );
  CPPUNIT_TEST( testElementWithEmptyAttribute );
  CPPUNIT_TEST( testElementWithEmptyAttributeButSomeSpaces );
  CPPUNIT_TEST( testElementWithOneAttribute );
  CPPUNIT_TEST( testElementWithThreeAttributes );
  CPPUNIT_TEST( testSkipComment );
  CPPUNIT_TEST( testElementWithContent );
  CPPUNIT_TEST( testElementsHierarchyWithContents );
  CPPUNIT_TEST( testAssertXmlEqual );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a XmlUniformiserTest object.
   */
  XmlUniformiserTest();

  /// Destructor.
  virtual ~XmlUniformiserTest();

  void setUp();
  void tearDown();

  void testEmpty();
  void testSkipProcessed();
  void testOpenElementWithoutAttributeButSomeSpaces();
  void testOpenCloseElement();
  void testElementWithEmptyAttribute();
  void testElementWithEmptyAttributeButSomeSpaces();
  void testElementWithOneAttribute();
  void testElementWithThreeAttributes();
  void testSkipComment();
  void testElementWithContent();
  void testElementsHierarchyWithContents();

  void testAssertXmlEqual();

private:
  void check( const std::string &xml, 
              const std::string &expectedStrippedXml );

  /// Prevents the use of the copy constructor.
  XmlUniformiserTest( const XmlUniformiserTest &copy );

  /// Prevents the use of the copy operator.
  void operator =( const XmlUniformiserTest &copy );

private:
};



#endif  // XMLUNIFORMISERTEST_H
