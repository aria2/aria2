// //////////////////////////////////////////////////////////////////////////
// Header file MostRecentTests.h for class MostRecentTests
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/27
// //////////////////////////////////////////////////////////////////////////
#ifndef MOSTRECENTTESTS_H
#define MOSTRECENTTESTS_H

#include <cppunit/Test.h>
#include <deque>
#include <utility>


/*! \class MostRecentTests
 * \brief This class represents a list of the tests most recently run.
 */
class MostRecentTests
{
public:
  /*! Constructs a MostRecentTests object.
   */
  MostRecentTests();

  /*! Destructor.
   */
  virtual ~MostRecentTests();

  void setLastTestRun( CPPUNIT_NS::Test *test );
  CPPUNIT_NS::Test *lastTestRun() const;

  int getRunCount() const;
  CPPUNIT_NS::Test *getTestAt( int indexTest ) const;
  std::string getTestNameAt( int indexTest ) const;


private:
  /// Prevents the use of the copy constructor.
  MostRecentTests( const MostRecentTests &copy );

  /// Prevents the use of the copy operator.
  void operator =( const MostRecentTests &copy );

private:
  typedef std::pair<std::string, CPPUNIT_NS::Test *> TestRun;
  typedef std::deque<TestRun> TestRuns;
  TestRuns m_runs;
};


#endif  // MOSTRECENTTESTS_H
