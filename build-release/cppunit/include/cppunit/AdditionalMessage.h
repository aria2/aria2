#ifndef CPPUNIT_ADDITIONALMESSAGE_H
#define CPPUNIT_ADDITIONALMESSAGE_H

#include <cppunit/Message.h>


CPPUNIT_NS_BEGIN


/*! \brief An additional Message for assertions.
 * \ingroup CreatingNewAssertions
 *
 * Provides a implicit constructor that takes a single string. This allow this
 * class to be used as the message arguments in macros.
 *
 * The constructed object is either a Message with a single detail string if
 * a string was passed to the macro, or a copy of the Message passed to the macro.
 *
 * Here is an example of usage:
 * \code
 * 
 *   void checkStringEquals( const std::string &expected,
 *                          const std::string &actual,
 *                           const CppUnit::SourceLine &sourceLine,
 *                           const CppUnit::AdditionalMessage &message );
 *  
 *   #define XTLUT_ASSERT_STRING_EQUAL_MESSAGE( expected, actual, message )  \
 *     ::XtlUt::Impl::checkStringEquals( ::Xtl::toString(expected),        \
 *                                       ::Xtl::toString(actual),          \
 *                                       CPPUNIT_SOURCELINE(),             \
 *                                       message )
 * \endcode
 *
 * In the previous example, the user can specify a simple string for \a message,
 * or a complex Message object.
 *
 * \see Message
 */
class CPPUNIT_API AdditionalMessage : public Message
{
public:
  typedef Message SuperClass;

  /// Constructs an empty Message.
  AdditionalMessage();

  /*! \brief Constructs a Message with the specified detail string.
   * \param detail1 Detail string of the message. If empty, then it is not added.
   */
  AdditionalMessage( const std::string &detail1 );

  /*! \brief Constructs a Message with the specified detail string.
   * \param detail1 Detail string of the message. If empty, then it is not added.
   */
  AdditionalMessage( const char *detail1 );

  /*! \brief Constructs a copy of the specified message.
   * \param other Message to copy.
   */
  AdditionalMessage( const Message &other );

  /*! \brief Assignment operator.
   * \param other Message to copy.
   * \return Reference on this object.
   */
  AdditionalMessage &operator =( const Message &other );

private:
};


CPPUNIT_NS_END



#endif  // CPPUNIT_ADDITIONALMESSAGE_H
