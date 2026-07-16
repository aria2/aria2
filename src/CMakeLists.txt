add_executable(aria2c main.cc)

list(APPEND srcs A2STR.cc; AbstractAuthResolver.cc; AbstractCommand.cc; )
list(APPEND heds A2STR.h;  AbstractAuthResolver.h;  AbstractCommand.h;  a2algo.h; a2io.h; a2iterator.h; a2netcompat.h; a2time.h; )

list(APPEND srcs 
    a2algo.h;
	a2functional.h;
	a2io.h;
	a2iterator.h;
	a2netcompat.h;
	A2STR.cc; A2STR.h;
	a2time.h;
	AbstractAuthResolver.cc; AbstractAuthResolver.h;
	AbstractCommand.cc; AbstractCommand.h;
	AbstractDiskWriter.cc; AbstractDiskWriter.h;
	AbstractHttpServerResponseCommand.cc;
	AbstractHttpServerResponseCommand.h;
	AbstractOptionHandler.cc; AbstractOptionHandler.h;
	AbstractProxyRequestCommand.cc; AbstractProxyRequestCommand.h;
	AbstractProxyResponseCommand.cc; AbstractProxyResponseCommand.h;
	AbstractSingleDiskAdaptor.cc; AbstractSingleDiskAdaptor.h;
	AdaptiveFileAllocationIterator.cc; AdaptiveFileAllocationIterator.h;
	AdaptiveURISelector.cc; AdaptiveURISelector.h;
	AnonDiskWriterFactory.h;
	array_fun.h;
	AuthConfig.cc; AuthConfig.h;
	AuthConfigFactory.cc; AuthConfigFactory.h;
	AuthResolver.h;
	AutoSaveCommand.cc; AutoSaveCommand.h;
	BackupIPv4ConnectCommand.h; BackupIPv4ConnectCommand.cc;
	base32.cc; base32.h;
	base64.h;
	BinaryStream.h;
	bitfield.cc; bitfield.h;
	BitfieldMan.cc; BitfieldMan.h;
	BtProgressInfoFile.h;
	BufferedFile.cc; BufferedFile.h;
	ByteArrayDiskWriter.cc; ByteArrayDiskWriter.h;
	ByteArrayDiskWriterFactory.h;
	CheckIntegrityCommand.cc; CheckIntegrityCommand.h;
	CheckIntegrityDispatcherCommand.cc; CheckIntegrityDispatcherCommand.h;
	CheckIntegrityEntry.cc; CheckIntegrityEntry.h;
	CheckIntegrityMan.h;
	Checksum.cc; Checksum.h;
	ChecksumCheckIntegrityEntry.cc; ChecksumCheckIntegrityEntry.h;
	ChunkChecksum.cc; ChunkChecksum.h;
	ChunkedDecodingStreamFilter.cc; ChunkedDecodingStreamFilter.h;
	ColorizedStream.cc; ColorizedStream.h;
	Command.cc; Command.h;
	common.h;
	ConnectCommand.cc; ConnectCommand.h;
	console.cc; console.h;
	ConsoleStatCalc.cc; ConsoleStatCalc.h;
	ContentTypeRequestGroupCriteria.cc; ContentTypeRequestGroupCriteria.h;
	Context.cc; Context.h;
	ContextAttribute.cc; ContextAttribute.h;
	ControlChain.h;
	Cookie.cc; Cookie.h;
	CookieStorage.cc; CookieStorage.h;
	cookie_helper.cc; cookie_helper.h;
	CreateRequestCommand.cc; CreateRequestCommand.h;
	crypto_endian.h;
	CUIDCounter.cc; CUIDCounter.h;
	DefaultAuthResolver.cc; DefaultAuthResolver.h;
	DefaultBtProgressInfoFile.cc; DefaultBtProgressInfoFile.h;
	DefaultDiskWriter.cc; DefaultDiskWriter.h;
	DefaultDiskWriterFactory.cc; DefaultDiskWriterFactory.h;
	DefaultPieceStorage.cc; DefaultPieceStorage.h;
	DefaultStreamPieceSelector.cc; DefaultStreamPieceSelector.h;
	DelayedCommand.h;
	Dependency.h;
	DirectDiskAdaptor.cc; DirectDiskAdaptor.h;
	DiskAdaptor.cc; DiskAdaptor.h;
	DiskWriter.h;
	DiskWriterFactory.h;
	DlAbortEx.cc; DlAbortEx.h;
	DlRetryEx.cc; DlRetryEx.h;
	DNSCache.cc; DNSCache.h;
	DownloadCommand.cc; DownloadCommand.h;
	DownloadContext.cc; DownloadContext.h;
	DownloadEngine.cc; DownloadEngine.h;
	DownloadEngineFactory.cc; DownloadEngineFactory.h;
	DownloadFailureException.cc; DownloadFailureException.h;
	DownloadHandler.cc; DownloadHandler.h;
	DownloadHandlerConstants.cc; DownloadHandlerConstants.h;
	DownloadResult.cc; DownloadResult.h;
	download_handlers.cc; download_handlers.h;
	download_helper.cc; download_helper.h;
	error_code.h;
	Event.h;
	EventPoll.h;
	Exception.cc; Exception.h;
	FatalException.cc; FatalException.h;
	FeatureConfig.cc; FeatureConfig.h;
	FeedbackURISelector.cc; FeedbackURISelector.h;
	File.cc; File.h;
	FileAllocationCommand.cc; FileAllocationCommand.h;
	FileAllocationDispatcherCommand.cc; FileAllocationDispatcherCommand.h;
	FileAllocationEntry.cc; FileAllocationEntry.h;
	FileAllocationIterator.h;
	FileAllocationMan.h;
	FileEntry.cc; FileEntry.h;
	FillRequestGroupCommand.cc; FillRequestGroupCommand.h;
	fmt.cc; fmt.h;
	FtpConnection.cc; FtpConnection.h;
	FtpDownloadCommand.cc; FtpDownloadCommand.h;
	FtpFinishDownloadCommand.cc; FtpFinishDownloadCommand.h;
	FtpInitiateConnectionCommand.cc; FtpInitiateConnectionCommand.h;
	FtpNegotiationCommand.cc; FtpNegotiationCommand.h;
	FtpNegotiationConnectChain.h;
	FtpTunnelRequestCommand.cc; FtpTunnelRequestCommand.h;
	FtpTunnelRequestConnectChain.h;
	FtpTunnelResponseCommand.cc; FtpTunnelResponseCommand.h;
	GenericParser.h;
	GeomStreamPieceSelector.cc; GeomStreamPieceSelector.h;
	GroupId.cc; GroupId.h;
	GrowSegment.cc; GrowSegment.h;
	HashFuncEntry.h;
	HaveEraseCommand.cc; HaveEraseCommand.h;
	help_tags.cc; help_tags.h;
	HttpConnection.cc; HttpConnection.h;
	HttpDownloadCommand.cc; HttpDownloadCommand.h;
	HttpHeader.cc; HttpHeader.h;
	HttpHeaderProcessor.cc; HttpHeaderProcessor.h;
	HttpInitiateConnectionCommand.cc; HttpInitiateConnectionCommand.h;
	HttpListenCommand.cc; HttpListenCommand.h;
	HttpProxyRequestCommand.cc; HttpProxyRequestCommand.h;
	HttpProxyRequestConnectChain.h;
	HttpProxyResponseCommand.cc; HttpProxyResponseCommand.h;
	HttpRequest.cc; HttpRequest.h;
	HttpRequestCommand.cc; HttpRequestCommand.h;
	HttpRequestConnectChain.h;
	HttpResponse.cc; HttpResponse.h;
	HttpResponseCommand.cc; HttpResponseCommand.h;
	HttpServer.cc; HttpServer.h;
	HttpServerBodyCommand.cc; HttpServerBodyCommand.h;
	HttpServerCommand.cc; HttpServerCommand.h;
	HttpServerResponseCommand.cc; HttpServerResponseCommand.h;
	HttpSkipResponseCommand.cc; HttpSkipResponseCommand.h;
	IndexedList.h;
	InitiateConnectionCommand.cc; InitiateConnectionCommand.h;
	InitiateConnectionCommandFactory.cc; InitiateConnectionCommandFactory.h;
	InorderStreamPieceSelector.cc; InorderStreamPieceSelector.h;
	RandomStreamPieceSelector.cc; RandomStreamPieceSelector.h;
	InorderURISelector.cc; InorderURISelector.h;
	IOFile.cc; IOFile.h;
	IteratableChecksumValidator.cc; IteratableChecksumValidator.h;
	IteratableChunkChecksumValidator.cc; IteratableChunkChecksumValidator.h;
	IteratableValidator.h;
	json.cc; json.h;
	JsonDiskWriter.h;
	JsonParser.cc; JsonParser.h;
	Lock.h;
	LogFactory.cc; LogFactory.h;
	Logger.cc; Logger.h;
	LongestSequencePieceSelector.cc; LongestSequencePieceSelector.h;
	MemoryBufferPreDownloadHandler.h;
	MemoryPreDownloadHandler.h;
	message.h;
	MessageDigest.cc; MessageDigest.h;
	MessageDigestImpl.h;
	message_digest_helper.cc; message_digest_helper.h;
	MetadataInfo.cc; MetadataInfo.h;
	MetalinkHttpEntry.cc; MetalinkHttpEntry.h;
	MultiDiskAdaptor.cc; MultiDiskAdaptor.h;
	MultiFileAllocationIterator.cc; MultiFileAllocationIterator.h;
	MultiUrlRequestInfo.cc; MultiUrlRequestInfo.h;
	NameResolver.cc; NameResolver.h;
	Netrc.cc; Netrc.h;
	NetrcAuthResolver.cc; NetrcAuthResolver.h;
	NetStat.cc; NetStat.h;
	Notifier.cc; Notifier.h;
	NsCookieParser.cc; NsCookieParser.h;
	NullHandle.h;
	NullOutputFile.h;
	NullProgressInfoFile.h;
	NullSinkStreamFilter.cc; NullSinkStreamFilter.h;
	NullStatCalc.h;
	Option.cc; Option.h;
	OptionHandler.cc; OptionHandler.h;
	OptionHandlerException.cc; OptionHandlerException.h;
	OptionHandlerFactory.cc; OptionHandlerFactory.h;
	OptionHandlerImpl.cc; OptionHandlerImpl.h;
	OptionParser.cc; OptionParser.h;
	option_processing.cc;
	OutputFile.h;
	paramed_string.cc; paramed_string.h;
	PeerStat.cc; PeerStat.h;
	Piece.cc; Piece.h;
	PiecedSegment.cc; PiecedSegment.h;
	PieceHashCheckIntegrityEntry.cc; PieceHashCheckIntegrityEntry.h;
	PieceSelector.h;
	PieceStatMan.cc; PieceStatMan.h;
	PieceStorage.h;
	Platform.cc; Platform.h;
	PostDownloadHandler.h;
	PreDownloadHandler.h;
	prefs.cc; prefs.h;
	ProgressAwareEntry.h;
	ProtocolDetector.cc; ProtocolDetector.h;
	Randomizer.h;
	Range.cc; Range.h;
	RarestPieceSelector.cc; RarestPieceSelector.h;
	RealtimeCommand.cc; RealtimeCommand.h;
	RecoverableException.cc; RecoverableException.h;
	Request.cc; Request.h;
	RequestGroup.cc; RequestGroup.h;
	RequestGroupCriteria.h;
	RequestGroupEntry.cc; RequestGroupEntry.h;
	RequestGroupMan.cc; RequestGroupMan.h;
	RpcMethod.cc; RpcMethod.h;
	RpcMethodFactory.cc; RpcMethodFactory.h;
	RpcMethodImpl.cc; RpcMethodImpl.h;
	RpcRequest.cc; RpcRequest.h;
	RpcResponse.cc; RpcResponse.h;
	rpc_helper.cc; rpc_helper.h;
	SaveSessionCommand.h; SaveSessionCommand.cc;
	SegList.h;
	Segment.h;
	SegmentMan.cc; SegmentMan.h;
	SelectEventPoll.cc; SelectEventPoll.h;
	SequentialDispatcherCommand.h;
	SequentialPicker.h;
	ServerStat.cc; ServerStat.h;
	ServerStatMan.cc; ServerStatMan.h;
	SessionSerializer.cc; SessionSerializer.h;
	Signature.cc; Signature.h;
	SimpleRandomizer.cc; SimpleRandomizer.h;
	SingleFileAllocationIterator.cc; SingleFileAllocationIterator.h;
	SingletonHolder.h;
	SinkStreamFilter.cc; SinkStreamFilter.h;
	SocketBuffer.cc; SocketBuffer.h;
	SocketCore.cc; SocketCore.h;
	SocketRecvBuffer.cc; SocketRecvBuffer.h;
	SpeedCalc.cc; SpeedCalc.h;
	StatCalc.h;
	StreamCheckIntegrityEntry.cc; StreamCheckIntegrityEntry.h;
	StreamFileAllocationEntry.cc; StreamFileAllocationEntry.h;
	StreamFilter.cc; StreamFilter.h;
	StreamPieceSelector.h;
	StructParserStateMachine.h;
	TimeA2.cc; TimeA2.h;
	TimeBasedCommand.cc; TimeBasedCommand.h;
	TimedHaltCommand.cc; TimedHaltCommand.h;
	TimerA2.cc; TimerA2.h;
	timespec.h;
	TorrentAttribute.cc; TorrentAttribute.h;
	TransferStat.cc; TransferStat.h;
	TruncFileAllocationIterator.cc; TruncFileAllocationIterator.h;
	UnknownLengthPieceStorage.cc; UnknownLengthPieceStorage.h;
	UnknownOptionException.cc; UnknownOptionException.h;
	uri.cc; uri.h;
	UriListParser.cc; UriListParser.h;
	URIResult.cc; URIResult.h;
	URISelector.h;
	uri_split.c; uri_split.h;
	usage_text.h;
	util.cc; util.h;
	util_security.cc; util_security.h;
	ValueBase.cc; ValueBase.h;
	ValueBaseDiskWriter.h;
	ValueBaseJsonParser.h;
	ValueBaseStructParserState.h;
	ValueBaseStructParserStateImpl.cc; ValueBaseStructParserStateImpl.h;
	ValueBaseStructParserStateMachine.cc; ValueBaseStructParserStateMachine.h;
	version_usage.cc;
	wallclock.cc; wallclock.h;
	WatchProcessCommand.cc; WatchProcessCommand.h;
	WrDiskCache.cc; WrDiskCache.h;
	WrDiskCacheEntry.cc; WrDiskCacheEntry.h;
	XmlRpcRequestParserController.cc; XmlRpcRequestParserController.h;
	OpenedFileCounter.cc; OpenedFileCounter.;
	SHA1IOFile.cc; SHA1IOFile.h;
	EvictSocketPoolCommand.cc; EvictSocketPoolCommand.h;
	libssl_compat.h
)

if(MINGW_BUILD)
    list(APPEND srcs
        WinConsoleFile.cc; WinConsoleFile.h
    )
endif()

if(ENABLE_WEBSOCKET)
    list(APPEND srcs
        WebSocketInteractionCommand.cc; WebSocketInteractionCommand.h;
        WebSocketResponseCommand.cc; WebSocketResponseCommand.h;
        WebSocketSession.cc; WebSocketSession.h;
        WebSocketSessionMan.cc; WebSocketSessionMan.h
    )
endif()

if(NOT ENABLE_WEBSOCKET)
    list(APPEND srcs
        NullWebSocketSessionMan.h
    )
endif()

if(HAVE_SOME_XMLLIB)
    list(APPEND srcs
        ParserStateMachine.h;
	    XmlAttr.cc; XmlAttr.h;
	    XmlParser.cc; XmlParser.h
    )
endif()

if(HAVE_LIBXML2)
    list(APPEND srcs
        Xml2XmlParser.cc; Xml2XmlParser.h
    )
endif()

if(HAVE_LIBEXPAT)
    list(APPEND srcs
        ExpatXmlParser.cc; ExpatXmlParser.h
    )
endif()

if(ENABLE_XML_RPC)
    list(APPEND srcs
        XmlRpcDiskWriter.cc; XmlRpcDiskWriter.h;
	    XmlRpcRequestParserState.h;
	    XmlRpcRequestParserStateImpl.cc; XmlRpcRequestParserStateImpl.h;
	    XmlRpcRequestParserStateMachine.cc; XmlRpcRequestParserStateMachine.h;
    )
endif()

if(HAVE_SOME_FALLOCATE)
    list(APPEND srcs
        FallocFileAllocationIterator.cc; FallocFileAllocationIterator.h
    )
endif()

if(HAVE_EPOLL)
    list(APPEND srcs
        EpollEventPoll.cc; EpollEventPoll.h
    )
endif()

if(ENABLE_SSL)
    list(APPEND srcs
        TLSContext.h; TLSSession.h
    )
endif()

if(USE_APPLE_MD)
    list(APPEND srcs
        AppleMessageDigestImpl.cc
    )
endif()

if(HAVE_APPLETLS)
    list(APPEND srcs
        AppleTLSContext.cc; AppleTLSContext.h;
	    AppleTLSSession.cc; AppleTLSSession.h;
    )
endif()

if(HAVE_WINTLS)
    list(APPEND srcs
        WinTLSContext.cc; WinTLSContext.h;
	    WinTLSSession.cc; WinTLSSession.h;
    )
endif()

if(USE_INTERNAL_BIGNUM)
    list(APPEND srcs
        bignum.h;
	    InternalDHKeyExchange.cc; InternalDHKeyExchange.h;
    )
endif()

if(USE_INTERNAL_ARC4)
    list(APPEND srcs
        InternalARC4Encryptor.cc; InternalARC4Encryptor.h;
    )
endif()

if(USE_INTERNAL_MD)
    list(APPEND srcs
        InternalMessageDigestImpl.cc;
	    crypto_hash.cc; crypto_hash.h
    )
endif()

if(HAVE_LIBGNUTLS)
    list(APPEND srcs
        LibgnutlsTLSContext.cc; LibgnutlsTLSContext.h;
        LibgnutlsTLSSession.cc; LibgnutlsTLSSession.h;
    )
endif()

if(HAVE_LIBGCRYPT)
    list(APPEND srcs
        LibgcryptARC4Encryptor.cc; LibgcryptARC4Encryptor.h;
        LibgcryptDHKeyExchange.cc; LibgcryptDHKeyExchange.h;
    )

    if(USE_LIBGCRYPT_MD)
        list(APPEND srcs
            LibgcryptMessageDigestImpl.cc
        )
    endif()
endif()

if(HAVE_LIBNETTLE)
    list(APPEND srcs
        LibnettleARC4Encryptor.cc; LibnettleARC4Encryptor.h
    )
    
    if(USE_LIBNETTLE_MD)
        list(APPEND srcs
            LibnettleMessageDigestImpl.cc
        )
    endif()
endif()

if(HAVE_LIBGMP)
    list(APPEND srcs
        a2gmp.cc; a2gmp.h;
        LibgmpDHKeyExchange.cc; LibgmpDHKeyExchange.h;
    )
endif()

if(HAVE_OPENSSL)
    list(APPEND srcs
        LibsslARC4Encryptor.cc; LibsslARC4Encryptor.h;
        LibsslDHKeyExchange.cc; LibsslDHKeyExchange.h;
    )

    if(NOT HAVE_APPLETLS)
        list(APPEND srcs
            LibsslTLSContext.cc; LibsslTLSContext.h;
            LibsslTLSSession.cc; LibsslTLSSession.h;
        )
    endif()

    if(USE_OPENSSL_MD)
        list(APPEND srcs
            LibsslMessageDigestImpl.cc
        )
    endif()
endif()

if(HAVE_ZLIB)
    list(APPEND srcs
        GZipDecodingStreamFilter.cc; GZipDecodingStreamFilter.h;
	    GZipEncoder.cc; GZipEncoder.h;
	    GZipFile.cc; GZipFile.h;
	    Adler32MessageDigestImpl.cc; Adler32MessageDigestImpl.h;
    )
endif()

if(HAVE_SQLITE3)
    list(APPEND srcs
       Sqlite3CookieParser.cc; Sqlite3CookieParser.h;
	    Sqlite3CookieParserImpl.cc; Sqlite3CookieParserImpl.h;
    )
endif()

if(HAVE_LIBSSH2)
    list(APPEND srcs
        SSHSession.cc; SSHSession.h;
	    SftpNegotiationCommand.cc; SftpNegotiationCommand.h;
	    SftpNegotiationConnectChain.h;
	    SftpDownloadCommand.cc; SftpDownloadCommand.h;
	    SftpFinishDownloadCommand.cc; SftpFinishDownloadCommand.h;
    )
endif()

if(ENABLE_ASYNC_DNS)
    list(APPEND srcs
        AsyncNameResolver.cc; AsyncNameResolver.h;
	    AsyncNameResolverMan.cc; AsyncNameResolverMan.h;
    )
endif()

if(ENABLE_BITTORRENT)
    list(APPEND srcs
        AbstractBtMessage.cc; AbstractBtMessage.h;
	    ActivePeerConnectionCommand.cc; ActivePeerConnectionCommand.h;
	    AnnounceList.h; AnnounceList.cc;
	    AnnounceTier.cc; AnnounceTier.h;
	    ARC4Encryptor.h;
	    bencode2.cc; bencode2.h;
	    BencodeDiskWriter.h;
	    BencodeDiskWriterFactory.h;
	    BencodeParser.cc; BencodeParser.h;
	    bittorrent_helper.cc; bittorrent_helper.h;
	    BtAbortOutstandingRequestEvent.cc; BtAbortOutstandingRequestEvent.h;
	    BtAllowedFastMessage.cc; BtAllowedFastMessage.h;
	    BtAnnounce.cc; BtAnnounce.h;
	    BtBitfieldMessage.cc; BtBitfieldMessage.h;
	    BtBitfieldMessageValidator.cc; BtBitfieldMessageValidator.h;
	    BtCancelMessage.cc; BtCancelMessage.h;
	    BtCancelSendingPieceEvent.h;
	    BtCheckIntegrityEntry.cc; BtCheckIntegrityEntry.h;
	    BtChokeMessage.cc; BtChokeMessage.h;
	    BtChokingEvent.h;
	    BtConstants.h;
	    BtDependency.cc; BtDependency.h;
	    BtExtendedMessage.cc; BtExtendedMessage.h;
	    BtFileAllocationEntry.cc; BtFileAllocationEntry.h;
	    BtHandshakeMessage.cc; BtHandshakeMessage.h;
	    BtHandshakeMessageValidator.cc; BtHandshakeMessageValidator.h;
	    BtHaveAllMessage.cc; BtHaveAllMessage.h;
	    BtHaveMessage.cc; BtHaveMessage.h;
	    BtHaveNoneMessage.cc; BtHaveNoneMessage.h;
	    BtInteractive.h;
	    BtInterestedMessage.cc; BtInterestedMessage.h;
	    BtKeepAliveMessage.cc; BtKeepAliveMessage.h;
	    BtLeecherStateChoke.cc; BtLeecherStateChoke.h;
	    BtMessage.h;
	    BtMessageDispatcher.h;
	    BtMessageFactory.h;
	    BtMessageReceiver.h;
	    BtMessageValidator.h;
	    BtNotInterestedMessage.cc; BtNotInterestedMessage.h;
	    BtPieceMessage.cc; BtPieceMessage.h;
	    BtPieceMessageValidator.cc; BtPieceMessageValidator.h;
	    BtPortMessage.cc; BtPortMessage.h;
	    BtPostDownloadHandler.cc; BtPostDownloadHandler.h;
	    BtRegistry.cc; BtRegistry.h;
	    BtRejectMessage.cc; BtRejectMessage.h;
	    BtRequestFactory.h;
	    BtRequestMessage.cc; BtRequestMessage.h;
	    BtRuntime.cc; BtRuntime.h;
	    BtSeederStateChoke.cc; BtSeederStateChoke.h;
	    BtSetup.cc; BtSetup.h;
	    BtStopDownloadCommand.cc; BtStopDownloadCommand.h;
	    BtSuggestPieceMessage.cc; BtSuggestPieceMessage.h;
	    BtUnchokeMessage.cc; BtUnchokeMessage.h;
	    DefaultBtAnnounce.cc; DefaultBtAnnounce.h;
	    DefaultBtInteractive.cc; DefaultBtInteractive.h;
	    DefaultBtMessageDispatcher.cc; DefaultBtMessageDispatcher.h;
	    DefaultBtMessageFactory.cc; DefaultBtMessageFactory.h;
	    DefaultBtMessageReceiver.cc; DefaultBtMessageReceiver.h;
	    DefaultBtRequestFactory.cc; DefaultBtRequestFactory.h;
	    DefaultExtensionMessageFactory.cc; DefaultExtensionMessageFactory.h;
	    DefaultPeerStorage.cc; DefaultPeerStorage.h;
	    DHKeyExchange.h;
	    DHTAbstractMessage.cc; DHTAbstractMessage.h;
	    DHTAbstractNodeLookupTask.h;
	    DHTAbstractTask.cc; DHTAbstractTask.h;
	    DHTAnnouncePeerMessage.cc; DHTAnnouncePeerMessage.h;
	    DHTAnnouncePeerReplyMessage.cc; DHTAnnouncePeerReplyMessage.h;
	    DHTAutoSaveCommand.cc; DHTAutoSaveCommand.h;
	    DHTBucket.cc; DHTBucket.h;
	    DHTBucketRefreshCommand.cc; DHTBucketRefreshCommand.h;
	    DHTBucketRefreshTask.cc; DHTBucketRefreshTask.h;
	    DHTBucketTree.cc; DHTBucketTree.h;
	    DHTConnection.h;
	    DHTConnectionImpl.cc; DHTConnectionImpl.h;
	    DHTConstants.h;
	    DHTEntryPointNameResolveCommand.cc; DHTEntryPointNameResolveCommand.h;
	    DHTFindNodeMessage.cc; DHTFindNodeMessage.h;
	    DHTFindNodeReplyMessage.cc; DHTFindNodeReplyMessage.h;
	    DHTGetPeersCommand.cc; DHTGetPeersCommand.h;
	    DHTGetPeersMessage.cc; DHTGetPeersMessage.h;
	    DHTGetPeersReplyMessage.cc; DHTGetPeersReplyMessage.h;
	    DHTIDCloser.h;
	    DHTInteractionCommand.cc; DHTInteractionCommand.h;
	    DHTMessage.cc; DHTMessage.h;
	    DHTMessageCallback.h;
	    DHTMessageDispatcher.h;
	    DHTMessageDispatcherImpl.cc; DHTMessageDispatcherImpl.h;
	    DHTMessageEntry.cc; DHTMessageEntry.h;
	    DHTMessageFactory.h;
	    DHTMessageFactoryImpl.cc; DHTMessageFactoryImpl.h;
	    DHTMessageReceiver.cc; DHTMessageReceiver.h;
	    DHTMessageTracker.cc; DHTMessageTracker.h;
	    DHTMessageTrackerEntry.cc; DHTMessageTrackerEntry.h;
	    DHTNode.cc; DHTNode.h;
	    DHTNodeLookupEntry.cc; DHTNodeLookupEntry.h;
	    DHTNodeLookupTask.cc; DHTNodeLookupTask.h;
	    DHTNodeLookupTaskCallback.cc; DHTNodeLookupTaskCallback.h;
	    DHTPeerAnnounceCommand.cc; DHTPeerAnnounceCommand.h;
	    DHTPeerAnnounceEntry.cc; DHTPeerAnnounceEntry.h;
	    DHTPeerAnnounceStorage.cc; DHTPeerAnnounceStorage.h;
	    DHTPeerLookupTask.cc; DHTPeerLookupTask.h;
	    DHTPeerLookupTaskCallback.cc; DHTPeerLookupTaskCallback.h;
	    DHTPingMessage.cc; DHTPingMessage.h;
	    DHTPingReplyMessage.cc; DHTPingReplyMessage.h;
	    DHTPingReplyMessageCallback.h;
	    DHTPingTask.cc; DHTPingTask.h;
	    DHTQueryMessage.cc; DHTQueryMessage.h;
	    DHTRegistry.cc; DHTRegistry.h;
	    DHTReplaceNodeTask.cc; DHTReplaceNodeTask.h;
	    DHTResponseMessage.cc; DHTResponseMessage.h;
	    DHTRoutingTable.cc; DHTRoutingTable.h;
	    DHTRoutingTableDeserializer.cc; DHTRoutingTableDeserializer.h;
	    DHTRoutingTableSerializer.cc; DHTRoutingTableSerializer.h;
	    DHTSetup.cc; DHTSetup.h;
	    DHTTask.h;
	    DHTTaskExecutor.cc; DHTTaskExecutor.h;
	    DHTTaskFactory.h;
	    DHTTaskFactoryImpl.cc; DHTTaskFactoryImpl.h;
	    DHTTaskQueue.h;
	    DHTTaskQueueImpl.cc; DHTTaskQueueImpl.h;
	    DHTTokenTracker.cc; DHTTokenTracker.h;
	    DHTTokenUpdateCommand.cc; DHTTokenUpdateCommand.h;
	    DHTUnknownMessage.cc; DHTUnknownMessage.h;
	    ExtensionMessage.h;
	    ExtensionMessageFactory.h;
	    ExtensionMessageRegistry.cc; ExtensionMessageRegistry.h;
	    HandshakeExtensionMessage.cc; HandshakeExtensionMessage.h;
	    IndexBtMessage.cc; IndexBtMessage.h;
	    IndexBtMessageValidator.cc; IndexBtMessageValidator.h;
	    InitiatorMSEHandshakeCommand.cc; InitiatorMSEHandshakeCommand.h;
	    LpdDispatchMessageCommand.cc; LpdDispatchMessageCommand.h;
	    LpdMessage.cc; LpdMessage.h;
	    LpdMessageDispatcher.cc; LpdMessageDispatcher.h;
	    LpdMessageReceiver.cc; LpdMessageReceiver.h;
	    LpdReceiveMessageCommand.cc; LpdReceiveMessageCommand.h;
	    magnet.cc; magnet.h;
	    MemoryBencodePreDownloadHandler.h;
	    MSEHandshake.cc; MSEHandshake.h;
	    NameResolveCommand.cc; NameResolveCommand.h;
	    Peer.cc; Peer.h;
	    PeerAbstractCommand.cc; PeerAbstractCommand.h;
	    PeerAddrEntry.cc; PeerAddrEntry.h;
	    PeerChokeCommand.cc; PeerChokeCommand.h;
	    PeerConnection.cc; PeerConnection.h;
	    PeerInitiateConnectionCommand.cc; PeerInitiateConnectionCommand.h;
	    PeerInteractionCommand.cc; PeerInteractionCommand.h;
	    PeerListenCommand.cc; PeerListenCommand.h;
	    PeerReceiveHandshakeCommand.cc; PeerReceiveHandshakeCommand.h;
	    PeerSessionResource.cc; PeerSessionResource.h;
	    PeerStorage.h;
	    PriorityPieceSelector.cc; PriorityPieceSelector.h;
	    RangeBtMessage.cc; RangeBtMessage.h;
	    RangeBtMessageValidator.cc; RangeBtMessageValidator.h;
	    ReceiverMSEHandshakeCommand.cc; ReceiverMSEHandshakeCommand.h;
	    RequestSlot.cc; RequestSlot.h;
	    SeedCheckCommand.cc; SeedCheckCommand.h;
	    SeedCriteria.h;
	    ShareRatioSeedCriteria.cc; ShareRatioSeedCriteria.h;
	    SimpleBtMessage.cc; SimpleBtMessage.h;
	    TimeSeedCriteria.cc; TimeSeedCriteria.h;
	    TrackerWatcherCommand.cc; TrackerWatcherCommand.h;
	    UDPTrackerClient.cc; UDPTrackerClient.h;
	    UDPTrackerRequest.cc; UDPTrackerRequest.h;
	    UnionSeedCriteria.cc; UnionSeedCriteria.h;
	    UTMetadataDataExtensionMessage.cc; UTMetadataDataExtensionMessage.h;
	    UTMetadataExtensionMessage.cc; UTMetadataExtensionMessage.h;
	    UTMetadataPostDownloadHandler.cc; UTMetadataPostDownloadHandler.h;
	    UTMetadataRejectExtensionMessage.cc; UTMetadataRejectExtensionMessage.h;
	    UTMetadataRequestExtensionMessage.cc;
	    UTMetadataRequestExtensionMessage.h;
	    UTMetadataRequestFactory.cc; UTMetadataRequestFactory.h;
	    UTMetadataRequestTracker.cc; UTMetadataRequestTracker.h;
	    UTPexExtensionMessage.cc; UTPexExtensionMessage.h;
	    ValueBaseBencodeParser.h;
	    XORCloser.h
	    ZeroBtMessage.cc; ZeroBtMessage.h;
    )
endif()

if(ENABLE_METALINK)
    list(APPEND srcs
        Metalink2RequestGroup.cc; Metalink2RequestGroup.h;
	    MetalinkEntry.cc; MetalinkEntry.h;
	    Metalinker.cc; Metalinker.h;
	    MetalinkMetaurl.cc; MetalinkMetaurl.h;
	    MetalinkParserController.cc; MetalinkParserController.h;
	    MetalinkParserState.cc; MetalinkParserState.h;
	    MetalinkParserStateImpl.cc; MetalinkParserStateImpl.h;
	    MetalinkParserStateMachine.cc; MetalinkParserStateMachine.h;
	    MetalinkParserStateV3Impl.cc; MetalinkParserStateV3Impl.h;
	    MetalinkParserStateV4Impl.cc; MetalinkParserStateV4Impl.h;
	    MetalinkPostDownloadHandler.cc; MetalinkPostDownloadHandler.h;
	    MetalinkResource.cc; MetalinkResource.h;
	    metalink_helper.cc; metalink_helper.;
    )
endif()

if(NOT HAVE_ASCTIME_R)
    list(APPEND srcs
        asctime_r.c; asctime_r.h
    )
endif()

if(NOT HAVE_BASENAME)
    list(APPEND srcs
       libgen.c; libgen.h
    )
endif()

if(NOT HAVE_GETADDRINFO)
    list(APPEND srcs
        getaddrinfo.c; getaddrinfo.h
    )
endif()

if(NOT HAVE_GAI_STRERROR)
    list(APPEND srcs
        gai_strerror.c; gai_strerror.h
    )
endif()

if(NOT HAVE_GETTIMEOFDAY)
    list(APPEND srcs
        gettimeofday.c; gettimeofday.h
    )
endif()

if(NOT HAVE_LOCALTIME_R)
    list(APPEND srcs
        localtime_r.c; localtime_r.h
    )
endif()

if(NOT HAVE_STRPTIME)
    list(APPEND srcs
        strptime.c; strptime.h
    )
endif()

if(NOT HAVE_TIMEGM)
    list(APPEND srcs
        timegm.c; timegm.h
    )
endif()

if(NOT HAVE_DAEMON)
    list(APPEND srcs
        daemon.cc; daemon.h
    )
endif()

if(HAVE_POLL)
    list(APPEND srcs
        PollEventPoll.cc; PollEventPoll.h
    )
endif()

if(HAVE_PORT_ASSOCIATE)
    list(APPEND srcs
        PortEventPoll.cc; PortEventPoll.h
    )
endif()

if(HAVE_KQUEUE)
    list(APPEND srcs
        KqueueEventPoll.cc KqueueEventPoll.h
    )
endif()

if(HAVE_LIBUV)
    list(APPEND srcs
        LibuvEventPoll.cc; LibuvEventPoll.h
    )
endif()

# TODOm: AR = @AR@

if(ENABLE_LIBARIA2)



target_sources(aria2c
    PUBLIC
        ${srcs}
   
    PUBLIC
        FILE_SET aria2Headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_LIST_DIR}
        FILES
            ${hedrs}
)

