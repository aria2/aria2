TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
	FileUriListParserTest.cc\
	StreamUriListParserTest.cc\
	HttpHeaderProcessorTest.cc\
	UtilTest.cc\
	CookieBoxTest.cc\
	RequestTest.cc\
	CookieParserTest.cc\
	HttpRequestTest.cc\
	CookieBoxFactoryTest.cc\
	ByteArrayDiskWriterTest.cc\
	RequestGroupManTest.cc\
	IteratableChecksumValidatorTest.cc\
	IteratableChunkChecksumValidatorTest.cc\
	PeerTest.cc\
	DefaultPeerStorageTest.cc\
	RequestFactoryTest.cc\
	NetrcAuthResolverTest.cc\
	DefaultAuthResolverTest.cc\
	OptionHandlerTest.cc\
	SegmentManTest.cc\
	BitfieldManTest.cc\
	GlowFileAllocatorTest.cc\
	NetrcTest.cc\
	SingletonHolderTest.cc\
	HttpHeaderTest.cc\
	HttpResponseTest.cc\
	SharedHandleTest.cc\
	ChunkedEncodingTest.cc\
	FileTest.cc\
	OptionTest.cc\
	Base64Test.cc\
	DataTest.cc\
	DictionaryTest.cc\
	ListTest.cc\
	MetaFileUtilTest.cc\
	PeerMessageUtilTest.cc\
	DefaultDiskWriterTest.cc\
	MultiDiskAdaptorTest.cc\
	FeatureConfigTest.cc\
	ShareRatioSeedCriteriaTest.cc\
	TimeSeedCriteriaTest.cc\
	SpeedCalcTest.cc\
	DefaultPeerListProcessorTest.cc\
	AnnounceListTest.cc\
	TrackerWatcherCommandTest.cc\
	MockPeerStorage.h\
	FixedNumberRandomizer.h\
	ConsoleFileAllocationMonitorTest.cc\
	ChunkChecksumValidatorTest.cc
	
if ENABLE_BITTORRENT
aria2c_SOURCES += BtAllowedFastMessageTest.cc\
	BtBitfieldMessageTest.cc\
	BtCancelMessageTest.cc\
	BtChokeMessageTest.cc\
	BtHandshakeMessageTest.cc\
	BtHaveAllMessageTest.cc\
	BtHaveMessageTest.cc\
	BtHaveNoneMessageTest.cc\
	BtInterestedMessageTest.cc\
	BtKeepAliveMessageTest.cc\
	BtNotInterestedMessageTest.cc\
	BtPieceMessageTest.cc\
	BtPortMessageTest.cc\
	BtRegistryTest.cc\
	BtRejectMessageTest.cc\
	BtRequestMessageTest.cc\
	BtSuggestPieceMessageTest.cc\
	BtUnchokeMessageTest.cc\
	DefaultBtAnnounceTest.cc\
	DefaultBtContextTest.cc\
	DefaultBtMessageDispatcherTest.cc\
	DefaultBtRequestFactoryTest.cc\
	DefaultPieceStorageTest.cc\
	MockBtMessage.h\
	MockBtMessageDispatcher.h\
	MockBtMessageFactory.h\
	ShaVisitorTest.cc
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
aria2c_SOURCES += MetalinkerTest.cc\
	MetalinkEntryTest.cc\
	Xml2MetalinkProcessorTest.cc
endif # ENABLE_METALINK


#aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -I../lib -Wall -D_FILE_OFFSET_BITS=64
#aria2c_LDFLAGS = ${CPPUNIT_LIBS}

aria2c_LDADD = ../src/libaria2c.a\
    @LIBINTL@ @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@ @LIBARES_LIBS@\
	@LIBCARES_LIBS@ @WINSOCK_LIBS@\
	${CPPUNIT_LIBS}
AM_CPPFLAGS =  -Wall\
	${CPPUNIT_CFLAGS}\
	-I ../src\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBARES_CPPFLAGS@ @LIBCARES_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@
