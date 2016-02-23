#ifndef MOCKPROTECTOR_H
#define MOCKPROTECTOR_H

#include <stdexcept>
#include <cppunit/Protector.h>


class MockProtectorException : public std::runtime_error
{
public:
  MockProtectorException() 
    : std::runtime_error( "MockProtectorException" )
  {
  }
};


class MockProtector : public CPPUNIT_NS::Protector
{
public:
  MockProtector()
    : m_wasCalled( false )
    , m_wasTrapped( false )
    , m_expectException( false )
    , m_hasExpectation( false )
    , m_shouldPropagateException( false )
  {
  }

  bool protect( const CPPUNIT_NS::Functor &functor,
                const CPPUNIT_NS::ProtectorContext &context )
  {
    try
    {
      m_wasCalled = true;
      return functor();
    }
    catch ( MockProtectorException & )
    {
      m_wasTrapped = true;

      if ( m_shouldPropagateException )
        throw;

      reportError( context, CPPUNIT_NS::Message("MockProtector trap") );
    }

    return false;
  }

  void setExpectException()
  {
    m_expectException = true;
    m_hasExpectation = true;
  }

  void setExpectNoException()
  {
    m_expectException = false;
    m_hasExpectation = true;
  }

  void setExpectCatchAndPropagateException()
  {
    setExpectException();
    m_shouldPropagateException = true;
  }

  void verify()
  {
    if ( m_hasExpectation )
    {
      CPPUNIT_ASSERT_MESSAGE( "MockProtector::protect() was not called",
                              m_wasCalled );

      std::string message;
      if ( m_expectException )
        message = "did not catch the exception.";
      else
        message = "caught an unexpected exception.";
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "MockProtector::protect() " + message,
                                    m_expectException,
                                    m_wasTrapped );
    }
  }

private:
  bool m_wasCalled;
  bool m_wasTrapped;
  bool m_expectException;
  bool m_hasExpectation;
  bool m_shouldPropagateException;
};


#endif // MOCKPROTECTOR_H
