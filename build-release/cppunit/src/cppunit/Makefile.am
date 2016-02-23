#
# $Id: Makefile.am,v 1.44 2005/06/14 21:28:46 blep Exp $
#

EXTRA_DIST = cppunit.dsp cppunit_dll.dsp DllMain.cpp
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

lib_LTLIBRARIES = libcppunit.la

libcppunit_la_SOURCES = \
  AdditionalMessage.cpp \
  Asserter.cpp \
  BeOsDynamicLibraryManager.cpp \
  BriefTestProgressListener.cpp \
  CompilerOutputter.cpp \
  DefaultProtector.h \
  DefaultProtector.cpp \
  DynamicLibraryManager.cpp \
  DynamicLibraryManagerException.cpp \
  Exception.cpp \
  Message.cpp \
  RepeatedTest.cpp \
  PlugInManager.cpp \
  PlugInParameters.cpp \
  Protector.cpp \
  ProtectorChain.h \
  ProtectorContext.h \
  ProtectorChain.cpp \
  SourceLine.cpp \
  StringTools.cpp \
  SynchronizedObject.cpp \
  Test.cpp \
  TestAssert.cpp \
  TestCase.cpp \
  TestCaseDecorator.cpp \
  TestComposite.cpp \
  TestDecorator.cpp \
  TestFactoryRegistry.cpp \
  TestFailure.cpp \
  TestLeaf.cpp \
  TestNamer.cpp \
  TestPath.cpp \
  TestPlugInDefaultImpl.cpp \
  TestResult.cpp \
  TestResultCollector.cpp \
  TestRunner.cpp \
  TestSetUp.cpp \
  TestSuccessListener.cpp \
  TestSuite.cpp \
  TestSuiteBuilderContext.cpp \
  TextOutputter.cpp \
  TextTestProgressListener.cpp \
  TextTestResult.cpp \
  TextTestRunner.cpp \
  TypeInfoHelper.cpp \
  UnixDynamicLibraryManager.cpp \
  ShlDynamicLibraryManager.cpp \
  XmlDocument.cpp \
  XmlElement.cpp \
  XmlOutputter.cpp \
  XmlOutputterHook.cpp \
  Win32DynamicLibraryManager.cpp

libcppunit_la_LDFLAGS= \
 -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
 -release $(LT_RELEASE)

