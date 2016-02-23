#ifndef CPPUNIT_OUTPUTTER_H
#define CPPUNIT_OUTPUTTER_H

#include <cppunit/Portability.h>


CPPUNIT_NS_BEGIN


/*! \brief Abstract outputter to print test result summary.
 * \ingroup WritingTestResult
 */
class CPPUNIT_API Outputter
{
public:
  /// Destructor.
  virtual ~Outputter() {}

  virtual void write() =0;
};


CPPUNIT_NS_END


#endif  // CPPUNIT_OUTPUTTER_H
