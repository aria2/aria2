#ifndef CPPUNIT_BRIEFTESTPROGRESSLISTENER_H
#define CPPUNIT_BRIEFTESTPROGRESSLISTENER_H

#include <cppunit/TestListener.h>


CPPUNIT_NS_BEGIN


/*! \brief TestListener that prints the name of each test before running it.
 * \ingroup TrackingTestExecution
 */
class CPPUNIT_API BriefTestProgressListener : public TestListener
{
public:
  /*! Constructs a BriefTestProgressListener object.
   */
  BriefTestProgressListener();

  /// Destructor.
  virtual ~BriefTestProgressListener();

  void startTest( Test *test );

  void addFailure( const TestFailure &failure );

  void endTest( Test *test );

private:
  /// Prevents the use of the copy constructor.
  BriefTestProgressListener( const BriefTestProgressListener &copy );

  /// Prevents the use of the copy operator.
  void operator =( const BriefTestProgressListener &copy );

private:
  bool m_lastTestFailed;
};


CPPUNIT_NS_END

#endif  // CPPUNIT_BRIEFTESTPROGRESSLISTENER_H
