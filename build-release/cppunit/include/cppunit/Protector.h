#ifndef CPPUNIT_PROTECTOR_H
#define CPPUNIT_PROTECTOR_H

#include <cppunit/SourceLine.h>

CPPUNIT_NS_BEGIN

class Exception;
class Message;
class ProtectorContext;
class TestResult;


class CPPUNIT_API Functor
{
public:
  virtual ~Functor();

  virtual bool operator()() const =0;
};


/*! \brief Protects one or more test case run.
 *
 * Protector are used to globably 'decorate' a test case. The most common
 * usage of Protector is to catch exception that do not subclass std::exception,
 * such as MFC CException class or Rogue Wave RWXMsg class, and capture the
 * message associated to the exception. In fact, CppUnit capture message from
 * Exception and std::exception using a Protector.
 *
 * Protector are chained. When you add a Protector using 
 * TestResult::pushProtector(), your protector is in fact passed as a Functor
 * to the first protector of the chain.
 *
 * TestCase protects call to setUp(), runTest() and tearDown() by calling
 * TestResult::protect().
 *
 * Because the protector chain is handled by TestResult, a protector can be
 * active for a single test, or a complete test run.
 *
 * Here are some possible usages:
 * - run all test case in a separate thread and assumes the test failed if it
 *   did not finish in a given time (infinite loop work around)
 * - performance tracing : time only the runTest() time.
 * \sa TestResult, TestCase, TestListener.
 */
class CPPUNIT_API Protector
{
public:
  virtual ~Protector();
  
  virtual bool protect( const Functor &functor,
                        const ProtectorContext &context ) =0;

protected:
  void reportError( const ProtectorContext &context,
                    const Exception &error ) const;

  void reportError( const ProtectorContext &context,
                    const Message &message,
                    const SourceLine &sourceLine = SourceLine() ) const;

  void reportFailure( const ProtectorContext &context,
                      const Exception &failure ) const;

  Message actualMessage( const Message &message,
                         const ProtectorContext &context ) const;
};


/*! \brief Scoped protector push to TestResult.
 *
 * Adds the specified Protector to the specified TestResult for the object
 * life-time.
 */
class CPPUNIT_API ProtectorGuard
{
public:
  /// Pushes the specified protector.
  ProtectorGuard( TestResult *result,
                  Protector *protector );

  /// Pops the protector.
  ~ProtectorGuard();

private:
  TestResult *m_result;
};

CPPUNIT_NS_END


#endif // CPPUNIT_PROTECTOR_H

