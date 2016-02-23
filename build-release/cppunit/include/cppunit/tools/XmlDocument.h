#ifndef CPPUNIT_TOOLS_XMLDOCUMENT_H
#define CPPUNIT_TOOLS_XMLDOCUMENT_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <string>


CPPUNIT_NS_BEGIN


class XmlElement;


/*! \brief A XML Document.
 *
 * A XmlDocument represents a XML file. It holds a pointer on the root XmlElement
 * of the document. It also holds the encoding and style sheet used.
 *
 * By default, the XML document is stand-alone and tagged with enconding "ISO-8859-1".
 */
class CPPUNIT_API XmlDocument
{
public:
  /*! \brief Constructs a XmlDocument object.
   * \param encoding Encoding used in the XML file (default is Latin-1, ISO-8859-1 ). 
   * \param styleSheet Name of the XSL style sheet file used. If empty then no
   *                   style sheet will be specified in the output.
   */
  XmlDocument( const std::string &encoding = "",
               const std::string &styleSheet = "" );

  /// Destructor.
  virtual ~XmlDocument();

  std::string encoding() const;
  void setEncoding( const std::string &encoding = "" );
  
  std::string styleSheet() const;
  void setStyleSheet( const std::string &styleSheet = "" );

  bool standalone() const;

  /*! \brief set the output document as standalone or not.
   *
   *  For the output document, specify wether it's a standalone XML
   *  document, or not.
   *
   *  \param standalone if true, the output will be specified as standalone.
   *         if false, it will be not.
   */
  void setStandalone( bool standalone );
 
  void setRootElement( XmlElement *rootElement );
  XmlElement &rootElement() const;

  std::string toString() const;

private:
  /// Prevents the use of the copy constructor.
  XmlDocument( const XmlDocument &copy );

  /// Prevents the use of the copy operator.
  void operator =( const XmlDocument &copy );

protected:
  std::string m_encoding;
  std::string m_styleSheet;
  XmlElement *m_rootElement;
  bool m_standalone;
};


#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


CPPUNIT_NS_END

#endif  // CPPUNIT_TOOLS_XMLDOCUMENT_H
