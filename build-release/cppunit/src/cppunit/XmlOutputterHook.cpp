#include <cppunit/XmlOutputterHook.h>


CPPUNIT_NS_BEGIN


void 
XmlOutputterHook::beginDocument( XmlDocument *document )
{
}


void 
XmlOutputterHook::endDocument( XmlDocument *document )
{
}


void 
XmlOutputterHook::failTestAdded( XmlDocument *document,
                                 XmlElement *testElement,
                                 Test *test,
                                 TestFailure *failure )
{
}


void 
XmlOutputterHook::successfulTestAdded( XmlDocument *document,
                                       XmlElement *testElement,
                                       Test *test )
{
}


void 
XmlOutputterHook::statisticsAdded( XmlDocument *document,
                                   XmlElement *statisticsElement )
{
}


CPPUNIT_NS_END

