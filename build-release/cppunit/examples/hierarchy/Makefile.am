EXTRA_DIST = hierarchy.dsw hierarchy.dsp

TESTS = hierarchy
XFAIL_TESTS = hierarchy

check_PROGRAMS = $(TESTS)

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

hierarchy_SOURCES= BoardGame.cpp \
                   Chess.cpp \
                   main.cpp \
                   BoardGame.h \
                   Chess.h \
                   BoardGameTest.h \
                   ChessTest.h

hierarchy_LDADD= \
  $(top_builddir)/src/cppunit/libcppunit.la 

hierarchy_LDFLAGS = $(LIBADD_DL)
