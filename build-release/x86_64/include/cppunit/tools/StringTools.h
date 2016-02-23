#ifndef CPPUNIT_TOOLS_STRINGTOOLS_H
#define CPPUNIT_TOOLS_STRINGTOOLS_H

#include <cppunit/Portability.h>
#include <string>
#include <cppunit/portability/CppUnitVector.h>


CPPUNIT_NS_BEGIN


/*! \brief Tool functions to manipulate string.
 */
struct StringTools
{

  typedef CppUnitVector<std::string> Strings;

  static std::string CPPUNIT_API toString( int value );

  static std::string CPPUNIT_API toString( double value );

  static Strings CPPUNIT_API split( const std::string &text, 
                                    char separator );

  static std::string CPPUNIT_API wrap( const std::string &text,
                                       int wrapColumn = CPPUNIT_WRAP_COLUMN );

};


CPPUNIT_NS_END

#endif  // CPPUNIT_TOOLS_STRINGTOOLS_H
