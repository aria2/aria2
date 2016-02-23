#ifndef CPPUNIT_EXCEPTION_H
#define CPPUNIT_EXCEPTION_H

#include <cppunit/Portability.h>
#include <cppunit/Message.h>
#include <cppunit/SourceLine.h>
#include <exception>


CPPUNIT_NS_BEGIN


/*! \brief Exceptions thrown by failed assertions.
 * \ingroup BrowsingCollectedTestResult
 *
 * Exception is an exception that serves
 * descriptive strings through its what() method
 */
class CPPUNIT_API Exception : public std::exception
{
public:
  /*! \brief Constructs the exception with the specified message and source location.
   * \param message Message associated to the exception.
   * \param sourceLine Source location related to the exception.
   */
  Exception( const Message &message = Message(), 
             const SourceLine &sourceLine = SourceLine() );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
  /*!
   * \deprecated Use other constructor instead.
   */
  Exception( std::string  message, 
	     long lineNumber, 
	     std::string fileName );
#endif

  /*! \brief Constructs a copy of an exception.
   * \param other Exception to copy.
   */
  Exception( const Exception &other );

  /// Destructs the exception
  virtual ~Exception() throw();

  /// Performs an assignment
  Exception &operator =( const Exception &other );

  /// Returns descriptive message
  const char *what() const throw();

  /// Location where the error occured
  SourceLine sourceLine() const;

  /// Message related to the exception.
  Message message() const;

  /// Set the message.
  void setMessage( const Message &message );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
  /// The line on which the error occurred
  long lineNumber() const;

  /// The file in which the error occurred
  std::string fileName() const;

  static const std::string UNKNOWNFILENAME;
  static const long UNKNOWNLINENUMBER;
#endif

  /// Clones the exception.
  virtual Exception *clone() const;

protected:
  // VC++ does not recognize call to parent class when prefixed
  // with a namespace. This is a workaround.
  typedef std::exception SuperClass;

  Message m_message;
  SourceLine m_sourceLine;
  std::string m_whatMessage;
};


CPPUNIT_NS_END


#endif // CPPUNIT_EXCEPTION_H

