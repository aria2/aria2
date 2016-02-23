#ifndef CPPUNIT_TESTLEAF_H
#define CPPUNIT_TESTLEAF_H

#include <cppunit/Test.h>


CPPUNIT_NS_BEGIN


/*! \brief A single test object.
 *
 * Base class for single test case: a test that doesn't have any children.
 *
 */
class CPPUNIT_API TestLeaf: public Test
{
public:
  /*! Returns 1 as the default number of test cases invoked by run().
   * 
   * You may override this method when many test cases are invoked (RepeatedTest
   * for example).
   * 
   * \return 1.
   * \see Test::countTestCases().
   */
  int countTestCases() const;

  /*! Returns the number of child of this test case: 0.
   *
   * You should never override this method: a TestLeaf as no children by definition.
   *
   * \return 0.
   */
  int getChildTestCount() const;

  /*! Always throws std::out_of_range.
   * \see Test::doGetChildTestAt().
   */
  Test *doGetChildTestAt( int index ) const;
};

CPPUNIT_NS_END

#endif // CPPUNIT_TESTLEAF_H
