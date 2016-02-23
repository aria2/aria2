#ifndef CPPUNIT_PROTECTORCONTEXT_H
#define CPPUNIT_PROTECTORCONTEXT_H

#include <cppunit/Portability.h>
#include <string>

CPPUNIT_NS_BEGIN

class Test;
class TestResult;


/*! \brief Protector context (Implementation).
 * Implementation detail.
 * \internal Context use to report failure in Protector.
 */
class CPPUNIT_API ProtectorContext
{
public:
  ProtectorContext( Test *test,
                    TestResult *result,
                    const std::string &shortDescription )
      : m_test( test )
      , m_result( result )
      , m_shortDescription( shortDescription )
  {
  }

  Test *m_test;
  TestResult *m_result;
  std::string m_shortDescription;
};


CPPUNIT_NS_END

#endif // CPPUNIT_PROTECTORCONTEXT_H

