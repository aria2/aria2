TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
	RequestTest.cc\
	ChunkedEncodingTest.cc\
	FileTest.cc\
	OptionTest.cc\
	Base64Test.cc\
	UtilTest.cc\
	CookieBoxTest.cc
aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -Wall -D_FILE_OFFSET_BITS=64
aria2c_LDFLAGS = ${CPPUNIT_LIBS}
aria2c_LDADD = ../src/libaria2c.a