#ifndef CPPUNIT_PLUGIN_PLUGINMANAGER_H
#define CPPUNIT_PLUGIN_PLUGINMANAGER_H

#include <cppunit/Portability.h>

#if !defined(CPPUNIT_NO_TESTPLUGIN)

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/plugin/PlugInParameters.h>
struct CppUnitTestPlugIn;

CPPUNIT_NS_BEGIN


class DynamicLibraryManager;
class TestResult;
class XmlOutputter;


/*! \brief Manges TestPlugIn.
 */
class CPPUNIT_API PlugInManager
{
public:
  /*! Constructs a PlugInManager object.
   */
  PlugInManager();

  /// Destructor.
  virtual ~PlugInManager();

  /*! \brief Loads the specified plug-in.
   *
   * After being loaded, the CppUnitTestPlugIn::initialize() is called.
   *
   * \param libraryFileName Name of the file that contains the TestPlugIn.
   * \param parameters List of string passed to the plug-in.
   * \return Pointer on the DynamicLibraryManager associated to the library.
   *         Valid until the library is unloaded. Never \c NULL.
   * \exception DynamicLibraryManagerException is thrown if an error occurs during loading.
   */
  void load( const std::string &libraryFileName,
             const PlugInParameters &parameters = PlugInParameters() );

  /*! \brief Unloads the specified plug-in.
   * \param libraryFileName Name of the file that contains the TestPlugIn passed
   *                        to a previous call to load().
   */
  void unload( const std::string &libraryFileName );

  /*! \brief Gives a chance to each loaded plug-in to register TestListener.
   *
   * For each plug-in, call CppUnitTestPlugIn::addListener().
   */
  void addListener( TestResult *eventManager );

  /*! \brief Gives a chance to each loaded plug-in to unregister TestListener.
   * For each plug-in, call CppUnitTestPlugIn::removeListener().
   */
  void removeListener( TestResult *eventManager );

  /*! \brief Provides a way for the plug-in to register some XmlOutputterHook.
   */
  void addXmlOutputterHooks( XmlOutputter *outputter );

  /*! \brief Called when the XmlOutputter is destroyed.
   * 
   * Can be used to free some resources allocated by addXmlOutputterHooks().
   */
  void removeXmlOutputterHooks();

protected:
  /*! \brief (INTERNAL) Information about a specific plug-in.
   */
  struct PlugInInfo
  {
    std::string m_fileName;
    DynamicLibraryManager *m_manager;
    CppUnitTestPlugIn *m_interface;
  };

  /*! Unloads the specified plug-in.
   * \param plugIn Information about the plug-in.
   */
  void unload( PlugInInfo &plugIn );

private:
  /// Prevents the use of the copy constructor.
  PlugInManager( const PlugInManager &copy );

  /// Prevents the use of the copy operator.
  void operator =( const PlugInManager &copy );

private:
  typedef CppUnitDeque<PlugInInfo> PlugIns;
  PlugIns m_plugIns;
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif

#endif // !defined(CPPUNIT_NO_TESTPLUGIN)


#endif  // CPPUNIT_PLUGIN_PLUGINMANAGER_H
