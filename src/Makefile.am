SUBDIRS = includes
bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc
SRCS =  \
	a2algo.h\
	a2functional.h\
	a2io.h\
	a2iterator.h\
	a2netcompat.h\
	A2STR.cc A2STR.h\
	a2time.h\
	AbstractAuthResolver.cc AbstractAuthResolver.h\
	AbstractCommand.cc AbstractCommand.h\
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	AbstractHttpServerResponseCommand.cc \
	AbstractHttpServerResponseCommand.h \
	AbstractOptionHandler.cc AbstractOptionHandler.h\
	AbstractProxyRequestCommand.cc AbstractProxyRequestCommand.h\
	AbstractProxyResponseCommand.cc AbstractProxyResponseCommand.h\
	AbstractSingleDiskAdaptor.cc AbstractSingleDiskAdaptor.h\
	AdaptiveFileAllocationIterator.cc AdaptiveFileAllocationIterator.h\
	AdaptiveURISelector.cc AdaptiveURISelector.h\
	AnonDiskWriterFactory.h\
	array_fun.h\
	AuthConfig.cc AuthConfig.h\
	AuthConfigFactory.cc AuthConfigFactory.h\
	AuthResolver.h\
	AutoSaveCommand.cc AutoSaveCommand.h\
	BackupIPv4ConnectCommand.h BackupIPv4ConnectCommand.cc\
	base32.cc base32.h\
	base64.h\
	BinaryStream.h\
	bitfield.cc bitfield.h\
	BitfieldMan.cc BitfieldMan.h\
	BtProgressInfoFile.h\
	BufferedFile.cc BufferedFile.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	ByteArrayDiskWriterFactory.h\
	CheckIntegrityCommand.cc CheckIntegrityCommand.h\
	CheckIntegrityDispatcherCommand.cc CheckIntegrityDispatcherCommand.h\
	CheckIntegrityEntry.cc CheckIntegrityEntry.h\
	CheckIntegrityMan.h\
	Checksum.cc Checksum.h\
	ChecksumCheckIntegrityEntry.cc ChecksumCheckIntegrityEntry.h\
	ChunkChecksum.cc ChunkChecksum.h\
	ChunkedDecodingStreamFilter.cc ChunkedDecodingStreamFilter.h\
	ColorizedStream.cc ColorizedStream.h\
	Command.cc Command.h\
	common.h\
	ConnectCommand.cc ConnectCommand.h\
	console.cc console.h\
	ConsoleStatCalc.cc ConsoleStatCalc.h\
	ContentTypeRequestGroupCriteria.cc ContentTypeRequestGroupCriteria.h\
	Context.cc Context.h\
	ContextAttribute.cc ContextAttribute.h\
	ControlChain.h\
	Cookie.cc Cookie.h\
	CookieStorage.cc CookieStorage.h\
	cookie_helper.cc cookie_helper.h\
	CreateRequestCommand.cc CreateRequestCommand.h\
	crypto_endian.h\
	CUIDCounter.cc CUIDCounter.h\
	DefaultAuthResolver.cc DefaultAuthResolver.h\
	DefaultBtProgressInfoFile.cc DefaultBtProgressInfoFile.h\
	DefaultDiskWriter.cc DefaultDiskWriter.h\
	DefaultDiskWriterFactory.cc DefaultDiskWriterFactory.h\
	DefaultPieceStorage.cc DefaultPieceStorage.h\
	DefaultStreamPieceSelector.cc DefaultStreamPieceSelector.h\
	DelayedCommand.h\
	Dependency.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	DiskAdaptor.cc DiskAdaptor.h\
	DiskWriter.h\
	DiskWriterFactory.h\
	DlAbortEx.cc DlAbortEx.h\
	DlRetryEx.cc DlRetryEx.h\
	DNSCache.cc DNSCache.h\
	DownloadCommand.cc DownloadCommand.h\
	DownloadContext.cc DownloadContext.h\
	DownloadEngine.cc DownloadEngine.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	DownloadFailureException.cc DownloadFailureException.h\
	DownloadHandler.cc DownloadHandler.h\
	DownloadHandlerConstants.cc DownloadHandlerConstants.h\
	DownloadResult.cc DownloadResult.h\
	download_handlers.cc download_handlers.h\
	download_helper.cc download_helper.h\
	error_code.h\
	Event.h\
	EventPoll.h\
	Exception.cc Exception.h\
	FatalException.cc FatalException.h\
	FeatureConfig.cc FeatureConfig.h\
	FeedbackURISelector.cc FeedbackURISelector.h\
	File.cc File.h\
	FileAllocationCommand.cc FileAllocationCommand.h\
	FileAllocationDispatcherCommand.cc FileAllocationDispatcherCommand.h\
	FileAllocationEntry.cc FileAllocationEntry.h\
	FileAllocationIterator.h\
	FileAllocationMan.h\
	FileEntry.cc FileEntry.h\
	FillRequestGroupCommand.cc FillRequestGroupCommand.h\
	fmt.cc fmt.h\
	FtpConnection.cc FtpConnection.h\
	FtpDownloadCommand.cc FtpDownloadCommand.h\
	FtpFinishDownloadCommand.cc FtpFinishDownloadCommand.h\
	FtpInitiateConnectionCommand.cc FtpInitiateConnectionCommand.h\
	FtpNegotiationCommand.cc FtpNegotiationCommand.h\
	FtpNegotiationConnectChain.h\
	FtpTunnelRequestCommand.cc FtpTunnelRequestCommand.h\
	FtpTunnelRequestConnectChain.h\
	FtpTunnelResponseCommand.cc FtpTunnelResponseCommand.h\
	GenericParser.h\
	GeomStreamPieceSelector.cc GeomStreamPieceSelector.h\
	GroupId.cc GroupId.h\
	GrowSegment.cc GrowSegment.h\
	HashFuncEntry.h \
	HaveEraseCommand.cc HaveEraseCommand.h\
	help_tags.cc help_tags.h\
	HttpConnection.cc HttpConnection.h\
	HttpDownloadCommand.cc HttpDownloadCommand.h\
	HttpHeader.cc HttpHeader.h\
	HttpHeaderProcessor.cc HttpHeaderProcessor.h\
	HttpInitiateConnectionCommand.cc HttpInitiateConnectionCommand.h\
	HttpListenCommand.cc HttpListenCommand.h\
	HttpProxyRequestCommand.cc HttpProxyRequestCommand.h\
	HttpProxyRequestConnectChain.h\
	HttpProxyResponseCommand.cc HttpProxyResponseCommand.h\
	HttpRequest.cc HttpRequest.h\
	HttpRequestCommand.cc HttpRequestCommand.h\
	HttpRequestConnectChain.h\
	HttpResponse.cc HttpResponse.h\
	HttpResponseCommand.cc HttpResponseCommand.h\
	HttpServer.cc HttpServer.h\
	HttpServerBodyCommand.cc HttpServerBodyCommand.h\
	HttpServerCommand.cc HttpServerCommand.h\
	HttpServerResponseCommand.cc HttpServerResponseCommand.h\
	HttpSkipResponseCommand.cc HttpSkipResponseCommand.h\
	IndexedList.h\
	InitiateConnectionCommand.cc InitiateConnectionCommand.h\
	InitiateConnectionCommandFactory.cc InitiateConnectionCommandFactory.h\
	InorderStreamPieceSelector.cc InorderStreamPieceSelector.h\
	RandomStreamPieceSelector.cc RandomStreamPieceSelector.h\
	InorderURISelector.cc InorderURISelector.h\
	IOFile.cc IOFile.h\
	IteratableChecksumValidator.cc IteratableChecksumValidator.h\
	IteratableChunkChecksumValidator.cc IteratableChunkChecksumValidator.h\
	IteratableValidator.h\
	json.cc json.h\
	JsonDiskWriter.h\
	JsonParser.cc JsonParser.h\
	Lock.h \
	LogFactory.cc LogFactory.h\
	Logger.cc Logger.h\
	LongestSequencePieceSelector.cc LongestSequencePieceSelector.h\
	MemoryBufferPreDownloadHandler.h\
	MemoryPreDownloadHandler.h\
	message.h\
	MessageDigest.cc MessageDigest.h\
	MessageDigestImpl.h\
	message_digest_helper.cc message_digest_helper.h\
	MetadataInfo.cc MetadataInfo.h\
	MetalinkHttpEntry.cc MetalinkHttpEntry.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	MultiFileAllocationIterator.cc MultiFileAllocationIterator.h\
	MultiUrlRequestInfo.cc MultiUrlRequestInfo.h\
	NameResolver.cc NameResolver.h\
	Netrc.cc Netrc.h\
	NetrcAuthResolver.cc NetrcAuthResolver.h\
	NetStat.cc NetStat.h\
	Notifier.cc Notifier.h\
	NsCookieParser.cc NsCookieParser.h\
	NullHandle.h\
	NullOutputFile.h\
	NullProgressInfoFile.h\
	NullSinkStreamFilter.cc NullSinkStreamFilter.h\
	NullStatCalc.h\
	Option.cc Option.h\
	OptionHandler.cc OptionHandler.h\
	OptionHandlerException.cc OptionHandlerException.h\
	OptionHandlerFactory.cc OptionHandlerFactory.h\
	OptionHandlerImpl.cc OptionHandlerImpl.h\
	OptionParser.cc OptionParser.h\
	option_processing.cc\
	OutputFile.h\
	paramed_string.cc paramed_string.h\
	PeerStat.cc PeerStat.h\
	Piece.cc Piece.h\
	PiecedSegment.cc PiecedSegment.h\
	PieceHashCheckIntegrityEntry.cc PieceHashCheckIntegrityEntry.h\
	PieceSelector.h\
	PieceStatMan.cc PieceStatMan.h\
	PieceStorage.h\
	Platform.cc Platform.h\
	PostDownloadHandler.h\
	PreDownloadHandler.h\
	prefs.cc prefs.h\
	ProgressAwareEntry.h\
	ProtocolDetector.cc ProtocolDetector.h\
	Randomizer.h\
	Range.cc Range.h\
	RarestPieceSelector.cc RarestPieceSelector.h\
	RealtimeCommand.cc RealtimeCommand.h\
	RecoverableException.cc RecoverableException.h\
	Request.cc Request.h\
	RequestGroup.cc RequestGroup.h\
	RequestGroupCriteria.h\
	RequestGroupEntry.cc RequestGroupEntry.h\
	RequestGroupMan.cc RequestGroupMan.h\
	RpcMethod.cc RpcMethod.h\
	RpcMethodFactory.cc RpcMethodFactory.h\
	RpcMethodImpl.cc RpcMethodImpl.h\
	RpcRequest.cc RpcRequest.h\
	RpcResponse.cc RpcResponse.h\
	rpc_helper.cc rpc_helper.h\
	SaveSessionCommand.h SaveSessionCommand.cc\
	SegList.h\
	Segment.h\
	SegmentMan.cc SegmentMan.h\
	SelectEventPoll.cc SelectEventPoll.h\
	SequentialDispatcherCommand.h\
	SequentialPicker.h\
	ServerStat.cc ServerStat.h\
	ServerStatMan.cc ServerStatMan.h\
	SessionSerializer.cc SessionSerializer.h\
	Signature.cc Signature.h\
	SimpleRandomizer.cc SimpleRandomizer.h\
	SingleFileAllocationIterator.cc SingleFileAllocationIterator.h\
	SingletonHolder.h\
	SinkStreamFilter.cc SinkStreamFilter.h\
	SocketBuffer.cc SocketBuffer.h\
	SocketCore.cc SocketCore.h\
	SocketRecvBuffer.cc SocketRecvBuffer.h\
	SpeedCalc.cc SpeedCalc.h\
	StatCalc.h\
	StreamCheckIntegrityEntry.cc StreamCheckIntegrityEntry.h\
	StreamFileAllocationEntry.cc StreamFileAllocationEntry.h\
	StreamFilter.cc StreamFilter.h\
	StreamPieceSelector.h\
	StructParserStateMachine.h\
	TimeA2.cc TimeA2.h\
	TimeBasedCommand.cc TimeBasedCommand.h\
	TimedHaltCommand.cc TimedHaltCommand.h\
	TimerA2.cc TimerA2.h\
	timespec.h\
	TorrentAttribute.cc TorrentAttribute.h\
	TransferStat.cc TransferStat.h\
	TruncFileAllocationIterator.cc TruncFileAllocationIterator.h\
	UnknownLengthPieceStorage.cc UnknownLengthPieceStorage.h\
	UnknownOptionException.cc UnknownOptionException.h\
	uri.cc uri.h\
	UriListParser.cc UriListParser.h\
	URIResult.cc URIResult.h\
	URISelector.h\
	uri_split.c uri_split.h\
	usage_text.h\
	util.cc util.h\
	util_security.cc util_security.h\
	ValueBase.cc ValueBase.h\
	ValueBaseDiskWriter.h\
	ValueBaseJsonParser.h\
	ValueBaseStructParserState.h\
	ValueBaseStructParserStateImpl.cc ValueBaseStructParserStateImpl.h\
	ValueBaseStructParserStateMachine.cc ValueBaseStructParserStateMachine.h\
	version_usage.cc\
	wallclock.cc wallclock.h\
	WatchProcessCommand.cc WatchProcessCommand.h\
	WrDiskCache.cc WrDiskCache.h\
	WrDiskCacheEntry.cc WrDiskCacheEntry.h\
	XmlRpcRequestParserController.cc XmlRpcRequestParserController.h\
	OpenedFileCounter.cc OpenedFileCounter.h \
	SHA1IOFile.cc SHA1IOFile.h \
	EvictSocketPoolCommand.cc EvictSocketPoolCommand.h\
	libssl_compat.h

if MINGW_BUILD
SRCS += WinConsoleFile.cc WinConsoleFile.h
endif # MINGW_BUILD

if ENABLE_WEBSOCKET
SRCS += \
	WebSocketInteractionCommand.cc WebSocketInteractionCommand.h\
	WebSocketResponseCommand.cc WebSocketResponseCommand.h\
	WebSocketSession.cc WebSocketSession.h\
	WebSocketSessionMan.cc WebSocketSessionMan.h
endif # ENABLE_WEBSOCKET

if !ENABLE_WEBSOCKET
SRCS += NullWebSocketSessionMan.h
endif # !ENABLE_WEBSOCKET

if HAVE_SOME_XMLLIB
SRCS += \
	ParserStateMachine.h\
	XmlAttr.cc XmlAttr.h\
	XmlParser.cc XmlParser.h
endif # HAVE_SOME_XMLLIB

if HAVE_LIBXML2
SRCS +=  Xml2XmlParser.cc Xml2XmlParser.h
endif # HAVE_LIBXML2

if HAVE_LIBEXPAT
SRCS +=  ExpatXmlParser.cc ExpatXmlParser.h
endif # HAVE_LIBEXPAT

if ENABLE_XML_RPC
SRCS += \
	XmlRpcDiskWriter.cc XmlRpcDiskWriter.h\
	XmlRpcRequestParserState.h\
	XmlRpcRequestParserStateImpl.cc XmlRpcRequestParserStateImpl.h\
	XmlRpcRequestParserStateMachine.cc XmlRpcRequestParserStateMachine.h

endif # ENABLE_XML_RPC

if HAVE_SOME_FALLOCATE
SRCS += FallocFileAllocationIterator.cc FallocFileAllocationIterator.h
endif # HAVE_SOME_FALLOCATE

if HAVE_EPOLL
SRCS += EpollEventPoll.cc EpollEventPoll.h
endif # HAVE_EPOLL

if ENABLE_SSL
SRCS += TLSContext.h TLSSession.h
endif # ENABLE_SSL

if USE_APPLE_MD
SRCS += AppleMessageDigestImpl.cc
endif # USE_APPLE_MD

if HAVE_APPLETLS
SRCS += \
	AppleTLSContext.cc AppleTLSContext.h \
	AppleTLSSession.cc AppleTLSSession.h
endif # HAVE_APPLETLS

if HAVE_WINTLS
SRCS += \
	WinTLSContext.cc WinTLSContext.h \
	WinTLSSession.cc WinTLSSession.h
endif # HAVE_WINTLS

if USE_INTERNAL_BIGNUM
SRCS += \
	bignum.h\
	InternalDHKeyExchange.cc InternalDHKeyExchange.h
endif

if USE_INTERNAL_ARC4
SRCS += InternalARC4Encryptor.cc InternalARC4Encryptor.h
endif

if USE_INTERNAL_MD
SRCS += \
	InternalMessageDigestImpl.cc\
	crypto_hash.cc crypto_hash.h
endif # USE_WINDOWS_MD

if HAVE_LIBGNUTLS
SRCS += \
	LibgnutlsTLSContext.cc LibgnutlsTLSContext.h \
        LibgnutlsTLSSession.cc LibgnutlsTLSSession.h
endif # HAVE_LIBGNUTLS

if HAVE_LIBGCRYPT
SRCS += \
	LibgcryptARC4Encryptor.cc LibgcryptARC4Encryptor.h \
        LibgcryptDHKeyExchange.cc LibgcryptDHKeyExchange.h
if USE_LIBGCRYPT_MD
SRCS += LibgcryptMessageDigestImpl.cc
endif # USE_LIBGCRYPT_MD
endif # HAVE_LIBGCRYPT

if HAVE_LIBNETTLE
SRCS += LibnettleARC4Encryptor.cc LibnettleARC4Encryptor.h
if USE_LIBNETTLE_MD
SRCS += LibnettleMessageDigestImpl.cc
endif # USE_LIBNETTLE_MD
endif # HAVE_LIBNETTLE

if HAVE_LIBGMP
SRCS += \
	a2gmp.cc a2gmp.h \
        LibgmpDHKeyExchange.cc LibgmpDHKeyExchange.h
endif # HAVE_LIBGMP

if HAVE_OPENSSL
SRCS += \
	LibsslARC4Encryptor.cc LibsslARC4Encryptor.h \
        LibsslDHKeyExchange.cc LibsslDHKeyExchange.h
if !HAVE_APPLETLS
SRCS += \
	LibsslTLSContext.cc LibsslTLSContext.h \
        LibsslTLSSession.cc LibsslTLSSession.h
endif # !HAVE_APPLETLS
if USE_OPENSSL_MD
SRCS += LibsslMessageDigestImpl.cc
endif
endif # HAVE_OPENSSL

if HAVE_ZLIB
SRCS += \
	GZipDecodingStreamFilter.cc GZipDecodingStreamFilter.h\
	GZipEncoder.cc GZipEncoder.h\
	GZipFile.cc GZipFile.h \
	Adler32MessageDigestImpl.cc Adler32MessageDigestImpl.h
endif # HAVE_ZLIB

if HAVE_SQLITE3
SRCS += \
	Sqlite3CookieParser.cc Sqlite3CookieParser.h\
	Sqlite3CookieParserImpl.cc Sqlite3CookieParserImpl.h
endif # HAVE_SQLITE3

if HAVE_LIBSSH2
SRCS += SSHSession.cc SSHSession.h \
	SftpNegotiationCommand.cc SftpNegotiationCommand.h \
	SftpNegotiationConnectChain.h \
	SftpDownloadCommand.cc SftpDownloadCommand.h \
	SftpFinishDownloadCommand.cc SftpFinishDownloadCommand.h
endif # HAVE_LIBSSH2

if ENABLE_ASYNC_DNS
SRCS += \
	AsyncNameResolver.cc AsyncNameResolver.h\
	AsyncNameResolverMan.cc AsyncNameResolverMan.h
endif # ENABLE_ASYNC_DNS

if ENABLE_BITTORRENT
SRCS += \
	AbstractBtMessage.cc AbstractBtMessage.h\
	ActivePeerConnectionCommand.cc ActivePeerConnectionCommand.h\
	AnnounceList.h AnnounceList.cc\
	AnnounceTier.cc AnnounceTier.h\
	ARC4Encryptor.h\
	bencode2.cc bencode2.h\
	BencodeDiskWriter.h\
	BencodeDiskWriterFactory.h\
	BencodeParser.cc BencodeParser.h\
	bittorrent_helper.cc bittorrent_helper.h\
	BtAbortOutstandingRequestEvent.cc BtAbortOutstandingRequestEvent.h\
	BtAllowedFastMessage.cc BtAllowedFastMessage.h\
	BtAnnounce.cc BtAnnounce.h\
	BtBitfieldMessage.cc BtBitfieldMessage.h\
	BtBitfieldMessageValidator.cc BtBitfieldMessageValidator.h\
	BtCancelMessage.cc BtCancelMessage.h\
	BtCancelSendingPieceEvent.h\
	BtCheckIntegrityEntry.cc BtCheckIntegrityEntry.h\
	BtChokeMessage.cc BtChokeMessage.h\
	BtChokingEvent.h\
	BtConstants.h\
	BtDependency.cc BtDependency.h\
	BtExtendedMessage.cc BtExtendedMessage.h\
	BtFileAllocationEntry.cc BtFileAllocationEntry.h\
	BtHandshakeMessage.cc BtHandshakeMessage.h\
	BtHandshakeMessageValidator.cc BtHandshakeMessageValidator.h\
	BtHaveAllMessage.cc BtHaveAllMessage.h\
	BtHaveMessage.cc BtHaveMessage.h\
	BtHaveNoneMessage.cc BtHaveNoneMessage.h\
	BtInteractive.h\
	BtInterestedMessage.cc BtInterestedMessage.h\
	BtKeepAliveMessage.cc BtKeepAliveMessage.h\
	BtLeecherStateChoke.cc BtLeecherStateChoke.h\
	BtMessage.h\
	BtMessageDispatcher.h\
	BtMessageFactory.h\
	BtMessageReceiver.h\
	BtMessageValidator.h\
	BtNotInterestedMessage.cc BtNotInterestedMessage.h\
	BtPieceMessage.cc BtPieceMessage.h\
	BtPieceMessageValidator.cc BtPieceMessageValidator.h\
	BtPortMessage.cc BtPortMessage.h\
	BtPostDownloadHandler.cc BtPostDownloadHandler.h\
	BtRegistry.cc BtRegistry.h\
	BtRejectMessage.cc BtRejectMessage.h\
	BtRequestFactory.h\
	BtRequestMessage.cc BtRequestMessage.h\
	BtRuntime.cc BtRuntime.h\
	BtSeederStateChoke.cc BtSeederStateChoke.h\
	BtSetup.cc BtSetup.h\
	BtStopDownloadCommand.cc BtStopDownloadCommand.h\
	BtSuggestPieceMessage.cc BtSuggestPieceMessage.h\
	BtUnchokeMessage.cc BtUnchokeMessage.h\
	DefaultBtAnnounce.cc DefaultBtAnnounce.h\
	DefaultBtInteractive.cc DefaultBtInteractive.h\
	DefaultBtMessageDispatcher.cc DefaultBtMessageDispatcher.h\
	DefaultBtMessageFactory.cc DefaultBtMessageFactory.h\
	DefaultBtMessageReceiver.cc DefaultBtMessageReceiver.h\
	DefaultBtRequestFactory.cc DefaultBtRequestFactory.h\
	DefaultExtensionMessageFactory.cc DefaultExtensionMessageFactory.h\
	DefaultPeerStorage.cc DefaultPeerStorage.h\
	DHKeyExchange.h\
	DHTAbstractMessage.cc DHTAbstractMessage.h\
	DHTAbstractNodeLookupTask.h\
	DHTAbstractTask.cc DHTAbstractTask.h\
	DHTAnnouncePeerMessage.cc DHTAnnouncePeerMessage.h\
	DHTAnnouncePeerReplyMessage.cc DHTAnnouncePeerReplyMessage.h\
	DHTAutoSaveCommand.cc DHTAutoSaveCommand.h\
	DHTBucket.cc DHTBucket.h\
	DHTBucketRefreshCommand.cc DHTBucketRefreshCommand.h\
	DHTBucketRefreshTask.cc DHTBucketRefreshTask.h\
	DHTBucketTree.cc DHTBucketTree.h\
	DHTConnection.h\
	DHTConnectionImpl.cc DHTConnectionImpl.h\
	DHTConstants.h\
	DHTEntryPointNameResolveCommand.cc DHTEntryPointNameResolveCommand.h\
	DHTFindNodeMessage.cc DHTFindNodeMessage.h\
	DHTFindNodeReplyMessage.cc DHTFindNodeReplyMessage.h\
	DHTGetPeersCommand.cc DHTGetPeersCommand.h\
	DHTGetPeersMessage.cc DHTGetPeersMessage.h\
	DHTGetPeersReplyMessage.cc DHTGetPeersReplyMessage.h\
	DHTIDCloser.h\
	DHTInteractionCommand.cc DHTInteractionCommand.h\
	DHTMessage.cc DHTMessage.h\
	DHTMessageCallback.h\
	DHTMessageDispatcher.h\
	DHTMessageDispatcherImpl.cc DHTMessageDispatcherImpl.h\
	DHTMessageEntry.cc DHTMessageEntry.h\
	DHTMessageFactory.h\
	DHTMessageFactoryImpl.cc DHTMessageFactoryImpl.h\
	DHTMessageReceiver.cc DHTMessageReceiver.h\
	DHTMessageTracker.cc DHTMessageTracker.h\
	DHTMessageTrackerEntry.cc DHTMessageTrackerEntry.h\
	DHTNode.cc DHTNode.h\
	DHTNodeLookupEntry.cc DHTNodeLookupEntry.h\
	DHTNodeLookupTask.cc DHTNodeLookupTask.h\
	DHTNodeLookupTaskCallback.cc DHTNodeLookupTaskCallback.h\
	DHTPeerAnnounceCommand.cc DHTPeerAnnounceCommand.h\
	DHTPeerAnnounceEntry.cc DHTPeerAnnounceEntry.h\
	DHTPeerAnnounceStorage.cc DHTPeerAnnounceStorage.h\
	DHTPeerLookupTask.cc DHTPeerLookupTask.h\
	DHTPeerLookupTaskCallback.cc DHTPeerLookupTaskCallback.h\
	DHTPingMessage.cc DHTPingMessage.h\
	DHTPingReplyMessage.cc DHTPingReplyMessage.h\
	DHTPingReplyMessageCallback.h\
	DHTPingTask.cc DHTPingTask.h\
	DHTQueryMessage.cc DHTQueryMessage.h\
	DHTRegistry.cc DHTRegistry.h\
	DHTReplaceNodeTask.cc DHTReplaceNodeTask.h\
	DHTResponseMessage.cc DHTResponseMessage.h\
	DHTRoutingTable.cc DHTRoutingTable.h\
	DHTRoutingTableDeserializer.cc DHTRoutingTableDeserializer.h\
	DHTRoutingTableSerializer.cc DHTRoutingTableSerializer.h\
	DHTSetup.cc DHTSetup.h\
	DHTTask.h\
	DHTTaskExecutor.cc DHTTaskExecutor.h\
	DHTTaskFactory.h\
	DHTTaskFactoryImpl.cc DHTTaskFactoryImpl.h\
	DHTTaskQueue.h\
	DHTTaskQueueImpl.cc DHTTaskQueueImpl.h\
	DHTTokenTracker.cc DHTTokenTracker.h\
	DHTTokenUpdateCommand.cc DHTTokenUpdateCommand.h\
	DHTUnknownMessage.cc DHTUnknownMessage.h\
	ExtensionMessage.h\
	ExtensionMessageFactory.h\
	ExtensionMessageRegistry.cc ExtensionMessageRegistry.h\
	HandshakeExtensionMessage.cc HandshakeExtensionMessage.h\
	IndexBtMessage.cc IndexBtMessage.h\
	IndexBtMessageValidator.cc IndexBtMessageValidator.h\
	InitiatorMSEHandshakeCommand.cc InitiatorMSEHandshakeCommand.h\
	LpdDispatchMessageCommand.cc LpdDispatchMessageCommand.h\
	LpdMessage.cc LpdMessage.h\
	LpdMessageDispatcher.cc LpdMessageDispatcher.h\
	LpdMessageReceiver.cc LpdMessageReceiver.h\
	LpdReceiveMessageCommand.cc LpdReceiveMessageCommand.h\
	magnet.cc magnet.h\
	MemoryBencodePreDownloadHandler.h\
	MSEHandshake.cc MSEHandshake.h\
	NameResolveCommand.cc NameResolveCommand.h\
	Peer.cc Peer.h\
	PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerAddrEntry.cc PeerAddrEntry.h\
	PeerChokeCommand.cc PeerChokeCommand.h\
	PeerConnection.cc PeerConnection.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	PeerListenCommand.cc PeerListenCommand.h\
	PeerReceiveHandshakeCommand.cc PeerReceiveHandshakeCommand.h\
	PeerSessionResource.cc PeerSessionResource.h\
	PeerStorage.h\
	PriorityPieceSelector.cc PriorityPieceSelector.h\
	RangeBtMessage.cc RangeBtMessage.h\
	RangeBtMessageValidator.cc RangeBtMessageValidator.h\
	ReceiverMSEHandshakeCommand.cc ReceiverMSEHandshakeCommand.h\
	RequestSlot.cc RequestSlot.h\
	SeedCheckCommand.cc SeedCheckCommand.h\
	SeedCriteria.h\
	ShareRatioSeedCriteria.cc ShareRatioSeedCriteria.h\
	SimpleBtMessage.cc SimpleBtMessage.h\
	TimeSeedCriteria.cc TimeSeedCriteria.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	UDPTrackerClient.cc UDPTrackerClient.h\
	UDPTrackerRequest.cc UDPTrackerRequest.h\
	UnionSeedCriteria.cc UnionSeedCriteria.h\
	UTMetadataDataExtensionMessage.cc UTMetadataDataExtensionMessage.h\
	UTMetadataExtensionMessage.cc UTMetadataExtensionMessage.h\
	UTMetadataPostDownloadHandler.cc UTMetadataPostDownloadHandler.h\
	UTMetadataRejectExtensionMessage.cc UTMetadataRejectExtensionMessage.h\
	UTMetadataRequestExtensionMessage.cc\
	UTMetadataRequestExtensionMessage.h\
	UTMetadataRequestFactory.cc UTMetadataRequestFactory.h\
	UTMetadataRequestTracker.cc UTMetadataRequestTracker.h\
	UTPexExtensionMessage.cc UTPexExtensionMessage.h\
	ValueBaseBencodeParser.h\
	XORCloser.h\
	ZeroBtMessage.cc ZeroBtMessage.h
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
SRCS += \
	Metalink2RequestGroup.cc Metalink2RequestGroup.h\
	MetalinkEntry.cc MetalinkEntry.h\
	Metalinker.cc Metalinker.h\
	MetalinkMetaurl.cc MetalinkMetaurl.h\
	MetalinkParserController.cc MetalinkParserController.h\
	MetalinkParserState.cc MetalinkParserState.h\
	MetalinkParserStateImpl.cc MetalinkParserStateImpl.h\
	MetalinkParserStateMachine.cc MetalinkParserStateMachine.h\
	MetalinkParserStateV3Impl.cc MetalinkParserStateV3Impl.h\
	MetalinkParserStateV4Impl.cc MetalinkParserStateV4Impl.h\
	MetalinkPostDownloadHandler.cc MetalinkPostDownloadHandler.h\
	MetalinkResource.cc MetalinkResource.h\
	metalink_helper.cc metalink_helper.h

endif # ENABLE_METALINK

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

if !HAVE_LOCALTIME_R
SRCS += localtime_r.c localtime_r.h
endif # !HAVE_LOCALTIME_R

if !HAVE_STRPTIME
SRCS += strptime.c strptime.h
endif # !HAVE_STRPTIME

if !HAVE_TIMEGM
SRCS += timegm.c timegm.h
endif # !HAVE_TIMEGM

if !HAVE_DAEMON
SRCS += daemon.cc daemon.h
endif # !HAVE_DAEMON

if HAVE_POLL
SRCS += PollEventPoll.cc PollEventPoll.h
endif # HAVE_POLL

if HAVE_PORT_ASSOCIATE
SRCS += PortEventPoll.cc PortEventPoll.h
endif # HAVE_PORT_ASSOCIATE

if HAVE_KQUEUE
SRCS += KqueueEventPoll.cc KqueueEventPoll.h
endif # HAVE_KQUEUE

if HAVE_LIBUV
SRCS += LibuvEventPoll.cc LibuvEventPoll.h
endif # HAVE_LIBUV

AR = @AR@

if ENABLE_LIBARIA2
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libaria2.pc
DISTCLEANFILES = $(pkgconfig_DATA)

lib_LTLIBRARIES = libaria2.la
SRCS += \
	ApiCallbackDownloadEventListener.cc ApiCallbackDownloadEventListener.h\
	aria2api.cc aria2api.h \
	KeepRunningCommand.cc KeepRunningCommand.h
else # !ENABLE_LIBARIA2
noinst_LTLIBRARIES = libaria2.la
endif # !ENABLE_LIBARIA2

libaria2_la_SOURCES = $(SRCS)


AM_CPPFLAGS = \
	-I$(top_srcdir)/lib -I$(top_srcdir)/intl\
	-I$(srcdir)/includes -I$(builddir)/includes\
	-DLOCALEDIR=\"@localedir@\" @DEFS@ \
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

EXTLDADD = @ALLOCA@ \
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
	@TCMALLOC_LIBS@ \
	@JEMALLOC_LIBS@ \
	@LTLIBINTL@

LDADD = libaria2.la

if ENABLE_LIBARIA2
libaria2_la_LIBADD = $(EXTLDADD)
else # !ENABLE_LIBARIA2
LDADD += $(EXTLDADD)
endif # !ENABLE_LIBARIA2

AM_CFLAGS = @EXTRACFLAGS@

AM_CXXFLAGS = @WARNCXXFLAGS@ @CXX1XCXXFLAGS@ @EXTRACXXFLAGS@

