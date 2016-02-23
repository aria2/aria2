#----------------------------------------------------------------------
# File:    qt_example.pro
# Purpose: qmake config file for the QtTestRunner example.
#          The program is built with the QtTestRunner debug staticlib.
#          Set the CONFIG variable accordingly to build it differently.
#----------------------------------------------------------------------

TEMPLATE = app
LANGUAGE = C++
TARGET   = qt_example

# Get rid of possibly predefined options

CONFIG -= debug
CONFIG -= release

CONFIG += qt warn_on debug use_static

#CONFIG += qt warn_on release use_static
#CONFIG += qt warn_on debug use_dll
#CONFIG += qt warn_on release use_dll


CPPUNIT_LIB_DIR = ../../lib   # Location of libraries


#----------------------------------------------------------------------
# MS Windows
#----------------------------------------------------------------------

win32 {
    # Suppress program database creation (should better be done
    # in the qmake spec file)
    QMAKE_CXXFLAGS_DEBUG += /Z7
    QMAKE_CXXFLAGS_DEBUG -= -Gm
    QMAKE_CXXFLAGS_DEBUG -= -Zi
}

win32 {
    use_dll {
        DEFINES += QTTESTRUNNER_DLL
        debug {
            OBJECTS_DIR = DebugDLL
            LIBS += $${CPPUNIT_LIB_DIR}\cppunitd_dll.lib
            LIBS += $${CPPUNIT_LIB_DIR}\qttestrunnerd_dll.lib
        }
        release {
            OBJECTS_DIR = ReleaseDLL
            LIBS += $${CPPUNIT_LIB_DIR}\cppunit_dll.lib
            LIBS += $${CPPUNIT_LIB_DIR}\qttestrunner_dll.lib
        }
    }
    use_static {
        debug {
            OBJECTS_DIR = Debug
            LIBS += $${CPPUNIT_LIB_DIR}\cppunitd.lib
            LIBS += $${CPPUNIT_LIB_DIR}\qttestrunnerd.lib
        }
        release {
            OBJECTS_DIR = Release
            LIBS += $${CPPUNIT_LIB_DIR}\cppunit.lib
            LIBS += $${CPPUNIT_LIB_DIR}\qttestrunner.lib
        }
    }
    DESTDIR = $${OBJECTS_DIR}
}

#----------------------------------------------------------------------
# Linux/Unix
#----------------------------------------------------------------------

unix {
    debug {
        OBJECTS_DIR = .obj_debug
        use_static: LIBS += -L$${CPPUNIT_LIB_DIR} -lqttestrunnerd
        use_dll:    LIBS += -L$${CPPUNIT_LIB_DIR} -lqttestrunnerd_shared
        LIBS += -L$${CPPUNIT_LIB_DIR} -lcppunit
    }
    release {
        OBJECTS_DIR = .obj_release
        use_static: LIBS += -L$${CPPUNIT_LIB_DIR} -lqttestrunner
        use_dll:    LIBS += -L$${CPPUNIT_LIB_DIR} -lqttestrunner_shared
        LIBS += -L$${CPPUNIT_LIB_DIR} -lcppunit
    }
}

#----------------------------------------------------------------------

HEADERS = \
        ExampleTestCases.h

SOURCES = \
        ExampleTestCases.cpp \
        Main.cpp

INCLUDEPATH += . ../../include
