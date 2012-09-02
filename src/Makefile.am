bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc\
	option_processing.cc\
	version_usage.cc
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
	util.cc util.h\
	Request.cc Request.h\
	common.h\
	message.h\
	Exception.cc Exception.h\
	FatalException.cc FatalException.h\
	RecoverableException.cc RecoverableException.h\
	DlAbortEx.cc DlAbortEx.h\
	DlRetryEx.cc DlRetryEx.h\
	DownloadFailureException.cc DownloadFailureException.h\
	Logger.cc Logger.h\
	fmt.cc fmt.h\
	DiskWriter.h\
	DiskWriterFactory.h\
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	DefaultDiskWriter.cc DefaultDiskWriter.h\
	DefaultDiskWriterFactory.cc DefaultDiskWriterFactory.h\
	File.cc File.h\
	Option.cc Option.h\
	base64.h\
	base32.cc base32.h\
	LogFactory.cc LogFactory.h\
	TimerA2.cc TimerA2.h\
	TimeA2.cc TimeA2.h\
	SharedHandle.h\
	FeatureConfig.cc FeatureConfig.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	SpeedCalc.cc SpeedCalc.h\
	PeerStat.cc PeerStat.h\
	BitfieldMan.cc BitfieldMan.h\
	Randomizer.h\
	SimpleRandomizer.cc SimpleRandomizer.h\
	HttpResponse.cc HttpResponse.h\
	HttpRequest.cc HttpRequest.h\
	Range.cc Range.h\
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
	OptionHandlerImpl.cc OptionHandlerImpl.h\
	AbstractOptionHandler.cc AbstractOptionHandler.h\
	NameResolver.cc NameResolver.h\
	RequestGroup.cc RequestGroup.h\
	RequestGroupMan.cc RequestGroupMan.h\
	FileAllocationMan.h\
	FileAllocationCommand.cc FileAllocationCommand.h\
	FillRequestGroupCommand.cc FillRequestGroupCommand.h\
	FileAllocationDispatcherCommand.cc FileAllocationDispatcherCommand.h\
	FileAllocationEntry.cc FileAllocationEntry.h\
	StreamFileAllocationEntry.cc StreamFileAllocationEntry.h\
	MultiUrlRequestInfo.cc MultiUrlRequestInfo.h\
	UriListParser.cc UriListParser.h\
	RealtimeCommand.cc RealtimeCommand.h\
	ProgressAwareEntry.h\
	RequestGroupEntry.cc RequestGroupEntry.h\
	Cookie.cc Cookie.h\
	HttpHeaderProcessor.cc HttpHeaderProcessor.h\
	FileEntry.cc FileEntry.h\
	Platform.cc Platform.h\
	TimeBasedCommand.cc TimeBasedCommand.h\
	AutoSaveCommand.cc AutoSaveCommand.h\
	PieceStorage.h\
	DefaultPieceStorage.cc DefaultPieceStorage.h\
	UnknownLengthPieceStorage.cc UnknownLengthPieceStorage.h\
	PieceStatMan.cc PieceStatMan.h\
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
	MemoryPreDownloadHandler.h\
	MemoryBufferPreDownloadHandler.h\
	HaveEraseCommand.cc HaveEraseCommand.h\
	Piece.cc Piece.h\
	CheckIntegrityMan.h\
	CheckIntegrityEntry.cc CheckIntegrityEntry.h\
	PieceHashCheckIntegrityEntry.cc PieceHashCheckIntegrityEntry.h\
	StreamCheckIntegrityEntry.cc StreamCheckIntegrityEntry.h\
	IteratableValidator.h\
	DiskAdaptor.cc DiskAdaptor.h\
	AbstractSingleDiskAdaptor.cc AbstractSingleDiskAdaptor.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	PeerSessionResource.cc PeerSessionResource.h\
	BtRegistry.cc BtRegistry.h\
	MultiFileAllocationIterator.cc MultiFileAllocationIterator.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	ByteArrayDiskWriterFactory.h\
	DownloadContext.cc DownloadContext.h\
	TimedHaltCommand.cc TimedHaltCommand.h\
	CUIDCounter.cc CUIDCounter.h\
	DNSCache.cc DNSCache.h\
	DownloadResult.cc DownloadResult.h\
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
	HttpSkipResponseCommand.cc HttpSkipResponseCommand.h\
	InitiateConnectionCommand.cc InitiateConnectionCommand.h\
	FtpFinishDownloadCommand.cc FtpFinishDownloadCommand.h\
	A2STR.cc A2STR.h\
	RarestPieceSelector.cc RarestPieceSelector.h\
	Decoder.h\
	Signature.cc Signature.h\
	ServerStat.cc ServerStat.h\
	ServerStatMan.cc ServerStatMan.h\
	URISelector.h\
	AdaptiveURISelector.cc AdaptiveURISelector.h\
	InorderURISelector.cc InorderURISelector.h\
	FeedbackURISelector.cc FeedbackURISelector.h\
	NsCookieParser.cc NsCookieParser.h\
	CookieStorage.cc CookieStorage.h\
	SocketBuffer.cc SocketBuffer.h\
	SocketRecvBuffer.cc SocketRecvBuffer.h\
	OptionHandlerException.cc OptionHandlerException.h\
	URIResult.cc URIResult.h\
	EventPoll.h\
	SelectEventPoll.cc SelectEventPoll.h\
	SequentialPicker.h\
	SequentialDispatcherCommand.h\
	PieceSelector.h\
	LongestSequencePieceSelector.cc LongestSequencePieceSelector.h\
	bitfield.cc bitfield.h\
	CreateRequestCommand.cc CreateRequestCommand.h\
	error_code.h\
	wallclock.cc wallclock.h\
	download_helper.cc download_helper.h\
	MetadataInfo.cc MetadataInfo.h\
	SessionSerializer.cc SessionSerializer.h\
	Event.h\
	timespec.h\
	ValueBase.cc ValueBase.h\
	ContextAttribute.h\
	TorrentAttribute.cc TorrentAttribute.h\
	AdaptiveFileAllocationIterator.cc AdaptiveFileAllocationIterator.h\
	TruncFileAllocationIterator.cc TruncFileAllocationIterator.h\
	StreamFilter.cc StreamFilter.h\
	SinkStreamFilter.cc SinkStreamFilter.h\
	ChunkedDecodingStreamFilter.cc ChunkedDecodingStreamFilter.h\
	NullSinkStreamFilter.cc NullSinkStreamFilter.h\
	uri.cc uri.h\
	Triplet.h\
	cookie_helper.cc cookie_helper.h\
	json.cc json.h\
	JsonParser.cc JsonParser.h\
	StructParserStateMachine.h\
	GenericParser.h\
	ValueBaseJsonParser.h\
	ValueBaseStructParserState.h\
	ValueBaseStructParserStateImpl.cc ValueBaseStructParserStateImpl.h\
	ValueBaseStructParserStateMachine.cc ValueBaseStructParserStateMachine.h\
	JsonDiskWriter.h\
	HttpServerBodyCommand.cc HttpServerBodyCommand.h\
	RpcRequest.cc RpcRequest.h\
	RpcMethod.cc RpcMethod.h\
	RpcMethodImpl.cc RpcMethodImpl.h\
	RpcMethodFactory.cc RpcMethodFactory.h\
	RpcResponse.cc RpcResponse.h\
	HttpListenCommand.cc HttpListenCommand.h\
	HttpServerCommand.cc HttpServerCommand.h\
	HttpServerResponseCommand.cc HttpServerResponseCommand.h\
	AbstractHttpServerResponseCommand.cc \
	AbstractHttpServerResponseCommand.h \
	HttpServer.cc HttpServer.h\
	StreamPieceSelector.h\
	DefaultStreamPieceSelector.cc DefaultStreamPieceSelector.h\
	InorderStreamPieceSelector.cc InorderStreamPieceSelector.h\
	GeomStreamPieceSelector.cc GeomStreamPieceSelector.h\
	MetalinkHttpEntry.cc MetalinkHttpEntry.h\
	OutputFile.h\
	NullOutputFile.h\
	console.cc console.h\
	BufferedFile.cc BufferedFile.h\
	SegList.h\
	NullHandle.h\
	a2iterator.h\
	paramed_string.cc paramed_string.h\
	rpc_helper.cc rpc_helper.h\
	WatchProcessCommand.cc WatchProcessCommand.h\
	UnknownOptionException.cc UnknownOptionException.h\
	Notifier.cc Notifier.h\
	ValueBaseDiskWriter.h\
	AnonDiskWriterFactory.h\
	XmlRpcRequestParserController.cc XmlRpcRequestParserController.h

if MINGW_BUILD
SRCS += WinConsoleFile.cc WinConsoleFile.h
endif # MINGW_BUILD

if ENABLE_WEBSOCKET
SRCS += WebSocketSession.cc WebSocketSession.h\
	WebSocketSessionMan.cc WebSocketSessionMan.h\
	WebSocketResponseCommand.cc WebSocketResponseCommand.h\
	WebSocketInteractionCommand.cc WebSocketInteractionCommand.h
endif # ENABLE_WEBSOCKET

if !ENABLE_WEBSOCKET
SRCS += NullWebSocketSessionMan.h
endif # !ENABLE_WEBSOCKET

if HAVE_SOME_XMLLIB
SRCS += XmlAttr.cc XmlAttr.h\
	XmlParser.cc XmlParser.h\
	ParserStateMachine.h
endif # HAVE_SOME_XMLLIB

if HAVE_LIBXML2
SRCS +=  Xml2XmlParser.cc Xml2XmlParser.h
endif # HAVE_LIBXML2

if HAVE_LIBEXPAT
SRCS +=  ExpatXmlParser.cc ExpatXmlParser.h
endif # HAVE_LIBEXPAT

if ENABLE_XML_RPC
SRCS += XmlRpcRequestParserStateMachine.cc XmlRpcRequestParserStateMachine.h\
	XmlRpcRequestParserState.h\
	XmlRpcRequestParserStateImpl.cc XmlRpcRequestParserStateImpl.h\
	XmlRpcDiskWriter.cc XmlRpcDiskWriter.h

endif # ENABLE_XML_RPC

if HAVE_SOME_FALLOCATE
SRCS += FallocFileAllocationIterator.cc FallocFileAllocationIterator.h
endif # HAVE_SOME_FALLOCATE

if HAVE_EPOLL
SRCS += EpollEventPoll.cc EpollEventPoll.h
endif # HAVE_EPOLL

if ENABLE_SSL
SRCS += TLSContext.h
endif # ENABLE_SSL

if HAVE_LIBGNUTLS
SRCS += LibgnutlsTLSContext.cc LibgnutlsTLSContext.h
endif # HAVE_LIBGNUTLS

if HAVE_LIBGCRYPT
SRCS += LibgcryptMessageDigestImpl.cc LibgcryptMessageDigestImpl.h\
	LibgcryptARC4Encryptor.cc LibgcryptARC4Encryptor.h\
	LibgcryptDHKeyExchange.cc LibgcryptDHKeyExchange.h
endif # HAVE_LIBGCRYPT

if HAVE_LIBNETTLE
SRCS += LibnettleMessageDigestImpl.cc LibnettleMessageDigestImpl.h\
	LibnettleARC4Encryptor.cc LibnettleARC4Encryptor.h
endif # HAVE_LIBNETTLE

if HAVE_LIBGMP
SRCS += a2gmp.cc a2gmp.h\
	LibgmpDHKeyExchange.cc LibgmpDHKeyExchange.h
endif # HAVE_LIBGMP

if HAVE_OPENSSL
SRCS += LibsslTLSContext.cc LibsslTLSContext.h\
	LibsslMessageDigestImpl.cc LibsslMessageDigestImpl.h\
	LibsslARC4Encryptor.cc LibsslARC4Encryptor.h\
	LibsslDHKeyExchange.cc LibsslDHKeyExchange.h
endif # HAVE_OPENSSL

if HAVE_ZLIB
SRCS += GZipDecoder.cc GZipDecoder.h\
	GZipEncoder.cc GZipEncoder.h\
	GZipDecodingStreamFilter.cc GZipDecodingStreamFilter.h
endif # HAVE_ZLIB

if HAVE_SQLITE3
SRCS += Sqlite3CookieParser.cc Sqlite3CookieParser.h\
	Sqlite3CookieParserImpl.cc Sqlite3CookieParserImpl.h
endif # HAVE_SQLITE3

if ENABLE_ASYNC_DNS
SRCS += AsyncNameResolver.cc AsyncNameResolver.h
endif # ENABLE_ASYNC_DNS

if ENABLE_MESSAGE_DIGEST
SRCS += IteratableChunkChecksumValidator.cc IteratableChunkChecksumValidator.h\
	IteratableChecksumValidator.cc IteratableChecksumValidator.h\
	CheckIntegrityDispatcherCommand.cc CheckIntegrityDispatcherCommand.h\
	CheckIntegrityCommand.cc CheckIntegrityCommand.h\
	ChecksumCheckIntegrityEntry.cc ChecksumCheckIntegrityEntry.h\
	message_digest_helper.cc message_digest_helper.h\
	Checksum.cc Checksum.h\
	ChunkChecksum.cc ChunkChecksum.h\
	MessageDigest.cc MessageDigest.h\
	MessageDigestImpl.h\
	HashFuncEntry.h
endif # ENABLE_MESSAGE_DIGEST

if ENABLE_BITTORRENT
SRCS += PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	Peer.cc Peer.h\
	PeerConnection.cc PeerConnection.h\
	PeerListenCommand.cc PeerListenCommand.h\
	RequestSlot.cc RequestSlot.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	PeerChokeCommand.cc PeerChokeCommand.h\
	SeedCriteria.h\
	TimeSeedCriteria.cc TimeSeedCriteria.h\
	ShareRatioSeedCriteria.cc ShareRatioSeedCriteria.h\
	UnionSeedCriteria.cc UnionSeedCriteria.h\
	SeedCheckCommand.cc SeedCheckCommand.h\
	AnnounceTier.cc AnnounceTier.h\
	AnnounceList.h AnnounceList.cc\
	PeerStorage.h\
	DefaultPeerStorage.cc DefaultPeerStorage.h\
	BtAnnounce.cc BtAnnounce.h\
	DefaultBtAnnounce.cc DefaultBtAnnounce.h\
	BtRuntime.cc BtRuntime.h\
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
	BtBitfieldMessageValidator.cc BtBitfieldMessageValidator.h\
	BtPieceMessageValidator.cc BtPieceMessageValidator.h\
	BtHandshakeMessageValidator.cc BtHandshakeMessageValidator.h\
	BtMessageFactory.h\
	DefaultBtMessageFactory.cc DefaultBtMessageFactory.h\
	BtMessageDispatcher.h\
	DefaultBtMessageDispatcher.cc DefaultBtMessageDispatcher.h\
	BtMessageReceiver.h\
	DefaultBtMessageReceiver.cc DefaultBtMessageReceiver.h\
	BtRequestFactory.h\
	DefaultBtRequestFactory.cc DefaultBtRequestFactory.h\
	BtCancelSendingPieceEvent.h\
	BtAbortOutstandingRequestEvent.cc BtAbortOutstandingRequestEvent.h\
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
	UTMetadataExtensionMessage.cc UTMetadataExtensionMessage.h\
	UTMetadataRequestExtensionMessage.cc\
	UTMetadataRequestExtensionMessage.h\
	UTMetadataRejectExtensionMessage.cc UTMetadataRejectExtensionMessage.h\
	UTMetadataDataExtensionMessage.cc UTMetadataDataExtensionMessage.h\
	UTMetadataRequestTracker.cc UTMetadataRequestTracker.h\
	UTMetadataRequestFactory.cc UTMetadataRequestFactory.h\
	UTMetadataPostDownloadHandler.cc UTMetadataPostDownloadHandler.h\
	magnet.cc magnet.h\
	DHTNode.cc DHTNode.h\
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
	DHTBucketTree.cc DHTBucketTree.h\
	DHTMessageCallback.h\
	DHTNodeLookupTaskCallback.cc DHTNodeLookupTaskCallback.h\
	DHTPingReplyMessageCallback.h\
	DHTPeerLookupTaskCallback.cc DHTPeerLookupTaskCallback.h\
	DHTAbstractTask.cc DHTAbstractTask.h\
	DHTTask.h\
	DHTPingTask.cc DHTPingTask.h\
	DHTTaskQueue.h\
	DHTTaskQueueImpl.cc DHTTaskQueueImpl.h\
	DHTTaskExecutor.cc DHTTaskExecutor.h\
	DHTBucketRefreshTask.cc DHTBucketRefreshTask.h\
	DHTAbstractNodeLookupTask.h\
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
	PeerAddrEntry.cc PeerAddrEntry.h\
	InitiatorMSEHandshakeCommand.cc InitiatorMSEHandshakeCommand.h\
	ReceiverMSEHandshakeCommand.cc ReceiverMSEHandshakeCommand.h\
	MSEHandshake.cc MSEHandshake.h\
	ARC4Encryptor.h\
	DHKeyExchange.h\
	BtConstants.h\
	BtLeecherStateChoke.cc BtLeecherStateChoke.h\
	BtSeederStateChoke.cc BtSeederStateChoke.h\
	RangeBtMessage.cc RangeBtMessage.h\
	IndexBtMessage.cc IndexBtMessage.h\
	ZeroBtMessage.cc ZeroBtMessage.h\
	RangeBtMessageValidator.cc RangeBtMessageValidator.h\
	IndexBtMessageValidator.cc IndexBtMessageValidator.h\
	ExtensionMessageRegistry.cc ExtensionMessageRegistry.h\
	bittorrent_helper.cc bittorrent_helper.h\
	BtStopDownloadCommand.cc BtStopDownloadCommand.h\
	PriorityPieceSelector.cc PriorityPieceSelector.h\
	LpdMessageDispatcher.cc LpdMessageDispatcher.h\
	LpdMessageReceiver.cc LpdMessageReceiver.h\
	LpdMessage.cc LpdMessage.h\
	LpdReceiveMessageCommand.cc LpdReceiveMessageCommand.h\
	LpdDispatchMessageCommand.cc LpdDispatchMessageCommand.h\
	bencode2.cc bencode2.h\
	BencodeParser.cc BencodeParser.h\
	ValueBaseBencodeParser.h\
	BencodeDiskWriter.h\
	BencodeDiskWriterFactory.h\
	MemoryBencodePreDownloadHandler.h
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
SRCS += Metalinker.cc Metalinker.h\
	MetalinkEntry.cc MetalinkEntry.h\
	MetalinkResource.cc MetalinkResource.h\
	MetalinkMetaurl.cc MetalinkMetaurl.h\
	MetalinkParserController.cc MetalinkParserController.h\
	MetalinkParserStateMachine.cc MetalinkParserStateMachine.h\
	MetalinkParserState.cc MetalinkParserState.h\
	MetalinkParserStateImpl.cc MetalinkParserStateImpl.h\
	MetalinkParserStateV3Impl.cc MetalinkParserStateV3Impl.h\
	MetalinkParserStateV4Impl.cc MetalinkParserStateV4Impl.h\
	Metalink2RequestGroup.cc Metalink2RequestGroup.h\
	MetalinkPostDownloadHandler.cc MetalinkPostDownloadHandler.h\
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

if HAVE_TIMEGETTIME
SRCS += clock_gettime_mingw.cc clock_gettime_mingw.h
endif # HAVE_TIMEGETTIME

if HAVE_MACH_ABSOLUTE_TIME
SRCS += clock_gettime_osx.cc clock_gettime_osx.h
endif # HAVE_MACH_ABSOLUTE_TIME

if HAVE_POLL
SRCS += PollEventPoll.cc PollEventPoll.h
endif # HAVE_POLL

if HAVE_PORT_ASSOCIATE
SRCS += PortEventPoll.cc PortEventPoll.h
endif # HAVE_PORT_ASSOCIATE

if HAVE_KQUEUE
SRCS += KqueueEventPoll.cc KqueueEventPoll.h
endif # HAVE_KQUEUE

AR = @AR@
noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a @LIBINTL@ @ALLOCA@ #-lprofiler
#aria2c_LDFLAGS = -pg
AM_CPPFLAGS =  -Wall\
	-I$(top_srcdir)/lib -I$(top_srcdir)/intl\
	-DLOCALEDIR=\"@localedir@\" -DCA_BUNDLE=\"$(ca_bundle)\" @DEFS@ #-pg
