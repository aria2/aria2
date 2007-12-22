TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
	Base64Test.cc\
	SequenceTest.cc\
	a2functionalTest.cc\
	FileEntryTest.cc\
	PieceTest.cc\
	SegmentTest.cc\
	GrowSegmentTest.cc\
	SingleFileAllocationIteratorTest.cc\
	DefaultBtProgressInfoFileTest.cc\
	SingleFileDownloadContextTest.cc\
	RequestGroupTest.cc\
	PStringBuildVisitorTest.cc\
	ParameterizedStringParserTest.cc\
	UtilTest.cc\
	AlphaNumberDecoratorTest.cc\
	FileUriListParserTest.cc\
	StreamUriListParserTest.cc\
	HttpHeaderProcessorTest.cc\
	CookieBoxTest.cc\
	RequestTest.cc\
	CookieParserTest.cc\
	HttpRequestTest.cc\
	CookieBoxFactoryTest.cc\
	RequestGroupManTest.cc\
	AuthConfigFactoryTest.cc\
	NetrcAuthResolverTest.cc\
	DefaultAuthResolverTest.cc\
	OptionHandlerTest.cc\
	SegmentManTest.cc\
	BitfieldManTest.cc\
	NetrcTest.cc\
	SingletonHolderTest.cc\
	HttpHeaderTest.cc\
	HttpResponseTest.cc\
	SharedHandleTest.cc\
	ChunkedEncodingTest.cc\
	FileTest.cc\
	OptionTest.cc\
	DefaultDiskWriterTest.cc\
	FeatureConfigTest.cc\
	SpeedCalcTest.cc\
	FixedNumberRandomizer.h\
	MultiDiskAdaptorTest.cc\
	MultiFileAllocationIteratorTest.cc

if ENABLE_MESSAGE_DIGEST
aria2c_SOURCES += MessageDigestHelperTest.cc\
	IteratableChunkChecksumValidatorTest.cc\
	IteratableChecksumValidatorTest.cc
endif # ENABLE_MESSAGE_DIGEST

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
	BtRejectMessageTest.cc\
	BtRequestMessageTest.cc\
	BtSuggestPieceMessageTest.cc\
	BtUnchokeMessageTest.cc\
	DefaultPieceStorageTest.cc\
	DefaultBtAnnounceTest.cc\
	DefaultBtContextTest.cc\
	DefaultBtMessageDispatcherTest.cc\
	DefaultBtRequestFactoryTest.cc\
	MockBtMessage.h\
	MockBtMessageDispatcher.h\
	MockBtMessageFactory.h\
	ShaVisitorTest.cc\
	DefaultPeerListProcessorTest.cc\
	AnnounceListTest.cc\
	DefaultPeerStorageTest.cc\
	MockPeerStorage.h\
	DataTest.cc\
	DictionaryTest.cc\
	ListTest.cc\
	MetaFileUtilTest.cc\
	ByteArrayDiskWriterTest.cc\
	PeerTest.cc\
	PeerMessageUtilTest.cc\
	ShareRatioSeedCriteriaTest.cc\
	BtRegistryTest.cc\
	BtDependencyTest.cc\
	BtPostDownloadHandlerTest.cc\
	DownloadHandlerFactoryTest.cc\
	TimeSeedCriteriaTest.cc\
	BencodeVisitorTest.cc\
	BtExtendedMessageTest.cc\
	HandshakeExtensionMessageTest.cc\
	UTPexExtensionMessageTest.cc\
	DefaultBtMessageFactoryTest.cc\
	DefaultExtensionMessageFactoryTest.cc
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
aria2c_SOURCES += MetalinkerTest.cc\
	MetalinkEntryTest.cc\
	Metalink2RequestGroupTest.cc\
	MetalinkPostDownloadHandlerTest.cc\
	MetalinkHelperTest.cc\
	MetalinkParserControllerTest.cc\
	MetalinkProcessorTest.cc
endif # ENABLE_METALINK

#aria2c_CXXFLAGS = ${CPPUNIT_CFLAGS} -I../src -I../lib -Wall -D_FILE_OFFSET_BITS=64
#aria2c_LDFLAGS = ${CPPUNIT_LIBS}

aria2c_LDADD = ../src/libaria2c.a\
    @LIBINTL@ @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@ @LIBARES_LIBS@\
	@LIBCARES_LIBS@ @WINSOCK_LIBS@ @LIBEXPAT_LIBS@\
	${CPPUNIT_LIBS}
AM_CPPFLAGS =  -Wall\
	${CPPUNIT_CFLAGS}\
	-I ../src\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBARES_CPPFLAGS@ @LIBCARES_CPPFLAGS@ @LIBEXPAT_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@
