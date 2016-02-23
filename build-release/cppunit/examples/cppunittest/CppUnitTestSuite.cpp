#include <cppunit/config/SourcePrefix.h>
#include <cppunit/extensions/HelperMacros.h>
#include "CoreSuite.h"
#include "HelperSuite.h"
#include "ExtensionSuite.h"
#include "OutputSuite.h"
#include "ToolsSuite.h"
#include "UnitTestToolSuite.h"

CPPUNIT_REGISTRY_ADD_TO_DEFAULT( coreSuiteName() );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( extensionSuiteName() );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( helperSuiteName() );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( outputSuiteName() );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( toolsSuiteName() );
CPPUNIT_REGISTRY_ADD_TO_DEFAULT( unitTestToolSuiteName() );
