#----------------------------------------------------------------------
# File:    qttestrunnerlib.pro
# Purpose: qmake config file for the QtTestRunner library.
#          The library is built as debug staticlib. Set the CONFIG
#          variable accordingly to build it differently.
#----------------------------------------------------------------------

TEMPLATE = lib
LANGUAGE = C++

# Get rid of possibly predefined options

CONFIG -= debug
CONFIG -= release
CONFIG -= dll
CONFIG -= staticlib

CONFIG += qt warn_on debug staticlib

#CONFIG += qt warn_on release staticlib
#CONFIG += qt warn_on debug dll
#CONFIG += qt warn_on release dll


QTRUNNER_LIB = qttestrunner   # Name of the library


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
    MOC_DIR = tmp\moc
    UI_DIR = tmp\moc
    dll {
        DEFINES += QTTESTRUNNER_DLL_BUILD
        DLLDESTDIR = ..\..\lib
        debug {
            TARGET = $${QTRUNNER_LIB}d_dll
            QTRUNNER_IMPORTLIB = $${QTRUNNER_LIB}d_dll.lib
            OBJECTS_DIR = DebugDLL
            LIBS += ..\..\lib\cppunitd_dll.lib
        }
        release {
            TARGET = $${QTRUNNER_LIB}_dll
            QTRUNNER_IMPORTLIB = $${QTRUNNER_LIB}_dll.lib
            OBJECTS_DIR = ReleaseDLL
            LIBS += ..\..\lib\cppunit_dll.lib
        }
        DESTDIR = $${OBJECTS_DIR}
        QMAKE_CLEAN += $${QTRUNNER_IMPORTLIB}
        
        # Also copy the import library after build of the DLL       
        QTRUNNER_IMPORTLIB = $${DESTDIR}-SEP-$${QTRUNNER_IMPORTLIB}
        QTRUNNER_IMPORTLIB ~= s/-SEP-/\/
        QMAKE_POST_LINK = copy $${QTRUNNER_IMPORTLIB} $${DLLDESTDIR}
    }
    staticlib {
        DESTDIR = ..\..\lib
        debug {
            TARGET = $${QTRUNNER_LIB}d
            OBJECTS_DIR = Debug
        }
        release {
            TARGET = $${QTRUNNER_LIB}
            OBJECTS_DIR = Release
        }
    }
}

#----------------------------------------------------------------------
# Linux/Unix
#----------------------------------------------------------------------

unix {
    MOC_DIR = .moc
    UI_DIR = .moc
    DESTDIR = ../../lib
    dll {
        debug {
            TARGET = $${QTRUNNER_LIB}d_shared
            OBJECTS_DIR = .obj_debug_shared
            LIBS += -L../../lib -lcppunit
        }
        release {
            TARGET = $${QTRUNNER_LIB}_shared
            OBJECTS_DIR = .obj_release_shared
            LIBS += -L../../lib -lcppunit
        }
    }
    staticlib {
        debug {
            TARGET = $${QTRUNNER_LIB}d
            OBJECTS_DIR = .obj_debug
        }
        release {
            TARGET = $${QTRUNNER_LIB}
            OBJECTS_DIR = .obj_release
        }
    }
}

#----------------------------------------------------------------------

HEADERS = \
        MostRecentTests.h \
        TestBrowserDlgImpl.h \
        TestFailureInfo.h \
        TestFailureListViewItem.h \
        TestListViewItem.h \
        TestRunnerDlgImpl.h \
        TestRunnerFailureEvent.h \
        TestRunnerModel.h \
        TestRunnerModelThreadInterface.h \
        TestRunnerTestCaseRunEvent.h \
        TestRunnerThread.h \
        TestRunnerThreadEvent.h \
        TestRunnerThreadFinishedEvent.h \
        ../../include/cppunit/ui/qt/TestRunner.h

SOURCES = \
        MostRecentTests.cpp \
        TestBrowserDlgImpl.cpp \
        TestFailureInfo.cpp \
        TestFailureListViewItem.cpp \
        TestListViewItem.cpp \
        QtTestRunner.cpp \
        TestRunnerDlgImpl.cpp \
        TestRunnerFailureEvent.cpp \
        TestRunnerModel.cpp \
        TestRunnerModelThreadInterface.cpp \
        TestRunnerTestCaseRunEvent.cpp \
        TestRunnerThread.cpp \
        TestRunnerThreadEvent.cpp \
        TestRunnerThreadFinishedEvent.cpp

INTERFACES = \
        testbrowserdlg.ui \
        testrunnerdlg.ui

INCLUDEPATH += . ../../include
