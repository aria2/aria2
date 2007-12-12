bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc\
	option_processing.cc\
	version_usage.cc
SRCS =  Socket.h\
	SocketCore.cc SocketCore.h\
	Command.cc Command.h\
	AbstractCommand.cc AbstractCommand.h\
	InitiateConnectionCommandFactory.cc InitiateConnectionCommandFactory.h\
	DownloadCommand.cc DownloadCommand.h\
	HttpInitiateConnectionCommand.cc HttpInitiateConnectionCommand.h\
	HttpRequestCommand.cc HttpRequestCommand.h\
	HttpResponseCommand.cc HttpResponseCommand.h\
	HttpProxyRequestCommand.cc HttpProxyRequestCommand.h\
	HttpProxyResponseCommand.cc HttpProxyResponseCommand.h\
	HttpDownloadCommand.cc HttpDownloadCommand.h\
	HttpHeader.cc HttpHeader.h\
	HttpConnection.cc HttpConnection.h\
	FtpConnection.cc FtpConnection.h\
	FtpInitiateConnectionCommand.cc FtpInitiateConnectionCommand.h\
	FtpNegotiationCommand.cc FtpNegotiationCommand.h\
	FtpDownloadCommand.cc FtpDownloadCommand.h\
	FtpTunnelRequestCommand.cc FtpTunnelRequestCommand.h\
	FtpTunnelResponseCommand.cc FtpTunnelResponseCommand.h\
	SleepCommand.cc SleepCommand.h\
	DownloadEngine.cc DownloadEngine.h\
	Segment.h\
	GrowSegment.cc GrowSegment.h\
	PiecedSegment.cc PiecedSegment.h\
	SegmentMan.cc SegmentMan.h\
	Util.cc Util.h\
	Request.cc Request.h\
	common.h\
	message.h\
	Exception.cc Exception.h\
	FatalException.h\
	RecoverableException.h\
	DlAbortEx.h\
	DlRetryEx.h\
	Logger.h\
	SimpleLogger.cc SimpleLogger.h\
	TransferEncoding.h\
	ChunkedEncoding.cc ChunkedEncoding.h\
	DiskWriter.h\
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	DefaultDiskWriter.cc DefaultDiskWriter.h\
	File.cc File.h\
	Option.cc Option.h\
	Base64.cc\
	CookieBox.cc CookieBox.h\
	LogFactory.cc LogFactory.h\
	NullLogger.h\
	TimeA2.cc TimeA2.h\
	SharedHandle.h\
	HandleRegistry.h\
	FeatureConfig.cc FeatureConfig.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	RequestInfo.h\
	SpeedCalc.cc SpeedCalc.h\
	PeerStat.h\
	BitfieldMan.cc BitfieldMan.h\
	BitfieldManFactory.cc BitfieldManFactory.h\
	Randomizer.h\
	SimpleRandomizer.cc SimpleRandomizer.h\
	FileAllocator.h\
	HttpResponse.cc HttpResponse.h\
	HttpRequest.cc HttpRequest.h\
	Range.h\
	AbstractProxyRequestCommand.cc AbstractProxyRequestCommand.h\
	AbstractProxyResponseCommand.cc AbstractProxyResponseCommand.h\
	Netrc.cc Netrc.h\
	AuthConfig.cc AuthConfig.h\
	AuthResolver.h\
	AbstractAuthResolver.h\
	DefaultAuthResolver.cc DefaultAuthResolver.h\
	NetrcAuthResolver.cc NetrcAuthResolver.h\
	AuthConfigFactory.cc AuthConfigFactory.h\
	OptionParser.cc OptionParser.h\
	OptionHandlerFactory.cc OptionHandlerFactory.h\
	NameResolver.cc NameResolver.h\
	RequestGroup.cc RequestGroup.h\
	RequestGroupAware.cc RequestGroupAware.h\
	RequestGroupMan.cc RequestGroupMan.h\
	FileAllocationMan.cc FileAllocationMan.h\
	FileAllocationCommand.cc FileAllocationCommand.h\
	FillRequestGroupCommand.cc FillRequestGroupCommand.h\
	FileAllocationDispatcherCommand.cc FileAllocationDispatcherCommand.h\
	FileAllocationEntry.cc FileAllocationEntry.h\
	StreamFileAllocationEntry.cc StreamFileAllocationEntry.h\
	MultiUrlRequestInfo.cc MultiUrlRequestInfo.h\
	UriListParser.cc UriListParser.h\
	SegmentManFactory.h\
	AbstractSegmentManFactory.h\
	DefaultSegmentManFactory.cc DefaultSegmentManFactory.h\
	RealtimeCommand.cc RealtimeCommand.h\
	ProgressAwareEntry.h\
	RequestGroupEntry.cc RequestGroupEntry.h\
	Cookie.cc Cookie.h\
	CookieParser.cc CookieParser.h\
	CookieBoxFactory.cc CookieBoxFactory.h\
	HttpHeaderProcessor.cc HttpHeaderProcessor.h\
	FileEntry.cc FileEntry.h\
	Platform.cc Platform.h\
	PStringDatum.h\
	PStringSegment.cc PStringSegment.h\
	PStringNumLoop.h\
	PStringSelect.h\
	PStringVisitor.h\
	PStringBuildVisitor.cc PStringBuildVisitor.h\
	ParameterizedStringParser.cc ParameterizedStringParser.h\
	FixedWidthNumberDecorator.h\
	NumberDecorator.h\
	AlphaNumberDecorator.h\
	TimeBasedCommand.cc TimeBasedCommand.h\
	AutoSaveCommand.cc AutoSaveCommand.h\
	PieceStorage.h\
	DefaultPieceStorage.cc DefaultPieceStorage.h\
	UnknownLengthPieceStorage.cc UnknownLengthPieceStorage.h\
	StatCalc.h\
	ConsoleStatCalc.cc ConsoleStatCalc.h\
	TransferStat.cc TransferStat.h\
	Dependency.h\
	BtProgressInfoFile.h\
	DefaultBtProgressInfoFile.cc DefaultBtProgressInfoFile.h\
	NullProgressInfoFile.h\
	FileAllocationIterator.h\
	SingleFileAllocationIterator.cc SingleFileAllocationIterator.h\
	ContentTypeRequestGroupCriteria.cc ContentTypeRequestGroupCriteria.h\
	DownloadHandler.cc DownloadHandler.h\
	DownloadHandlerConstants.cc DownloadHandlerConstants.h\
	DownloadHandlerFactory.cc DownloadHandlerFactory.h\
	MemoryBufferPreDownloadHandler.cc MemoryBufferPreDownloadHandler.h\
	HaveEraseCommand.cc HaveEraseCommand.h\
	Piece.cc Piece.h\
	CheckIntegrityMan.cc CheckIntegrityMan.h\
	CheckIntegrityEntry.cc CheckIntegrityEntry.h\
	PieceHashCheckIntegrityEntry.cc PieceHashCheckIntegrityEntry.h\
	StreamCheckIntegrityEntry.cc StreamCheckIntegrityEntry.h\
	IteratableValidator.h\
	DiskAdaptor.cc DiskAdaptor.h\
	AbstractSingleDiskAdaptor.cc AbstractSingleDiskAdaptor.h\
	CopyDiskAdaptor.cc CopyDiskAdaptor.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	Peer.cc Peer.h\
	BtRegistry.cc BtRegistry.h\
	MultiFileAllocationIterator.cc MultiFileAllocationIterator.h\
	PeerConnection.cc PeerConnection.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	ServerHost.cc
#	debug_new.cpp

if ENABLE_MESSAGE_DIGEST
SRCS += IteratableChunkChecksumValidator.cc IteratableChunkChecksumValidator.h\
	IteratableChecksumValidator.cc IteratableChecksumValidator.h\
	CheckIntegrityCommand.cc CheckIntegrityCommand.h\
	ChecksumCheckIntegrityEntry.cc ChecksumCheckIntegrityEntry.h\
	messageDigest.cc messageDigest.h\
	MessageDigestHelper.cc MessageDigestHelper.h
endif # ENABLE_MESSAGE_DIGEST

if ENABLE_BITTORRENT
SRCS += MetaEntry.h\
	Data.cc Data.h\
	Dictionary.cc Dictionary.h\
	List.cc List.h\
	MetaFileUtil.cc MetaFileUtil.h\
	MetaEntryVisitor.h\
	ShaVisitor.cc ShaVisitor.h\
	PeerMessageUtil.cc PeerMessageUtil.h\
	PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	Peer.cc Peer.h\
	PeerListenCommand.cc PeerListenCommand.h\
	RequestSlot.cc RequestSlot.h\
	Directory.cc Directory.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	PeerChokeCommand.cc PeerChokeCommand.h\
	SeedCriteria.h\
	TimeSeedCriteria.h\
	ShareRatioSeedCriteria.h\
	UnionSeedCriteria.h\
	SeedCheckCommand.cc SeedCheckCommand.h\
	PeerListProcessor.h\
	DefaultPeerListProcessor.cc DefaultPeerListProcessor.h\
	CompactPeerListProcessor.cc CompactPeerListProcessor.h\
	DelegatingPeerListProcessor.cc DelegatingPeerListProcessor.h\
	AnnounceTier.h\
	AnnounceList.h AnnounceList.cc\
	BtContext.h\
	DefaultBtContext.cc DefaultBtContext.h\
	DefaultPeerStorage.cc DefaultPeerStorage.h\
	BtAnnounce.h\
	DefaultBtAnnounce.cc DefaultBtAnnounce.h\
	BtRuntime.h\
	BtContextAwareCommand.cc BtContextAwareCommand.h\
	BtMessage.h\
	AbstractBtMessage.h\
	SimpleBtMessage.cc SimpleBtMessage.h\
	BtAllowedFastMessage.cc BtAllowedFastMessage.h\
	BtBitfieldMessage.cc BtBitfieldMessage.h\
	BtCancelMessage.cc BtCancelMessage.h\
	BtChokeMessage.cc BtChokeMessage.h\
	BtHaveAllMessage.cc BtHaveAllMessage.h\
	BtHaveMessage.cc BtHaveMessage.h\
	BtHaveNoneMessage.cc BtHaveNoneMessage.h\
	BtInterestedMessage.cc BtInterestedMessage.h\
	BtKeepAliveMessage.cc BtKeepAliveMessage.h\
	BtNotInterestedMessage.cc BtNotInterestedMessage.h\
	BtPieceMessage.cc BtPieceMessage.h\
	BtPortMessage.cc BtPortMessage.h\
	BtRejectMessage.cc BtRejectMessage.h\
	BtRequestMessage.cc BtRequestMessage.h\
	BtSuggestPieceMessage.cc BtSuggestPieceMessage.h\
	BtUnchokeMessage.cc BtUnchokeMessage.h\
	BtHandshakeMessage.cc BtHandshakeMessage.h\
	BtMessageValidator.h\
	BtAllowedFastMessageValidator.h\
	BtBitfieldMessageValidator.h\
	BtCancelMessageValidator.h\
	BtHaveMessageValidator.h\
	BtPieceMessageValidator.h\
	BtRejectMessageValidator.h\
	BtRequestMessageValidator.h\
	BtSuggestPieceMessageValidator.h\
	BtHandshakeMessageValidator.h\
	BtMessageFactory.h\
	DefaultBtMessageFactory.cc DefaultBtMessageFactory.h\
	BtMessageDispatcher.h\
	DefaultBtMessageDispatcher.cc DefaultBtMessageDispatcher.h\
	BtMessageReceiver.h\
	DefaultBtMessageReceiver.cc DefaultBtMessageReceiver.h\
	BtRequestFactory.h\
	DefaultBtRequestFactory.cc DefaultBtRequestFactory.h\
	BtEvent.h\
	BtEventListener.h\
	BtCancelSendingPieceEvent.h\
	BtAbortOutstandingRequestEvent.h\
	BtChokedEvent.h\
	BtChokingEvent.h\
	BtInteractive.h\
	DefaultBtInteractive.cc DefaultBtInteractive.h\
	PeerObject.h\
	ActivePeerConnectionCommand.cc ActivePeerConnectionCommand.h\
	BtDependency.cc BtDependency.h\
	PeerReceiveHandshakeCommand.cc PeerReceiveHandshakeCommand.h\
	BtSetup.cc BtSetup.h\
	BtFileAllocationEntry.cc BtFileAllocationEntry.h\
	BtPostDownloadHandler.cc BtPostDownloadHandler.h\
	BtCheckIntegrityEntry.cc BtCheckIntegrityEntry.h
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
SRCS += Metalinker.cc Metalinker.h\
	MetalinkEntry.cc MetalinkEntry.h\
	MetalinkResource.cc MetalinkResource.h\
	MetalinkProcessor.h\
	MetalinkParserController.cc\
	MetalinkParserStateMachine.cc\
	InitialMetalinkParserState.cc\
	MetalinkMetalinkParserState.cc\
	FilesMetalinkParserState.cc\
	FileMetalinkParserState.cc\
	SizeMetalinkParserState.cc\
	VersionMetalinkParserState.cc\
	LanguageMetalinkParserState.cc\
	OSMetalinkParserState.cc\
	VerificationMetalinkParserState.cc\
	HashMetalinkParserState.cc\
	PiecesMetalinkParserState.cc\
	PieceHashMetalinkParserState.cc\
	ResourcesMetalinkParserState.cc\
	URLMetalinkParserState.cc\
	FinMetalinkParserState.cc\
	SkipTagMetalinkParserState.cc\
	XML2SAXMetalinkProcessor.cc\
	Metalink2RequestGroup.cc Metalink2RequestGroup.h\
	MetalinkPostDownloadHandler.cc MetalinkPostDownloadHandler.h\
	MetalinkHelper.cc MetalinkHelper.h
endif # ENABLE_METALINK

if !HAVE_BASENAME
SRCS += libgen.c libgen.h
endif # !HAVE_BASENAME

if !HAVE_GETADDRINFO
SRCS += getaddrinfo.c getaddrinfo.h
endif # !HAVE_GETADDRINFO

if !HAVE_GAI_STRERROR
SRCS += gai_strerror.c gai_strerror.h
endif # !HAVE_GAI_STRERROR

if !HAVE_GETTIMEOFDAY
SRCS += gettimeofday.c gettimeofday.h
endif # !HAVE_GETTIMEOFDAY

if !HAVE_GETTIMEOFDAY
SRCS += gettimeofday.c gettimeofday.h
endif # !HAVE_GETTIMEOFDAY

if !HAVE_INET_ATON
SRCS += inet_aton.c inet_aton.h
endif # !HAVE_INET_ATON

if !HAVE_LOCALTIME_R
SRCS += localtime_r.c localtime_r.h
endif # !HAVE_LOCALTIME_R

if !HAVE_STRPTIME
SRCS += strptime.c strptime.h
endif # !HAVE_STRPTIME

if !HAVE_TIMEGM
SRCS += timegm.c timegm.h
endif # !HAVE_TIMEGM

noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a @LIBINTL@ @ALLOCA@ @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@ @LIBARES_LIBS@\
	@LIBCARES_LIBS@ @WINSOCK_LIBS@ #-lprofiler
#aria2c_LDFLAGS =  -pg
AM_CPPFLAGS =  -Wall\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBARES_CPPFLAGS@ @LIBCARES_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@ #-pg