#include <cppunit/Portability.h>
#include <cppunit/Test.h>
#include <cppunit/TestPath.h>
#include <stdexcept>


CPPUNIT_NS_BEGIN


Test *
Test::getChildTestAt( int index ) const
{
  checkIsValidIndex( index );
  return doGetChildTestAt( index );
}


Test *
Test::findTest( const std::string &testName ) const
{
  TestPath path;
  Test *mutableThis = CPPUNIT_CONST_CAST( Test *, this );
  mutableThis->findTestPath( testName, path );
  if ( !path.isValid() )
    throw std::invalid_argument( "No test named <" + testName + "> found in test <"
                                 + getName() + ">." );
  return path.getChildTest();
}


bool 
Test::findTestPath( const std::string &testName,
                    TestPath &testPath ) const
{
  Test *mutableThis = CPPUNIT_CONST_CAST( Test *, this );
  if ( getName() == testName )
  {
    testPath.add( mutableThis );
    return true;
  }

  int childCount = getChildTestCount();
  for ( int childIndex =0; childIndex < childCount; ++childIndex )
  {
    if ( getChildTestAt( childIndex )->findTestPath( testName, testPath ) )
    {
      testPath.insert( mutableThis, 0 );
      return true;
    }
  }

  return false;
}


bool 
Test::findTestPath( const Test *test,
                    TestPath &testPath ) const
{
  Test *mutableThis = CPPUNIT_CONST_CAST( Test *, this );
  if ( this == test )
  {
    testPath.add( mutableThis );
    return true;
  }

  int childCount = getChildTestCount();
  for ( int childIndex =0; childIndex < childCount; ++childIndex )
  {
    if ( getChildTestAt( childIndex )->findTestPath( test, testPath ) )
    {
      testPath.insert( mutableThis, 0 );
      return true;
    }
  }

  return false;
}


TestPath 
Test::resolveTestPath( const std::string &testPath ) const
{
  Test *mutableThis = CPPUNIT_CONST_CAST( Test *, this );
  return TestPath( mutableThis, testPath );
}


void 
Test::checkIsValidIndex( int index ) const
{
  if ( index < 0  ||  index >= getChildTestCount() )
    throw std::out_of_range( "Test::checkValidIndex(): invalid index" );
}
  

CPPUNIT_NS_END
