EXTRA_DIST = CppUnitTestMain.dsw CppUnitTestMain.dsp CppUnitTestPlugIn.dsp CppUnitTestPlugIn.cpp

TESTS = cppunittestmain
check_PROGRAMS = $(TESTS)

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

cppunittestmain_SOURCES = \
	assertion_traitsTest.cpp \
	assertion_traitsTest.h \
	BaseTestCase.cpp \
	BaseTestCase.h \
	CoreSuite.h \
	CppUnitTestMain.cpp \
	CppUnitTestSuite.cpp \
	ExceptionTest.cpp \
	ExceptionTest.h \
  ExceptionTestCaseDecoratorTest.h \
  ExceptionTestCaseDecoratorTest.cpp \
	ExtensionSuite.h \
	FailureException.h \
	HelperMacrosTest.cpp \
	HelperMacrosTest.h \
	HelperSuite.h \
	MessageTest.h \
	MessageTest.cpp \
  MockFunctor.h \
  MockProtector.h \
	MockTestCase.h \
	MockTestCase.cpp \
	MockTestListener.cpp \
	MockTestListener.h \
	OrthodoxTest.cpp \
	OrthodoxTest.h \
	OutputSuite.h \
	RepeatedTestTest.cpp \
	RepeatedTestTest.h \
  StringToolsTest.h \
  StringToolsTest.cpp \
	SubclassedTestCase.cpp \
	SubclassedTestCase.h \
	SynchronizedTestResult.h \
	TestAssertTest.cpp \
	TestAssertTest.h \
	TestCallerTest.cpp \
	TestCallerTest.h \
	TestCaseTest.cpp \
	TestCaseTest.h \
	TestDecoratorTest.cpp \
	TestDecoratorTest.h \
	TestFailureTest.cpp \
	TestFailureTest.h \
	TestPathTest.h \
	TestPathTest.cpp \
	TestResultCollectorTest.cpp \
	TestResultCollectorTest.h \
	TestResultTest.cpp \
	TestResultTest.h \
	TestSetUpTest.cpp \
	TestSetUpTest.h \
	TestSuiteTest.cpp \
	TestSuiteTest.h \
	TestTest.cpp \
	TestTest.h \
  ToolsSuite.h \
	TrackedTestCase.cpp \
	TrackedTestCase.h \
	UnitTestToolSuite.h \
	XmlElementTest.h \
	XmlElementTest.cpp \
	XmlOutputterTest.h \
	XmlOutputterTest.cpp \
	XmlUniformiser.h \
	XmlUniformiser.cpp \
	XmlUniformiserTest.h \
	XmlUniformiserTest.cpp

cppunittestmain_LDADD= \
  $(top_builddir)/src/cppunit/libcppunit.la 

cppunittestmain_LDFLAGS = $(LIBADD_DL)


