#ifndef STRINGTOOLSTEST_H
#define STRINGTOOLSTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/tools/StringTools.h>


/// Unit tests for StringToolsTest
class StringToolsTest : public CPPUNIT_NS::TestCase
{
  CPPUNIT_TEST_SUITE( StringToolsTest );
  CPPUNIT_TEST( testToStringInt );
  CPPUNIT_TEST( testToStringDouble );
  CPPUNIT_TEST( testSplitEmptyString );
  CPPUNIT_TEST( testSplitOneItem );
  CPPUNIT_TEST( testSplitItemEmpty );
  CPPUNIT_TEST( testSplitTwoItem );
  CPPUNIT_TEST( testSplitEmptyTwoItem );
  CPPUNIT_TEST( testSplitEmptyItemEmpty );
  CPPUNIT_TEST( testSplitEmptyItemEmptyEmptyItem );
  CPPUNIT_TEST( testWrapEmpty );
  CPPUNIT_TEST( testWrapNotNeeded );
  CPPUNIT_TEST( testWrapLimitNotNeeded );
  CPPUNIT_TEST( testWrapOneNeeded );
  CPPUNIT_TEST( testWrapTwoNeeded );
  CPPUNIT_TEST( testWrapLimitTwoNeeded );
  CPPUNIT_TEST( testWrapOneNeededTwoNeeded );
  CPPUNIT_TEST( testWrapNotNeededEmptyLinesOneNeeded );
  CPPUNIT_TEST_SUITE_END();

public:
  /*! Constructs a StringToolsTest object.
   */
  StringToolsTest();

  /// Destructor.
  virtual ~StringToolsTest();

  void setUp();
  void tearDown();

  void testToStringInt();
  void testToStringDouble();

  void testSplitEmptyString();
  void testSplitOneItem();
  void testSplitItemEmpty();
  void testSplitTwoItem();
  void testSplitEmptyTwoItem();
  void testSplitEmptyItemEmpty();
  void testSplitEmptyItemEmptyEmptyItem();

  void testWrapEmpty();
  void testWrapNotNeeded();
  void testWrapLimitNotNeeded();
  void testWrapOneNeeded();
  void testWrapTwoNeeded();
  void testWrapLimitTwoNeeded();
  void testWrapOneNeededTwoNeeded();
  void testWrapNotNeededEmptyLinesOneNeeded();

private:
  /// Prevents the use of the copy constructor.
  StringToolsTest( const StringToolsTest &other );

  /// Prevents the use of the copy operator.
  void operator =( const StringToolsTest &other );

private:
};



#endif  // STRINGTOOLSTEST_H
