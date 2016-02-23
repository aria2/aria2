#ifndef CPPUNIT_XMLTESTRESULTOUTPUTTER_H
#define CPPUNIT_XMLTESTRESULTOUTPUTTER_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/Outputter.h>
#include <cppunit/portability/CppUnitDeque.h>
#include <cppunit/portability/CppUnitMap.h>
#include <cppunit/portability/Stream.h>


CPPUNIT_NS_BEGIN


class Test;
class TestFailure;
class TestResultCollector;
class XmlDocument;
class XmlElement;
class XmlOutputterHook;


/*! \brief Outputs a TestResultCollector in XML format.
 * \ingroup WritingTestResult
 *
 * Save the test result as a XML stream. 
 *
 * Additional datas can be added to the XML document using XmlOutputterHook. 
 * Hook are not owned by the XmlOutputter. They should be valid until 
 * destruction of the XmlOutputter. They can be removed with removeHook().
 *
 * \see XmlDocument, XmlElement, XmlOutputterHook.
 */
class CPPUNIT_API XmlOutputter : public Outputter
{
public:
  /*! \brief Constructs a XmlOutputter object.
   * \param result Result of the test run.
   * \param stream Stream used to output the XML output.
   * \param encoding Encoding used in the XML file (default is Latin-1). 
   */
  XmlOutputter( TestResultCollector *result,
                OStream &stream,
                std::string encoding = std::string("ISO-8859-1") );

  /// Destructor.
  virtual ~XmlOutputter();

  /*! \brief Adds the specified hook to the outputter.
   * \param hook Hook to add. Must not be \c NULL.
   */
  virtual void addHook( XmlOutputterHook *hook );

  /*! \brief Removes the specified hook from the outputter.
   * \param hook Hook to remove.
   */
  virtual void removeHook( XmlOutputterHook *hook );

  /*! \brief Writes the specified result as an XML document to the stream.
   *
   * Refer to examples/cppunittest/XmlOutputterTest.cpp for example
   * of use and XML document structure.
   */
  virtual void write();

  /*! \brief Sets the XSL style sheet used.
   *
   * \param styleSheet Name of the style sheet used. If empty, then no style sheet
   *                   is used (default).
   */
  virtual void setStyleSheet( const std::string &styleSheet );

  /*! \brief set the output document as standalone or not.
   *
   *  For the output document, specify wether it's a standalone XML
   *  document, or not.
   *
   *  \param standalone if true, the output will be specified as standalone.
   *         if false, it will be not.
   */
  virtual void setStandalone( bool standalone );

  typedef CppUnitMap<Test *,TestFailure*, std::less<Test*> > FailedTests;

  /*! \brief Sets the root element and adds its children.
   *
   * Set the root element of the XML Document and add its child elements.
   *
   * For all hooks, call beginDocument() just after creating the root element (it
   * is empty at this time), and endDocument() once all the datas have been added
   * to the root element.
   */
  virtual void setRootNode();

  virtual void addFailedTests( FailedTests &failedTests,
                               XmlElement *rootNode );

  virtual void addSuccessfulTests( FailedTests &failedTests,
                                   XmlElement *rootNode );

  /*! \brief Adds the statics element to the root node.
   * 
   * Creates a new element containing statistics data and adds it to the root element.
   * Then, for all hooks, call statisticsAdded().
   * \param rootNode Root element.
   */
  virtual void addStatistics( XmlElement *rootNode );

  /*! \brief Adds a failed test to the failed tests node.
   * Creates a new element containing datas about the failed test, and adds it to 
   * the failed tests element.
   * Then, for all hooks, call failTestAdded().
   */
  virtual void addFailedTest( Test *test,
                              TestFailure *failure,
                              int testNumber,
                              XmlElement *testsNode );

  virtual void addFailureLocation( TestFailure *failure,
                                   XmlElement *testElement );


  /*! \brief Adds a successful test to the successful tests node.
   * Creates a new element containing datas about the successful test, and adds it to 
   * the successful tests element.
   * Then, for all hooks, call successfulTestAdded().
   */
  virtual void addSuccessfulTest( Test *test, 
                                  int testNumber,
                                  XmlElement *testsNode );
protected:
  virtual void fillFailedTestsMap( FailedTests &failedTests );

protected:
  typedef CppUnitDeque<XmlOutputterHook *> Hooks;

  TestResultCollector *m_result;
  OStream &m_stream;
  std::string m_encoding;
  std::string m_styleSheet;
  XmlDocument *m_xml;
  Hooks m_hooks;

private:
  /// Prevents the use of the copy constructor.
  XmlOutputter( const XmlOutputter &copy );

  /// Prevents the use of the copy operator.
  void operator =( const XmlOutputter &copy );

private:
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


#endif  // CPPUNIT_XMLTESTRESULTOUTPUTTER_H
