#include <cppunit/Portability.h>
#include <cppunit/extensions/TypeInfoHelper.h>

#if CPPUNIT_HAVE_RTTI

#include <string>

#if CPPUNIT_HAVE_GCC_ABI_DEMANGLE
#include <cxxabi.h>
#endif


CPPUNIT_NS_BEGIN


std::string 
TypeInfoHelper::getClassName( const std::type_info &info )
{
#if defined(CPPUNIT_HAVE_GCC_ABI_DEMANGLE)  &&  CPPUNIT_HAVE_GCC_ABI_DEMANGLE

  int status = 0;
  char* c_name = 0;

  c_name = abi::__cxa_demangle( info.name(), 0, 0, &status );
  
  std::string name( c_name );
  free( c_name );  

#else   // CPPUNIT_HAVE_GCC_ABI_DEMANGLE

  static std::string classPrefix( "class " );
  std::string name( info.name() );

  // Work around gcc 3.0 bug: strip number before type name.
  unsigned int firstNotDigitIndex = 0;
  while ( firstNotDigitIndex < name.length()  &&
          name[firstNotDigitIndex] >= '0'  &&
          name[firstNotDigitIndex] <= '9' )
    ++firstNotDigitIndex;
  name = name.substr( firstNotDigitIndex );

  if ( name.substr( 0, classPrefix.length() ) == classPrefix )
    return name.substr( classPrefix.length() );

#endif  // CPPUNIT_HAVE_GCC_ABI_DEMANGLE

  return name;
}


CPPUNIT_NS_END

#endif // CPPUNIT_HAVE_RTTI
