A test plug-ins that track tests and test suites running time. It demonstrates
TestListener, TestPlugIn, and XmlOutputterHook.

Both suite and test case times are tracked. The plug-in include in the XML
output the TestPath of each test cases and its tracked time.

The timed test hierarchy is also included in the XML output. This way it is
possible to see the time each suite takes to run.



* Usage:

Just add this plug-in to DllPlugInTester command line. It will add a test 
listener to track test time, and add a hook to the XmlOutputter to include
test time to the XmlOutput.

If the option "text" is passed to the plug-in, the timed test tree will be
printed to stdout.

DllPlugInRunnerd.exe ClockerPlugInd.dll
or
DllPlugInRunnerd.exe ClockerPlugInd.dll=text

* Example:

DllPlugInTesterd_dll.exe -x timed.xml ClockerPlugInd.dll CppUnitTestPlugInd.dll 

Will track time of all tests contains in CppUnitTestPlugInd.dll and save the
result in timed.xml.

* Notes:

The id of the <TimedTestTree> are different than those of the
<SuccessfulTests> and <FailedTests> trees. You can use the <TestPath> to 
cross-reference the datas.

* Remarks:

You may want to review ClockerModel before using this plug-in for serious
purpose, add timing based on the process cpu time.

A version is provided for NT that use the main thread cpu time. This is an issue 
if the test cases are multithreaded.
