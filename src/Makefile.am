bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc\
	option_processing.cc\
	version_usage.cc\
	download_helper.cc download_helper.h
SRCS =  Socket.h\
	SocketCore.cc SocketCore.h\
	BinaryStream.h\
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
	DownloadFailureException.h\
	Logger.h\
	SimpleLogger.cc SimpleLogger.h\
	DiskWriter.h\
	DiskWriterFactory.h\
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	DefaultDiskWriter.cc DefaultDiskWriter.h\
	DefaultDiskWriterFactory.cc DefaultDiskWriterFactory.h\
	File.cc File.h\
	Option.cc Option.h\
	Base64.cc Base64.h\
	LogFactory.cc LogFactory.h\
	NullLogger.h\
	TimeA2.cc TimeA2.h\
	SharedHandle.h\
	HandleRegistry.h\
	FeatureConfig.cc FeatureConfig.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	SpeedCalc.cc SpeedCalc.h\
	PeerStat.h\
	BitfieldMan.cc BitfieldMan.h\
	BitfieldManFactory.cc BitfieldManFactory.h\
	Randomizer.h\
	SimpleRandomizer.cc SimpleRandomizer.h\
	HttpResponse.cc HttpResponse.h\
	HttpRequest.cc HttpRequest.h\
	Range.h\
	AbstractProxyRequestCommand.cc AbstractProxyRequestCommand.h\
	AbstractProxyResponseCommand.cc AbstractProxyResponseCommand.h\
	Netrc.cc Netrc.h\
	AuthConfig.cc AuthConfig.h\
	AuthResolver.h\
	AbstractAuthResolver.cc AbstractAuthResolver.h\
	DefaultAuthResolver.cc DefaultAuthResolver.h\
	NetrcAuthResolver.cc NetrcAuthResolver.h\
	AuthConfigFactory.cc AuthConfigFactory.h\
	OptionParser.cc OptionParser.h\
	OptionHandlerFactory.cc OptionHandlerFactory.h\
	OptionHandler.cc OptionHandler.h\
	OptionHandlerImpl.h\
	NameMatchOptionHandler.h\
	NameResolver.cc NameResolver.h\
	RequestGroup.cc RequestGroup.h\
	RequestGroupAware.cc RequestGroupAware.h\
	RequestGroupMan.cc RequestGroupMan.h\
	FileAllocationMan.h\
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
	RequestGroupCriteria.h\
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
	Peer.cc\
	PeerSessionResource.cc PeerSessionResource.h\
	BtRegistry.cc BtRegistry.h\
	MultiFileAllocationIterator.cc MultiFileAllocationIterator.h\
	PeerConnection.cc PeerConnection.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	ByteArrayDiskWriterFactory.cc ByteArrayDiskWriterFactory.h\
	ServerHost.cc ServerHost.h\
	DownloadContext.cc DownloadContext.h\
	SingleFileDownloadContext.cc SingleFileDownloadContext.h\
	TimedHaltCommand.cc TimedHaltCommand.h\
	CUIDCounter.h\
	DNSCache.h\
	DownloadResult.h\
	Sequence.h\
	IntSequence.h\
	PostDownloadHandler.h\
	PreDownloadHandler.h\
	SingletonHolder.h\
	TrueRequestGroupCriteria.h\
	a2algo.h\
	a2functional.h\
	a2io.h\
	a2netcompat.h\
	a2time.h\
	array_fun.h\
	help_tags.h\
	prefs.cc prefs.h\
	usage_text.h\
	ProtocolDetector.cc ProtocolDetector.h\
	NullStatCalc.h\
	StringFormat.cc StringFormat.h\
	HttpSkipResponseCommand.cc HttpSkipResponseCommand.h\
	InitiateConnectionCommand.cc InitiateConnectionCommand.h\
	FtpFinishDownloadCommand.cc FtpFinishDownloadCommand.h\
	A2STR.cc A2STR.h\
	RarestPieceSelector.cc RarestPieceSelector.h\
	Decoder.h\
	ChunkedDecoder.cc ChunkedDecoder.h\
	Signature.cc Signature.h\
	ServerStat.cc ServerStat.h\
	ServerStatMan.cc ServerStatMan.h\
	URISelector.h\
	AdaptiveURISelector.cc AdaptiveURISelector.h\
	InOrderURISelector.cc InOrderURISelector.h\
	ServerStatURISelector.cc ServerStatURISelector.h\
	NsCookieParser.cc NsCookieParser.h\
	CookieStorage.cc CookieStorage.h\
	SocketBuffer.cc SocketBuffer.h\
	OptionHandlerException.cc OptionHandlerException.h\
	bencode.cc bencode.h\
	URIResult.cc URIResult.h\
	EventPoll.h\
	SelectEventPoll.cc SelectEventPoll.h\
	SequentialPicker.h\
	SequentialDispatcherCommand.h

if HAVE_EPOLL
SRCS += EpollEventPoll.cc EpollEventPoll.h
endif # HAVE_EPOLL

if ENABLE_SSL
SRCS += TLSContext.h
endif # ENABLE_SSL

if HAVE_LIBGNUTLS
SRCS += LibgnutlsTLSContext.cc LibgnutlsTLSContext.h
endif # HAVE_LIBGNUTLS

if HAVE_LIBSSL
SRCS += LibsslTLSContext.cc LibsslTLSContext.h
endif # HAVE_LIBSSL

if HAVE_LIBZ
SRCS += GZipDecoder.cc GZipDecoder.h
endif # HAVE_LIBZ

if HAVE_SQLITE3
SRCS += Sqlite3MozCookieParser.cc Sqlite3MozCookieParser.h
endif # HAVE_SQLITE3

if ENABLE_ASYNC_DNS
SRCS += AsyncNameResolver.cc AsyncNameResolver.h
endif # ENABLE_ASYNC_DNS

if ENABLE_MESSAGE_DIGEST
SRCS += IteratableChunkChecksumValidator.cc IteratableChunkChecksumValidator.h\
	IteratableChecksumValidator.cc IteratableChecksumValidator.h\
	CheckIntegrityCommand.cc CheckIntegrityCommand.h\
	ChecksumCheckIntegrityEntry.cc ChecksumCheckIntegrityEntry.h\
	messageDigest.cc messageDigest.h\
	MessageDigestHelper.cc MessageDigestHelper.h\
	Checksum.h\
	ChunkChecksum.h
endif # ENABLE_MESSAGE_DIGEST

if ENABLE_BITTORRENT
SRCS += PeerMessageUtil.cc PeerMessageUtil.h\
	PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	Peer.cc Peer.h\
	PeerListenCommand.cc PeerListenCommand.h\
	RequestSlot.cc RequestSlot.h\
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
	BtContext.cc BtContext.h\
	DefaultBtContext.cc DefaultBtContext.h\
	PeerStorage.h\
	DefaultPeerStorage.cc DefaultPeerStorage.h\
	BtAnnounce.cc BtAnnounce.h\
	DefaultBtAnnounce.cc DefaultBtAnnounce.h\
	BtRuntime.h\
	BtMessage.h\
	AbstractBtMessage.cc AbstractBtMessage.h\
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
	AbstractBtEventListener.h\
	BtCancelSendingPieceEvent.h\
	BtAbortOutstandingRequestEvent.h\
	BtChokedEvent.h\
	BtChokingEvent.h\
	BtInteractive.h\
	DefaultBtInteractive.cc DefaultBtInteractive.h\
	ActivePeerConnectionCommand.cc ActivePeerConnectionCommand.h\
	BtDependency.cc BtDependency.h\
	PeerReceiveHandshakeCommand.cc PeerReceiveHandshakeCommand.h\
	BtSetup.cc BtSetup.h\
	BtFileAllocationEntry.cc BtFileAllocationEntry.h\
	BtPostDownloadHandler.cc BtPostDownloadHandler.h\
	BtCheckIntegrityEntry.cc BtCheckIntegrityEntry.h\
	BtExtendedMessage.cc BtExtendedMessage.h\
	ExtensionMessage.h\
	ExtensionMessageFactory.h\
	DefaultExtensionMessageFactory.cc DefaultExtensionMessageFactory.h\
	HandshakeExtensionMessage.cc HandshakeExtensionMessage.h\
	UTPexExtensionMessage.cc UTPexExtensionMessage.h\
	ExtendedMessagingAware.h\
	DHTNode.cc DHTNode.h\
	DHTUtil.cc DHTUtil.h\
	DHTBucket.cc DHTBucket.h\
	DHTRoutingTable.cc DHTRoutingTable.h\
	DHTMessageEntry.cc DHTMessageEntry.h\
	DHTMessageDispatcher.h\
	DHTMessageDispatcherImpl.cc DHTMessageDispatcherImpl.h\
	DHTMessageReceiver.cc DHTMessageReceiver.h\
	DHTMessageTracker.cc DHTMessageTracker.h\
	DHTMessageTrackerEntry.cc DHTMessageTrackerEntry.h\
	DHTMessage.cc DHTMessage.h\
	DHTConnection.h\
	DHTConnectionImpl.cc DHTConnectionImpl.h\
	DHTAbstractMessage.cc DHTAbstractMessage.h\
	DHTQueryMessage.cc DHTQueryMessage.h\
	DHTResponseMessage.cc DHTResponseMessage.h\
	DHTPingMessage.cc DHTPingMessage.h\
	DHTPingReplyMessage.cc DHTPingReplyMessage.h\
	DHTFindNodeMessage.cc DHTFindNodeMessage.h\
	DHTFindNodeReplyMessage.cc DHTFindNodeReplyMessage.h\
	DHTGetPeersMessage.cc DHTGetPeersMessage.h\
	DHTGetPeersReplyMessage.cc DHTGetPeersReplyMessage.h\
	DHTAnnouncePeerMessage.cc DHTAnnouncePeerMessage.h\
	DHTAnnouncePeerReplyMessage.cc DHTAnnouncePeerReplyMessage.h\
	DHTUnknownMessage.cc DHTUnknownMessage.h\
	DHTMessageFactory.h\
	DHTMessageFactoryImpl.cc DHTMessageFactoryImpl.h\
	DHTNodeLookupTask.cc DHTNodeLookupTask.h\
	DHTNodeLookupEntry.cc DHTNodeLookupEntry.h\
	BNode.cc BNode.h\
	DHTMessageCallback.h\
	DHTMessageCallbackImpl.cc DHTMessageCallbackImpl.h\
	DHTMessageCallbackListener.h\
	DHTAbstractTask.cc DHTAbstractTask.h\
	DHTTask.h\
	DHTPingTask.cc DHTPingTask.h\
	DHTTaskQueue.h\
	DHTTaskQueueImpl.cc DHTTaskQueueImpl.h\
	DHTBucketRefreshTask.cc DHTBucketRefreshTask.h\
	DHTAbstractNodeLookupTask.cc DHTAbstractNodeLookupTask.h\
	DHTPeerLookupTask.cc DHTPeerLookupTask.h\
	DHTSetup.cc DHTSetup.h\
	DHTTaskFactory.h\
	DHTTaskFactoryImpl.cc DHTTaskFactoryImpl.h\
	DHTInteractionCommand.cc DHTInteractionCommand.h\
	DHTPeerAnnounceEntry.cc DHTPeerAnnounceEntry.h\
	DHTPeerAnnounceStorage.cc DHTPeerAnnounceStorage.h\
	DHTTokenTracker.cc DHTTokenTracker.h\
	DHTGetPeersCommand.cc DHTGetPeersCommand.h\
	DHTTokenUpdateCommand.cc DHTTokenUpdateCommand.h\
	DHTBucketRefreshCommand.cc DHTBucketRefreshCommand.h\
	DHTPeerAnnounceCommand.cc DHTPeerAnnounceCommand.h\
	DHTReplaceNodeTask.cc DHTReplaceNodeTask.h\
	DHTEntryPointNameResolveCommand.cc DHTEntryPointNameResolveCommand.h\
	DHTRoutingTableSerializer.cc DHTRoutingTableSerializer.h\
	DHTRoutingTableDeserializer.cc DHTRoutingTableDeserializer.h\
	DHTAutoSaveCommand.cc DHTAutoSaveCommand.h\
	DHTRegistry.cc DHTRegistry.h\
	DHTIDCloser.h\
	DHTConstants.h\
	XORCloser.h\
	PeerAddrEntry.h\
	InitiatorMSEHandshakeCommand.cc InitiatorMSEHandshakeCommand.h\
	ReceiverMSEHandshakeCommand.cc ReceiverMSEHandshakeCommand.h\
	MSEHandshake.cc MSEHandshake.h\
	ARC4Decryptor.h\
	ARC4Encryptor.h\
	DHKeyExchange.h\
	LibgcryptARC4Context.h\
	LibgcryptARC4Decryptor.h\
	LibgcryptARC4Encryptor.h\
	LibgcryptDHKeyExchange.h\
	LibsslARC4Context.h\
	LibsslARC4Decryptor.h\
	LibsslARC4Encryptor.h\
	LibsslDHKeyExchange.h\
	BtConstants.h\
	BtLeecherStateChoke.cc BtLeecherStateChoke.h\
	BtSeederStateChoke.cc BtSeederStateChoke.h
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
SRCS += Metalinker.cc Metalinker.h\
	MetalinkEntry.cc MetalinkEntry.h\
	MetalinkResource.cc MetalinkResource.h\
	MetalinkProcessor.h\
	MetalinkProcessorFactory.cc MetalinkProcessorFactory.h\
	MetalinkParserController.cc MetalinkParserController.h\
	MetalinkParserStateMachine.cc MetalinkParserStateMachine.h\
	MetalinkParserState.h\
	InitialMetalinkParserState.cc InitialMetalinkParserState.h\
	MetalinkMetalinkParserState.cc MetalinkMetalinkParserState.h\
	FilesMetalinkParserState.cc FilesMetalinkParserState.h\
	FileMetalinkParserState.cc FileMetalinkParserState.h\
	SizeMetalinkParserState.cc SizeMetalinkParserState.h\
	VersionMetalinkParserState.cc VersionMetalinkParserState.h\
	LanguageMetalinkParserState.cc LanguageMetalinkParserState.h\
	OSMetalinkParserState.cc OSMetalinkParserState.h\
	VerificationMetalinkParserState.cc VerificationMetalinkParserState.h\
	HashMetalinkParserState.cc HashMetalinkParserState.h\
	PiecesMetalinkParserState.cc PiecesMetalinkParserState.h\
	PieceHashMetalinkParserState.cc PieceHashMetalinkParserState.h\
	SignatureMetalinkParserState.cc SignatureMetalinkParserState.h\
	ResourcesMetalinkParserState.cc ResourcesMetalinkParserState.h\
	URLMetalinkParserState.cc URLMetalinkParserState.h\
	FinMetalinkParserState.cc FinMetalinkParserState.h\
	SkipTagMetalinkParserState.cc SkipTagMetalinkParserState.h\
	Metalink2RequestGroup.cc Metalink2RequestGroup.h\
	MetalinkPostDownloadHandler.cc MetalinkPostDownloadHandler.h\
	MetalinkHelper.cc MetalinkHelper.h
endif # ENABLE_METALINK

if ENABLE_LIBXML2
SRCS += XML2SAXMetalinkProcessor.cc XML2SAXMetalinkProcessor.h
endif # ENABLE_LIBXML2

if ENABLE_LIBEXPAT
SRCS += ExpatMetalinkProcessor.cc ExpatMetalinkProcessor.h
endif # ENABLE_LIBEXPAT

if !HAVE_ASCTIME_R
SRCS += asctime_r.c asctime_r.h
endif # !HAVE_ASCTIME_R

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
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@\
	@LIBCARES_LIBS@ @WINSOCK_LIBS@ @LIBEXPAT_LIBS@ @LIBZ_LIBS@\
	@SQLITE3_LIBS@ #-lprofiler
#aria2c_LDFLAGS =  -pg
AM_CPPFLAGS =  -Wall\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBCARES_CPPFLAGS@ @LIBEXPAT_CPPFLAGS@\
	@LIBZ_CPPFLAGS@	 @SQLITE3_CPPFLAGS@\
	-DLOCALEDIR=\"$(localedir)\" -DCA_BUNDLE=\"$(ca_bundle)\" @DEFS@ #-pg
