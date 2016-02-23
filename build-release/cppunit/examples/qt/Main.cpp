#include <qapplication.h>
#include <cppunit/ui/qt/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


int main( int argc, char** argv )
{
	QApplication app( argc, argv );

	//CPPUNIT_NS::QtUi::TestRunner runner;
	
    CPPUNIT_NS::QtTestRunner runner; 
		
	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
	runner.run( true );

	return 0;
}

