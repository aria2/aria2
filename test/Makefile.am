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
	DefaultDiskWriterTest.cc\
	MultiDiskWriterTest.cc\
	ChokeMessageTest.cc\
	UnchokeMessageTest.cc\
	HaveAllMessageTest.cc\
	HaveNoneMessageTest.cc\
	InterestedMessageTest.cc\
	NotInterestedMessageTest.cc\
	HaveMessageTest.cc\
	BitfieldMessageTest.cc\
	RequestMessageTest.cc\
	CancelMessageTest.cc\
	PieceMessageTest.cc\
	RejectMessageTest.cc\
	AllowedFastMessageTest.cc\
	SuggestPieceMessageTest.cc\
	Xml2MetalinkProcessorTest.cc\
	MetalinkerTest.cc\
	MetalinkEntryTest.cc\
	FeatureConfigTest.cc\
	ShareRatioSeedCriteriaTest.cc\
	TimeSeedCriteriaTest.cc\
	SegmentManTest.cc\
	SpeedCalcTest.cc\
	DefaultPeerListProcessorTest.cc\
	AnnounceListTest.cc
#aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -I../lib -Wall -D_FILE_OFFSET_BITS=64
#aria2c_LDFLAGS = ${CPPUNIT_LIBS}

aria2c_LDADD = ../src/libaria2c.a\
	 ${CPPUNIT_LIBS} @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@
AM_CPPFLAGS =  -Wall\
	${CPPUNIT_CFLAGS}\
	-I ../src\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@
