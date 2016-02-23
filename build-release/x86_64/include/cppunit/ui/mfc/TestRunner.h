#ifndef CPPUNITUI_MFC_TESTRUNNER_H
#define CPPUNITUI_MFC_TESTRUNNER_H

#include <cppunit/ui/mfc/MfcTestRunner.h>

CPPUNIT_NS_BEGIN

#if defined(CPPUNIT_HAVE_NAMESPACES)
namespace MfcUi
{
  /*! Mfc TestRunner (DEPRECATED).
   * \deprecated Use CppUnit::MfcTestRunner instead.
   */
  typedef CPPUNIT_NS::MfcTestRunner TestRunner;
}
#endif // defined(CPPUNIT_HAVE_NAMESPACES)

CPPUNIT_NS_END


#endif  // CPPUNITUI_MFC_TESTRUNNER_H
