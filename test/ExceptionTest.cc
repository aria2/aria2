#include "Exception.h"

#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

#include "DownloadFailureException.h"

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
  DownloadFailureException c1 = DOWNLOAD_FAILURE_EXCEPTION("cause1");
  DownloadFailureException c2 = DOWNLOAD_FAILURE_EXCEPTION2("cause2", c1);
  DownloadFailureException e = DOWNLOAD_FAILURE_EXCEPTION2("exception thrown",
							   c2);

  CPPUNIT_ASSERT_EQUAL
    (std::string("Exception: [ExceptionTest.cc:31] exception thrown\n"
		 "  -> [ExceptionTest.cc:29] cause2\n"
		 "  -> [ExceptionTest.cc:28] cause1\n"),
     e.stackTrace());
}

} // namespace aria2
