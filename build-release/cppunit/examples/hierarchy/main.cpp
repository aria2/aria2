#include <cppunit/ui/text/TestRunner.h>

#include "BoardGame.h"
#include "Chess.h"
#include "BoardGameTest.h"
#include "ChessTest.h"



int 
main(int argc, char** argv)
{
  CPPUNIT_NS::TextUi::TestRunner runner;

  runner.addTest( BoardGameTest<BoardGame>::suite() );
  runner.addTest( ChessTest<Chess>::suite() );

  bool wasSuccessful = runner.run();

  return wasSuccessful ? 0 : 1;
}
