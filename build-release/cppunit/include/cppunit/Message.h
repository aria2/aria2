#ifndef CPPUNIT_MESSAGE_H
#define CPPUNIT_MESSAGE_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/portability/CppUnitDeque.h>
#include <string>


CPPUNIT_NS_BEGIN


#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<std::string>;
#endif

/*! \brief Message associated to an Exception.
 * \ingroup CreatingNewAssertions
 *  A message is composed of two items:
 *  - a short description (~20/30 characters)
 *  - a list of detail strings
 *
 *  The short description is used to indicate how the detail strings should be
 *  interpreted. It usually indicates the failure types, such as
 *  "assertion failed", "forced failure", "unexpected exception caught",
 *  "equality assertion failed"... It should not contains new line character (\n).
 *
 *  Detail strings are used to provide more information about the failure. It
 *  can contains the asserted expression, the expected and actual values in an
 *  equality assertion, some addional messages... Detail strings can contains
 *  new line characters (\n).
 */
class CPPUNIT_API Message
{
public:
  Message();

  // Ensure thread-safe copy by detaching the string.
  Message( const Message &other );

  explicit Message( const std::string &shortDescription );

  Message( const std::string &shortDescription,
           const std::string &detail1 );

  Message( const std::string &shortDescription,
           const std::string &detail1,
           const std::string &detail2 );

  Message( const std::string &shortDescription,
           const std::string &detail1,
           const std::string &detail2,
           const std::string &detail3 );

  Message &operator =( const Message &other );

  /*! \brief Returns the short description.
   * \return Short description.
   */
  const std::string &shortDescription() const;

  /*! \brief Returns the number of detail string.
   * \return Number of detail string.
   */
  int detailCount() const;

  /*! \brief Returns the detail at the specified index.
   * \param index Zero based index of the detail string to return.
   * \returns Detail string at the specified index.
   * \exception std::invalid_argument if \a index < 0 or index >= detailCount().
   */
  std::string detailAt( int index ) const;

  /*! \brief Returns a string that represents a list of the detail strings.
   *
   * Example:
   * \code
   * Message message( "not equal", "Expected: 3", "Actual: 7" );
   * std::string details = message.details();
   * // details contains:
   * // "- Expected: 3\n- Actual: 7\n"  \endcode
   *
   * \return A string that is a concatenation of all the detail strings. Each detail
   *         string is prefixed with '- ' and suffixed with '\n' before being
   *         concatenated to the other.
   */
  std::string details() const;

  /*! \brief Removes all detail strings.
   */
  void clearDetails();

  /*! \brief Adds a single detail string.
   * \param detail Detail string to add.
   */
  void addDetail( const std::string &detail );

  /*! \brief Adds two detail strings.
   * \param detail1 Detail string to add.
   * \param detail2 Detail string to add.
   */
  void addDetail( const std::string &detail1,
                  const std::string &detail2 );

  /*! \brief Adds three detail strings.
   * \param detail1 Detail string to add.
   * \param detail2 Detail string to add.
   * \param detail3 Detail string to add.
   */
  void addDetail( const std::string &detail1,
                  const std::string &detail2,
                  const std::string &detail3 );

  /*! \brief Adds the detail strings of the specified message.
   * \param message All the detail strings of this message are added to this one.
   */
  void addDetail( const Message &message );

  /*! \brief Sets the short description.
   * \param shortDescription New short description.
   */
  void setShortDescription( const std::string &shortDescription );

  /*! \brief Tests if a message is identical to another one.
   * \param other Message this message is compared to.
   * \return \c true if the two message are identical, \c false otherwise.
   */
  bool operator ==( const Message &other ) const;

  /*! \brief Tests if a message is different from another one.
   * \param other Message this message is compared to.
   * \return \c true if the two message are not identical, \c false otherwise.
   */
  bool operator !=( const Message &other ) const;

private:
  std::string m_shortDescription;

  typedef CppUnitDeque<std::string> Details;
  Details m_details;
};


CPPUNIT_NS_END

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( pop )
#endif


#endif  // CPPUNIT_MESSAGE_H
