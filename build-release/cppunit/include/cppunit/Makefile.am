SUBDIRS = extensions ui plugin config tools portability

DISTCLEANFILES = config-auto.h

libcppunitincludedir = $(includedir)/cppunit
libcppunitinclude_HEADERS =  \
	config-auto.h \
  AdditionalMessage.h \
	Asserter.h \
	BriefTestProgressListener.h \
	CompilerOutputter.h \
	Exception.h \
	Message.h \
	Outputter.h \
	Portability.h \
	Protector.h \
	SourceLine.h \
	SynchronizedObject.h \
	Test.h \
	TestAssert.h \
	TestCase.h \
	TestCaller.h \
	TestComposite.h \
	TestFailure.h \
	TestFixture.h \
	TestLeaf.h \
	TestPath.h \
	TestResult.h \
	TestResultCollector.h \
	TestRunner.h \
	TestSuccessListener.h \
	TestSuite.h \
	TextOutputter.h \
	TextTestProgressListener.h \
	TextTestResult.h \
	TextTestRunner.h \
	TestListener.h \
	XmlOutputter.h \
	XmlOutputterHook.h

dist-hook:
	rm -f $(distdir)/config-auto.h
