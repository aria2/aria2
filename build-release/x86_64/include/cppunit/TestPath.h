#ifndef CPPUNIT_TESTPATH_H
#define CPPUNIT_TESTPATH_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif

#include <cppunit/portability/CppUnitDeque.h>
#include <string>

CPPUNIT_NS_BEGIN


class Test;

#if CPPUNIT_NEED_DLL_DECL
//  template class CPPUNIT_API std::deque<Test *>;
#endif


/*! \brief A List of Test representing a path to access a Test.
 * \ingroup ExecutingTest
 *
 * The path can be converted to a string and resolved from a string with toString()
 * and TestPath( Test *root, const std::string &pathAsString ).
 *
 * Pointed tests are not owned by the class.
 *
 * \see Test::resolvedTestPath()
 */
class CPPUNIT_API TestPath
{
public:
  /*! \brief Constructs an invalid path.
   * 
   * The path is invalid until a test is added with add().
   */
  TestPath();

  /*! \brief Constructs a valid path.
   *
   * \param root Test to add.
   */
  TestPath( Test *root );

  /*! \brief Constructs a path using a slice of another path.
   * \param otherPath Path the test are copied from.
   * \param indexFirst Zero based index of the first test to copy. Adjusted to be in valid
   *                   range. \a count is adjusted with \a indexFirst.
   * \param count Number of tests to copy. If < 0 then all test starting from index
   *              \a indexFirst are copied.
   */
  TestPath( const TestPath &otherPath, 
            int indexFirst, 
            int count = -1 );

  /*! \brief Resolves a path from a string returned by toString().
   *
   * If \a pathAsString is an absolute path (begins with '/'), then the first test name
   * of the path must be the name of \a searchRoot. Otherwise, \a pathAsString is a 
   * relative path, and the first test found using Test::findTest() matching the first
   * test name is used as root. An empty string resolve to a path containing 
   * \a searchRoot.
   *
   * The resolved path is always valid.
   *
   * \param searchRoot Test used to resolve the path.
   * \param pathAsString String that contains the path as a string created by toString().
   * \exception std::invalid_argument if one of the test names can not be resolved.
   * \see toString().
   */
  TestPath( Test *searchRoot, 
            const std::string &pathAsString );

  /*! \brief Copy constructor.
   * \param other Object to copy.
   */
  TestPath( const TestPath &other );

  virtual ~TestPath();

  /*! \brief Tests if the path contains at least one test.
   * \return \c true if the path contains at least one test, otherwise returns \c false.
   */
  virtual bool isValid() const;

  /*! \brief Adds a test to the path.
   * \param test Pointer on the test to add. Must not be \c NULL.
   */
  virtual void add( Test *test );

  /*! \brief Adds all the tests of the specified path.
   * \param path Path that contains the test to add.
   */
  virtual void add( const TestPath &path );

  /*! \brief Inserts a test at the specified index.
   * \param test Pointer on the test to insert. Must not be \c NULL.
   * \param index Zero based index indicating where the test is inserted.
   * \exception std::out_of_range is \a index < 0 or \a index > getTestCount().
   */
  virtual void insert( Test *test, int index );

  /*! \brief Inserts all the tests at the specified path at a given index.
   * \param path Path that contains the test to insert.
   * \param index Zero based index indicating where the tests are inserted.
   * \exception std::out_of_range is \a index < 0 or \a index > getTestCount(), and
   *            \a path is valid.
   */
  virtual void insert( const TestPath &path, int index );

  /*! \brief Removes all the test from the path.
   *
   * The path becomes invalid after this call.
   */
  virtual void removeTests();

  /*! \brief Removes the test at the specified index of the path.
   * \param index Zero based index of the test to remove.
   * \exception std::out_of_range is \a index < 0 or \a index >= getTestCount().
   */
  virtual void removeTest( int index );

  /*! \brief Removes the last test.
   * \exception std::out_of_range is the path is invalid.
   * \see isValid().
   */
  virtual void up();

  /*! \brief Returns the number of tests in the path.
   * \return Number of tests in the path.
   */
  virtual int getTestCount() const;

  /*! \brief Returns the test of the specified index.
   * \param index Zero based index of the test to return.
   * \return Pointer on the test at index \a index. Never \c NULL.
   * \exception std::out_of_range is \a index < 0 or \a index >= getTestCount().
   */
  virtual Test *getTestAt( int index ) const;

  /*! \brief Get the last test of the path.
   * \return Pointer on the last test (test at the bottom of the hierarchy). Never \c NULL.
   * \exception std::out_of_range if the path is not valid ( isValid() returns \c false ).
   */
  virtual Test *getChildTest() const;

  /*! \brief Returns the path as a string.
   *
   * For example, if a path is composed of three tests named "All Tests", "Math" and
   * "Math::testAdd", toString() will return:
   *
   * "All Tests/Math/Math::testAdd".
   * 
   * \return A string composed of the test names separated with a '/'. It is a relative
   *         path.
   */
  virtual std::string toString() const;

  /*! \brief Assignment operator.
   * \param other Object to copy.
   * \return This object.
   */
  TestPath &operator =( const TestPath &other );

protected:
  /*! \brief Checks that the specified test index is within valid range.
   * \param index Zero based index to check.
   * \exception std::out_of_range is \a index < 0 or \a index >= getTestCount().
   */
  void checkIndexValid( int index ) const;

  /// A list of test names.
  typedef CppUnitDeque<std::string> PathTestNames;

  /*! \brief Splits a path string into its test name components.
   * \param pathAsString Path string created with toString().
   * \param testNames Test name components are added to that container.
   * \return \c true if the path is relative (does not begin with '/'), \c false
   *         if it is absolute (begin with '/').
   */
  bool splitPathString( const std::string &pathAsString,
                        PathTestNames &testNames );

  /*! \brief Finds the actual root of a path string and get the path string name components.
   * \param searchRoot Test used as root if the path string is absolute, or to search
   *                   the root test if the path string is relative.
   * \param pathAsString Path string. May be absolute or relative.
   * \param testNames Test name components are added to that container.
   * \return Pointer on the resolved root test. Never \c NULL.
   * \exception std::invalid_argument if either the root name can not be resolved or if
   *            pathAsString contains no name components.
   */
  Test *findActualRoot( Test *searchRoot,
                        const std::string &pathAsString,
                        PathTestNames &testNames );

protected:
  typedef CppUnitDeque<Test *> Tests;
  Tests m_tests;

};


CPPUNIT_NS_END

#endif // CPPUNIT_TESTPATH_H

