#ifndef CPPUNIT_TESTCOMPSITE_H    // -*- C++ -*-
#define CPPUNIT_TESTCOMPSITE_H

#include <cppunit/Test.h>
#include <string>

CPPUNIT_NS_BEGIN


/*! \brief A Composite of Tests.
 *
 * Base class for all test composites. Subclass this class if you need to implement
 * a custom TestSuite.
 * 
 * \see Test, TestSuite.
 */
class CPPUNIT_API TestComposite : public Test
{
public:
  TestComposite( const std::string &name = "" );

  ~TestComposite();

  void run( TestResult *result );

  int countTestCases() const;
  
  std::string getName() const;

private:
  TestComposite( const TestComposite &other );
  TestComposite &operator =( const TestComposite &other ); 

  virtual void doStartSuite( TestResult *controller );
  virtual void doRunChildTests( TestResult *controller );
  virtual void doEndSuite( TestResult *controller );

private:
  const std::string m_name;
};


CPPUNIT_NS_END

#endif // CPPUNIT_TESTCOMPSITE_H
