#include "Exception.h"
#include "DownloadFailureException.h"
#include "Util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ExceptionTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ExceptionTest);
  CPPUNIT_TEST(testStackTrace);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testStackTrace();
};


CPPUNIT_TEST_SUITE_REGISTRATION(ExceptionTest);

void ExceptionTest::testStackTrace()
{
  DownloadFailureException c1("cause1");
  DownloadFailureException c2("cause2", c1);
  DownloadFailureException e("exception thrown", c2);

  CPPUNIT_ASSERT_EQUAL(std::string("Exception: exception thrown\n"
				   "  -> cause2\n"
				   "  -> cause1\n"),
		       e.stackTrace());
}

} // namespace aria2
