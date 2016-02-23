#ifndef MOCKFUNCTOR_H
#define MOCKFUNCTOR_H

#include <cppunit/TestAssert.h>
#include <cppunit/Protector.h>
#include "FailureException.h"
#include "MockProtector.h"


class MockFunctor : public CPPUNIT_NS::Functor
{
public:
  MockFunctor()
    : m_shouldSucceed( true )
    , m_shouldThrow( false )
    , m_shouldThrowFailureException( false )
    , m_hasExpectation( false )
    , m_actualCallCount( 0 )
    , m_expectedCallCount( 0 )
  {
  }


  bool operator()() const
  {
    ++CPPUNIT_CONST_CAST(MockFunctor *,this)->m_actualCallCount;

    if ( m_shouldThrow )
    {
      if ( m_shouldThrowFailureException )
        throw FailureException();
      throw MockProtectorException();
    }

    return m_shouldSucceed;
  }

  void setThrowFailureException()
  {
    m_shouldThrow = true;
    m_shouldThrowFailureException = true;
    ++m_expectedCallCount;
    m_hasExpectation = true;
  }

  void setThrowMockProtectorException()
  {
    m_shouldThrow = true;
    m_shouldThrowFailureException = false;
    ++m_expectedCallCount;
    m_hasExpectation = true;
  }

  void setShouldFail()
  {
    m_shouldSucceed = false;
  }

  void setShouldSucceed()
  {
    m_shouldSucceed = true;
  }

  void setExpectedCallCount( int callCount =1 )
  {
    m_expectedCallCount = callCount;
    m_hasExpectation = true;
  }

  void verify()
  {
    if ( m_hasExpectation )
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "MockFunctor: bad call count",
                                    m_expectedCallCount,
                                    m_actualCallCount );
    }
  }

private:
  bool m_shouldSucceed;
  bool m_shouldThrow;
  bool m_shouldThrowFailureException;
  bool m_hasExpectation;
  int m_actualCallCount;
  int m_expectedCallCount;
};


#endif // MOCKFUNCTOR_H
