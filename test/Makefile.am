TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
	RequestTest.cc\
	ChunkedEncodingTest.cc\
	FileTest.cc\
	OptionTest.cc\
	Base64Test.cc\
	UtilTest.cc\
	CookieBoxTest.cc\
	DataTest.cc\
	DictionaryTest.cc\
	ListTest.cc\
	MetaFileUtilTest.cc\
	ShaVisitorTest.cc\
	TorrentManTest.cc\
	PeerMessageUtilTest.cc\
	BitfieldManTest.cc\
	DefaultDiskWriterTest.cc
aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -I../lib -Wall -D_FILE_OFFSET_BITS=64
aria2c_LDFLAGS = ${CPPUNIT_LIBS}
aria2c_LDADD = ../src/libaria2c.a