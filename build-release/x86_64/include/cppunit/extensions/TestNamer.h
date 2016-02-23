#ifndef CPPUNIT_EXTENSIONS_TESTNAMER_H
#define CPPUNIT_EXTENSIONS_TESTNAMER_H

#include <cppunit/Portability.h>
#include <string>

#if CPPUNIT_HAVE_RTTI
#  include <typeinfo>
#endif



/*! \def CPPUNIT_TESTNAMER_DECL( variableName, FixtureType )
 * \brief Declares a TestNamer.
 *
 * Declares a TestNamer for the specified type, using RTTI if enabled, otherwise
 * using macro string expansion.
 *
 * RTTI is used if CPPUNIT_USE_TYPEINFO_NAME is defined and not null.
 *
 * \code
 * void someMethod() 
 * {
 *   CPPUNIT_TESTNAMER_DECL( namer, AFixtureType );
 *   std::string fixtureName = namer.getFixtureName();
 *   ...
 * \endcode
 *
 * \relates TestNamer
 * \see TestNamer
 */
#if CPPUNIT_USE_TYPEINFO_NAME
#  define CPPUNIT_TESTNAMER_DECL( variableName, FixtureType )       \
              CPPUNIT_NS::TestNamer variableName( typeid(FixtureType) )
#else
#  define CPPUNIT_TESTNAMER_DECL( variableName, FixtureType )       \
              CPPUNIT_NS::TestNamer variableName( std::string(#FixtureType) )
#endif



CPPUNIT_NS_BEGIN


/*! \brief Names a test or a fixture suite.
 *
 * TestNamer is usually instantiated using CPPUNIT_TESTNAMER_DECL.
 *
 */
class CPPUNIT_API TestNamer
{
public:
#if CPPUNIT_HAVE_RTTI
  /*! \brief Constructs a namer using the fixture's type-info.
   * \param typeInfo Type-info of the fixture type. Use to name the fixture suite.
   */
  TestNamer( const std::type_info &typeInfo );
#endif

  /*! \brief Constructs a namer using the specified fixture name.
   * \param fixtureName Name of the fixture suite. Usually extracted using a macro.
   */
  TestNamer( const std::string &fixtureName );

  virtual ~TestNamer();

  /*! \brief Returns the name of the fixture.
   * \return Name of the fixture.
   */
  virtual std::string getFixtureName() const;

  /*! \brief Returns the name of the test for the specified method.
   * \param testMethodName Name of the method that implements a test.
   * \return A string that is the concatenation of the test fixture name 
   *         (returned by getFixtureName()) and\a testMethodName, 
   *         separated using '::'. This provides a fairly unique name for a given
   *         test.
   */
  virtual std::string getTestNameFor( const std::string &testMethodName ) const;

protected:
  std::string m_fixtureName;
};


CPPUNIT_NS_END

#endif // CPPUNIT_EXTENSIONS_TESTNAMER_H

