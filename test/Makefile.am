a2_test_outdir = test_outdir
TESTS = aria2c
check_PROGRAMS = $(TESTS)
aria2c_SOURCES = AllTest.cc\
	TestUtil.cc TestUtil.h\
	SocketCoreTest.cc\
	array_funTest.cc\
	Base64Test.cc\
	Base32Test.cc\
	a2functionalTest.cc\
	FileEntryTest.cc\
	PieceTest.cc\
	SegmentTest.cc\
	GrowSegmentTest.cc\
	SingleFileAllocationIteratorTest.cc\
	DefaultBtProgressInfoFileTest.cc\
	RequestGroupTest.cc\
	UtilTest1.cc\
	UtilTest2.cc\
	UtilSecurityTest.cc\
	UriListParserTest.cc\
	HttpHeaderProcessorTest.cc\
	RequestTest.cc\
	HttpRequestTest.cc\
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
	FileTest.cc\
	OptionTest.cc\
	DefaultDiskWriterTest.cc\
	FeatureConfigTest.cc\
	SpeedCalcTest.cc\
	MultiDiskAdaptorTest.cc\
	MultiFileAllocationIteratorTest.cc\
	FixedNumberRandomizer.h\
	ProtocolDetectorTest.cc\
	ExceptionTest.cc\
	FmtTest.cc\
	DownloadHandlersTest.cc\
	SignatureTest.cc\
	ServerStatManTest.cc\
	FeedbackURISelectorTest.cc\
	InorderURISelectorTest.cc\
	ServerStatTest.cc\
	NsCookieParserTest.cc\
	DirectDiskAdaptorTest.cc\
	CookieTest.cc\
	CookieStorageTest.cc\
	TimeTest.cc\
	FtpConnectionTest.cc\
	OptionParserTest.cc\
	DNSCacheTest.cc\
	DownloadHelperTest.cc\
	SequentialPickerTest.cc\
	RarestPieceSelectorTest.cc\
	PieceStatManTest.cc\
	InorderPieceSelector.h\
	LongestSequencePieceSelectorTest.cc\
	a2algoTest.cc\
	bitfieldTest.cc\
	DownloadContextTest.cc\
	SessionSerializerTest.cc\
	ValueBaseTest.cc\
	ChunkedDecodingStreamFilterTest.cc\
	UriTest.cc\
	UriSplitTest.cc\
	MockSegment.h\
	CookieHelperTest.cc\
	JsonTest.cc\
	ValueBaseJsonParserTest.cc\
	RpcResponseTest.cc\
	RpcMethodTest.cc\
	HttpServerTest.cc\
	BufferedFileTest.cc\
	GeomStreamPieceSelectorTest.cc\
	SegListTest.cc\
	ParamedStringTest.cc\
	RpcHelperTest.cc\
	AbstractCommandTest.cc\
	SinkStreamFilterTest.cc\
	WrDiskCacheTest.cc\
	WrDiskCacheEntryTest.cc\
	GroupIdTest.cc\
	IndexedListTest.cc \
	SimpleRandomizerTest.cc

if ENABLE_XML_RPC
aria2c_SOURCES += XmlRpcRequestParserControllerTest.cc
endif # ENABLE_XML_RPC

if HAVE_SOME_FALLOCATE
aria2c_SOURCES += FallocFileAllocationIteratorTest.cc
endif  # HAVE_SOME_FALLOCATE

if HAVE_ZLIB
aria2c_SOURCES += \
	GZipDecoder.cc GZipDecoder.h\
	GZipDecoderTest.cc GZipEncoderTest.cc\
	GZipDecodingStreamFilterTest.cc\
	GZipFileTest.cc
endif # HAVE_ZLIB

if HAVE_SQLITE3
aria2c_SOURCES += Sqlite3CookieParserTest.cc
endif # HAVE_SQLITE3

aria2c_SOURCES += MessageDigestHelperTest.cc\
	IteratableChunkChecksumValidatorTest.cc\
	IteratableChecksumValidatorTest.cc\
	MessageDigestTest.cc

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
	DefaultBtMessageDispatcherTest.cc\
	DefaultBtRequestFactoryTest.cc\
	MockBtMessage.h\
	MockBtMessageDispatcher.h\
	MockBtMessageFactory.h\
	AnnounceListTest.cc\
	DefaultPeerStorageTest.cc\
	MockPeerStorage.h\
	ByteArrayDiskWriterTest.cc\
	PeerTest.cc\
	PeerSessionResourceTest.cc\
	ShareRatioSeedCriteriaTest.cc\
	BtRegistryTest.cc\
	BtDependencyTest.cc\
	BtPostDownloadHandlerTest.cc\
	TimeSeedCriteriaTest.cc\
	BtExtendedMessageTest.cc\
	HandshakeExtensionMessageTest.cc\
	UTPexExtensionMessageTest.cc\
	UTMetadataRequestExtensionMessageTest.cc\
	UTMetadataDataExtensionMessageTest.cc\
	UTMetadataRejectExtensionMessageTest.cc\
	UTMetadataRequestTrackerTest.cc\
	UTMetadataRequestFactoryTest.cc\
	UTMetadataPostDownloadHandlerTest.cc\
	MagnetTest.cc\
	DefaultBtMessageFactoryTest.cc\
	DefaultExtensionMessageFactoryTest.cc\
	DHTNodeTest.cc\
	DHTBucketTest.cc\
	DHTRoutingTableTest.cc\
	DHTMessageTrackerEntryTest.cc\
	DHTMessageTrackerTest.cc\
	DHTConnectionImplTest.cc\
	DHTPingMessageTest.cc\
	DHTPingReplyMessageTest.cc\
	DHTFindNodeMessageTest.cc\
	DHTFindNodeReplyMessageTest.cc\
	DHTGetPeersMessageTest.cc\
	DHTGetPeersReplyMessageTest.cc\
	DHTAnnouncePeerMessageTest.cc\
	DHTAnnouncePeerReplyMessageTest.cc\
	DHTUnknownMessageTest.cc\
	DHTMessageFactoryImplTest.cc\
	DHTBucketTreeTest.cc\
	DHTPeerAnnounceEntryTest.cc\
	DHTPeerAnnounceStorageTest.cc\
	DHTTokenTrackerTest.cc\
	XORCloserTest.cc\
	DHTIDCloserTest.cc\
	DHTRoutingTableSerializerTest.cc\
	DHTRoutingTableDeserializerTest.cc\
	DHTTaskExecutorTest.cc\
	DHKeyExchangeTest.cc\
	ARC4Test.cc\
	MSEHandshakeTest.cc\
	MockBtAnnounce.h\
	MockBtProgressInfoFile.h\
	MockBtRequestFactory.h\
	MockDHTMessage.h\
	MockDHTMessageCallback.h\
	MockDHTMessageDispatcher.h\
	MockDHTMessageFactory.h\
	MockDHTTask.h\
	MockDHTTaskFactory.h\
	MockDHTTaskQueue.h\
	MockExtensionMessage.h\
	MockExtensionMessageFactory.h\
	MockPieceStorage.h\
	BittorrentHelperTest.cc\
	PriorityPieceSelectorTest.cc\
	MockPieceSelector.h\
	extension_message_test_helper.h\
	LpdMessageDispatcherTest.cc\
	LpdMessageReceiverTest.cc\
	Bencode2Test.cc\
	PeerConnectionTest.cc\
	ValueBaseBencodeParserTest.cc\
	ExtensionMessageRegistryTest.cc\
	UDPTrackerClientTest.cc
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

if ENABLE_ASYNC_DNS
aria2c_SOURCES += AsyncNameResolverTest.cc
endif # ENABLE_ASYNC_DNS

if !HAVE_TIMEGM
aria2c_SOURCES += TimegmTest.cc
endif # !HAVE_TIMEGM

if ENABLE_LIBARIA2
aria2c_SOURCES += Aria2ApiTest.cc
endif # ENABLE_LIBARIA2

aria2c_LDADD = \
	../src/libaria2.la \
	@LIBINTL@ \
	@EXTRALIBS@ \
	@ZLIB_LIBS@ \
	@LIBUV_LIBS@ \
	@LIBXML2_LIBS@ \
	@EXPAT_LIBS@ \
	@SQLITE3_LIBS@ \
	@WINTLS_LIBS@ \
	@LIBGNUTLS_LIBS@ \
	@OPENSSL_LIBS@ \
	@LIBNETTLE_LIBS@ \
	@LIBGMP_LIBS@ \
	@LIBGCRYPT_LIBS@ \
	@LIBSSH2_LIBS@ \
	@LIBCARES_LIBS@ \
	@WSLAY_LIBS@ \
	@CPPUNIT_LIBS@ \
	@TCMALLOC_LIBS@ \
	@JEMALLOC_LIBS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/includes -I$(top_builddir)/src/includes \
	-I$(top_srcdir)/lib -I$(top_srcdir)/intl \
        -DLOCALEDIR=\"$(localedir)\" \
	-DA2_TEST_DIR=\"$(top_srcdir)/test\" \
	-DA2_TEST_OUT_DIR=\"${a2_test_outdir}\" \
	@CPPUNIT_CFLAGS@ \
	@DEFS@ \
	@EXTRACPPFLAGS@ \
	@ZLIB_CFLAGS@ \
	@LIBUV_CFLAGS@ \
	@LIBXML2_CFLAGS@ \
	@EXPAT_CFLAGS@ \
	@SQLITE3_CFLAGS@ \
	@LIBGNUTLS_CFLAGS@ \
	@OPENSSL_CFLAGS@ \
	@LIBNETTLE_CFLAGS@ \
	@LIBGMP_CFLAGS@ \
	@LIBGCRYPT_CFLAGS@ \
	@LIBSSH2_CFLAGS@ \
	@LIBCARES_CFLAGS@ \
	@WSLAY_CFLAGS@ \
	@TCMALLOC_CFLAGS@ \
	@JEMALLOC_CFLAGS@

AM_LDFLAGS = \
	@EXTRALDFLAGS@ \
	@APPLETLS_LDFLAGS@

AM_CFLAGS = @EXTRACFLAGS@

AM_CXXFLAGS = @WARNCXXFLAGS@ @CXX1XCXXFLAGS@ @EXTRACXXFLAGS@

EXTRA_DIST = 4096chunk.txt\
	chunkChecksumTestFile250.txt\
	cookies.sqlite\
	chromium_cookies.sqlite\
	emptyfile\
	file1r.txt\
	file2r.txt\
	file3r.txt\
	filelist1.txt\
	filelist2.txt\
	gzip_decode_test.gz\
	load-nonBt.aria2\
	load-nonBt-v0001.aria2\
	load.aria2\
	load-v0001.aria2\
	malformed.netrc\
	nscookietest.txt\
	sample.netrc\
	single.torrent\
	test.torrent\
	test.xml\
	url-list-multiFile.torrent\
	url-list-singleFile.torrent\
	url-list-singleFileEndsWithSlash.torrent\
	input_uris.txt\
	2files.metalink\
	utf8.torrent\
	metalink4.xml\
	metalink3-dirtraversal.xml\
	metalink4-groupbymetaurl.xml\
	serialize_session.meta4\
	metalink4-dosdirtraversal.xml\
	base_uri.xml\
	local-metaurl.meta4

clean-local:
	-rm -rf ${a2_test_outdir}
