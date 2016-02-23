#ifndef CPPUNIT_XMLOUTPUTTERHOOK_H
#define CPPUNIT_XMLOUTPUTTERHOOK_H

#include <cppunit/Portability.h>


CPPUNIT_NS_BEGIN


class Test;
class TestFailure;
class XmlDocument;
class XmlElement;



/*! \brief Hook to customize Xml output.
 *
 * XmlOutputterHook can be passed to XmlOutputter to customize the XmlDocument.
 *
 * Common customizations are:
 * - adding some datas to successfull or failed test with
 *   failTestAdded() and successfulTestAdded(),
 * - adding some statistics with statisticsAdded(),
 * - adding other datas with beginDocument() or endDocument().
 *
 * See examples/ClockerPlugIn which makes use of most the hook.
 *
 * Another simple example of an outputter hook is shown below. It may be  
 * used to add some meta information to your result files. In the example,
 * the author name as well as the project name and test creation date is
 * added to the head of the xml file.
 *
 * In order to make this information stored within the xml file, the virtual 
 * member function beginDocument() is overriden where a new 
 * XmlElement object is created.
 *
 * This element is simply added to the root node of the document which
 * makes the information automatically being stored when the xml file
 * is written.
 *
 * \code
 * #include <cppunit/XmlOutputterHook.h>
 * #include <cppunit/XmlElement.h>
 * #include <cppunit/tools/StringTools.h>
 * 
 * ...
 * 
 * class MyXmlOutputterHook : public CppUnit::XmlOutputterHook
 * {
 * public:
 *   MyXmlOutputterHook(const std::string projectName,
 *                      const std::string author)
 *   {
 *      m_projectName = projectName;
 *      m_author      = author;
 *   };
 * 
 *   virtual ~MyXmlOutputterHook()
 *   {
 *   };
 * 
 *   void beginDocument(CppUnit::XmlDocument* document)
 *   {
 *     if (!document)
 *       return;
 *
 *     // dump current time
 *     std::string szDate          = CppUnit::StringTools::toString( (int)time(0) );
 *     CppUnit::XmlElement* metaEl = new CppUnit::XmlElement("SuiteInfo", 
 *                                                           "");
 *
 *     metaEl->addElement( new CppUnit::XmlElement("Author", m_author) );
 *     metaEl->addElement( new CppUnit::XmlElement("Project", m_projectName) );
 *     metaEl->addElement( new CppUnit::XmlElement("Date", szDate ) );
 *    
 *     document->rootElement().addElement(metaEl);
 *   };
 * private:
 *   std::string m_projectName;
 *   std::string m_author;
 * }; 
 * \endcode
 *
 * Within your application's main code, you need to snap the hook 
 * object into your xml outputter object like shown below:
 *
 * \code
 * CppUnit::TextUi::TestRunner runner;
 * std::ofstream outputFile("testResults.xml");
 * 
 * CppUnit::XmlOutputter* outputter = new CppUnit::XmlOutputter( &runner.result(),
 *                                                               outputFile );    
 * MyXmlOutputterHook hook("myProject", "meAuthor");
 * outputter->addHook(&hook);
 * runner.setOutputter(outputter);    
 * runner.addTest( VectorFixture::suite() );   
 * runner.run();
 * outputFile.close();
 * \endcode
 *
 * This results into the following output:
 *
 * \code
 * <TestRun>
 *   <suiteInfo>
 *     <author>meAuthor</author>
 *     <project>myProject</project>
 *     <date>1028143912</date>
 *   </suiteInfo>
 *   <FailedTests>
 *    ...
 * \endcode
 *
 * \see XmlOutputter, CppUnitTestPlugIn.
 */
class CPPUNIT_API XmlOutputterHook
{
public:
  /*! Called before any elements is added to the root element.
   * \param document XML Document being created.
   */
  virtual void beginDocument( XmlDocument *document );

  /*! Called after adding all elements to the root element.
   * \param document XML Document being created.
   */
  virtual void endDocument( XmlDocument *document );

  /*! Called after adding a fail test element.
   * \param document XML Document being created.
   * \param testElement \<FailedTest\> element.
   * \param test Test that failed.
   * \param failure Test failure data.
   */
  virtual void failTestAdded( XmlDocument *document,
                              XmlElement *testElement,
                              Test *test,
                              TestFailure *failure );

  /*! Called after adding a successful test element.
   * \param document XML Document being created.
   * \param testElement \<Test\> element.
   * \param test Test that was successful.
   */
  virtual void successfulTestAdded( XmlDocument *document,
                                    XmlElement *testElement,
                                    Test *test );

  /*! Called after adding the statistic element.
   * \param document XML Document being created.
   * \param statisticsElement \<Statistics\> element.
   */
  virtual void statisticsAdded( XmlDocument *document,
                                XmlElement *statisticsElement );

  virtual ~XmlOutputterHook() {}
};


CPPUNIT_NS_END

#endif  // CPPUNIT_XMLOUTPUTTERHOOK_H
