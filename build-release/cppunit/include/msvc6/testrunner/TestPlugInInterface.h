#ifndef CPPUNIT_TESTPLUGINRUNNER_TESTPLUGININTERFACE_H
#define CPPUNIT_TESTPLUGINRUNNER_TESTPLUGININTERFACE_H

#include <cppunit/Test.h>
#include <cppunit/TestSuite.h>

#if !defined(WINAPI)
#define WIN32_LEAN_AND_MEAN 
#define NOGDI
#define NOUSER
#define NOKERNEL
#define NOSOUND
#define NOMINMAX
#include <windows.h>
#endif

/*! \brief Abstract TestPlugIn for DLL.
 * \deprecated Use CppUnitTestPlugIn instead.
 *
 * A Test plug-in DLL must subclass this class and "publish" an instance
 * using the following exported function:
 * \code
 * extern "C" {
 *   __declspec(dllimport) TestPlugInInterface *GetTestPlugInInterface();
 * }
 * \endcode
 *
 * When loading the DLL, the TestPlugIn runner look-up this function and
 * retreives the 
 *
 * See the TestPlugIn example for VC++ for details.
 */
class TestPlugInInterface
{
public:
  virtual ~TestPlugInInterface() {}

  /*! Returns an instance of the "All Tests" suite.
   *
   * \return Instance of the top-level suite that contains all test. Ownership
   *         is granted to the method caller.
   */
  virtual CppUnit::Test *makeTest() =0;
};

typedef TestPlugInInterface* (WINAPI *GetTestPlugInInterfaceFunction)(void);


extern "C" {
  __declspec(dllexport) TestPlugInInterface *GetTestPlugInInterface();
}



#endif // CPPUNIT_TESTPLUGINRUNNER_TESTPLUGININTERFACE_H
