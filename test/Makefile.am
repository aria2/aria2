TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
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
	UriFileListParserTest.cc\
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
	ShaVisitorTest.cc\
	PeerMessageUtilTest.cc\
	DefaultDiskWriterTest.cc\
	MultiDiskAdaptorTest.cc\
	Xml2MetalinkProcessorTest.cc\
	MetalinkerTest.cc\
	MetalinkEntryTest.cc\
	FeatureConfigTest.cc\
	ShareRatioSeedCriteriaTest.cc\
	TimeSeedCriteriaTest.cc\
	SpeedCalcTest.cc\
	DefaultPeerListProcessorTest.cc\
	AnnounceListTest.cc\
	TrackerWatcherCommandTest.cc\
	DefaultBtContextTest.cc\
	DefaultPieceStorageTest.cc\
	DefaultBtAnnounceTest.cc\
	BtRegistryTest.cc\
	DefaultBtMessageDispatcherTest.cc\
	MockPeerStorage.h\
	DefaultBtRequestFactoryTest.cc\
	BtAllowedFastMessageTest.cc\
	BtBitfieldMessageTest.cc\
	BtCancelMessageTest.cc\
	BtChokeMessageTest.cc\
	BtHaveAllMessageTest.cc\
	BtHaveMessageTest.cc\
	BtHaveNoneMessageTest.cc\
	BtInterestedMessageTest.cc\
	BtKeepAliveMessageTest.cc\
	BtNotInterestedMessageTest.cc\
	BtPieceMessageTest.cc\
	BtPortMessageTest.cc\
	BtRejectMessageTest.cc\
	BtRequestMessageTest.cc\
	BtSuggestPieceMessageTest.cc\
	BtUnchokeMessageTest.cc\
	BtHandshakeMessageTest.cc\
	MockBtMessageDispatcher.h\
	FixedNumberRandomizer.h\
	MockBtMessageFactory.h\
	MockBtMessage.h\
	ConsoleFileAllocationMonitorTest.cc\
	ChunkChecksumValidatorTest.cc
#aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -I../lib -Wall -D_FILE_OFFSET_BITS=64
#aria2c_LDFLAGS = ${CPPUNIT_LIBS}

aria2c_LDADD = ../src/libaria2c.a\
	 ${CPPUNIT_LIBS} @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@\
	@LIBARES_LIBS@ @LIBCARES_LIBS@
AM_CPPFLAGS =  -Wall\
	${CPPUNIT_CFLAGS}\
	-I ../src\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBARES_CPPFLAGS@ @LIBCARES_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@
