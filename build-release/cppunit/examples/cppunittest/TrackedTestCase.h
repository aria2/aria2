#ifndef TRACKEDTESTCASE_H
#define TRACKEDTESTCASE_H

#include <cppunit/TestCase.h>


class Tracker
{
public:
  virtual ~Tracker() {}

  virtual void onConstructor() {}
  virtual void onDestructor() {}
  virtual void onSetUp() {}
  virtual void onTearDown() {}
  virtual void onTest() {};
};


class TrackedTestCase : public CPPUNIT_NS::TestCase
{
public:
  TrackedTestCase();

  virtual ~TrackedTestCase();

  virtual void setUp();
  virtual void tearDown();

  void test();

  static void setTracker( Tracker *tracker );
  static void removeTracker();

private:
  TrackedTestCase( const TrackedTestCase &copy );

  void operator =( const TrackedTestCase &copy );

private:
  static Tracker *ms_tracker;
};


#endif  // TRACKEDTESTCASE_H
