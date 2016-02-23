#include <cppunit/Portability.h>
#include <cppunit/Test.h>
#include <cppunit/TestPath.h>
#include <stdexcept>


CPPUNIT_NS_BEGIN


TestPath::TestPath()
{
}


TestPath::TestPath( Test *root )
{
  add( root );
}


TestPath::TestPath( const TestPath &other, 
                    int indexFirst, 
                    int count )
{
  int countAdjustment = 0;
  if ( indexFirst < 0 )
  {
    countAdjustment = indexFirst;
    indexFirst = 0;
  }

  if ( count < 0 )
    count = other.getTestCount();
  else
    count += countAdjustment;

  int index = indexFirst;
  while ( count-- > 0  &&  index < other.getTestCount() )
    add( other.getTestAt( index++ ) );
}


TestPath::TestPath( Test *searchRoot, 
                    const std::string &pathAsString )
{
  PathTestNames testNames;

  Test *parentTest = findActualRoot( searchRoot, pathAsString, testNames );
  add( parentTest );

  for ( unsigned int index = 1; index < testNames.size(); ++index )
  {
    bool childFound = false;
    for ( int childIndex =0; childIndex < parentTest->getChildTestCount(); ++childIndex )
    {
      if ( parentTest->getChildTestAt( childIndex )->getName() == testNames[index] )
      {
        childFound = true;
        parentTest = parentTest->getChildTestAt( childIndex );
        break;
      }
    }

    if ( !childFound )
      throw std::invalid_argument( "TestPath::TestPath(): failed to resolve test name <"+
                                   testNames[index] + "> of path <" + pathAsString + ">" );

    add( parentTest );
  }
}


TestPath::TestPath( const TestPath &other )
  : m_tests( other.m_tests )
{
}


TestPath::~TestPath()
{
}


TestPath &
TestPath::operator =( const TestPath &other )
{
  if ( &other != this )
    m_tests = other.m_tests;
  return *this;
}


bool 
TestPath::isValid() const
{
  return getTestCount() > 0;
}


void 
TestPath::add( Test *test )
{
  m_tests.push_back( test );
}


void 
TestPath::add( const TestPath &path )
{
  for ( int index =0; index < path.getTestCount(); ++index )
    add( path.getTestAt( index ) );
}


void 
TestPath::insert( Test *test, 
                  int index )
{
  if ( index < 0  ||  index > getTestCount() )
    throw std::out_of_range( "TestPath::insert(): index out of range" );
  m_tests.insert( m_tests.begin() + index, test );
}

void 
TestPath::insert( const TestPath &path, 
                  int index )
{
  int itemIndex = path.getTestCount() -1;
  while ( itemIndex >= 0 )
    insert( path.getTestAt( itemIndex-- ), index );
}


void 
TestPath::removeTests()
{
  while ( isValid() )
    removeTest( 0 );
}


void 
TestPath::removeTest( int index )
{
  checkIndexValid( index );
  m_tests.erase( m_tests.begin() + index );
}


void 
TestPath::up()
{
  checkIndexValid( 0 );
  removeTest( getTestCount() -1 );
}


int 
TestPath::getTestCount() const
{
  return m_tests.size();
}


Test *
TestPath::getTestAt( int index ) const
{
  checkIndexValid( index );
  return m_tests[index];
}


Test *
TestPath::getChildTest() const
{
  return getTestAt( getTestCount() -1 );
}


void 
TestPath::checkIndexValid( int index ) const
{
  if ( index < 0  ||  index >= getTestCount() )
    throw std::out_of_range( "TestPath::checkIndexValid(): index out of range" );
}


std::string 
TestPath::toString() const
{
  std::string asString( "/" );
  for ( int index =0; index < getTestCount(); ++index )
  {
    if ( index > 0 )
      asString += '/';
    asString += getTestAt(index)->getName();
  }

  return asString;
}


Test *
TestPath::findActualRoot( Test *searchRoot,
                          const std::string &pathAsString,
                          PathTestNames &testNames )
{
  bool isRelative = splitPathString( pathAsString, testNames );

  if ( isRelative  &&  pathAsString.empty() )
    return searchRoot;

  if ( testNames.empty() )
    throw std::invalid_argument( "TestPath::TestPath(): invalid root or root name in absolute path" );

  Test *root = isRelative ? searchRoot->findTest( testNames[0] )  // throw if bad test name
                          : searchRoot;
  if ( root->getName() != testNames[0] )
    throw std::invalid_argument( "TestPath::TestPath(): searchRoot does not match path root name" );

  return root;
}


bool
TestPath::splitPathString( const std::string &pathAsString,
                           PathTestNames &testNames )
{
  if ( pathAsString.empty() )
    return true;

  bool isRelative = pathAsString[0] != '/';

  int index = (isRelative ? 0 : 1);
  while ( true )
  {
    int separatorIndex = pathAsString.find( '/', index );
    if ( separatorIndex >= 0 )
    {
      testNames.push_back( pathAsString.substr( index, separatorIndex - index ) );
      index = separatorIndex + 1;
    }
    else
    {
      testNames.push_back( pathAsString.substr( index ) );
      break;
    }
  }

  return isRelative;
}
  

CPPUNIT_NS_END
