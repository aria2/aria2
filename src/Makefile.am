bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc
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
	ConsoleDownloadEngine.cc ConsoleDownloadEngine.h\
	Segment.cc Segment.h\
	SegmentMan.cc SegmentMan.h\
	Util.cc Util.h\
	Request.cc Request.h\
	common.h\
	message.h\
	Exception.h\
	DlAbortEx.h\
	DlRetryEx.h\
	Logger.h\
	SimpleLogger.cc SimpleLogger.h\
	TransferEncoding.h\
	ChunkedEncoding.cc ChunkedEncoding.h\
	DiskWriter.h\
	DefaultDiskWriter.cc DefaultDiskWriter.h\
	PreAllocationDiskWriter.cc PreAllocationDiskWriter.h\
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	File.cc File.h\
	Option.cc Option.h\
	Base64.cc Base64.h\
	CookieBox.cc CookieBox.h\
	messageDigest.h\
	LogFactory.cc LogFactory.h\
	NullLogger.h\
	TimeA2.cc TimeA2.h\
	SharedHandle.h\
	HandleRegistry.h\
	FeatureConfig.cc FeatureConfig.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	RequestInfo.h\
	UrlRequestInfo.cc UrlRequestInfo.h\
	SpeedCalc.cc SpeedCalc.h\
	PeerStat.h\
	BitfieldMan.cc BitfieldMan.h\
	BitfieldManFactory.cc BitfieldManFactory.h\
	Randomizer.h\
	SimpleRandomizer.cc SimpleRandomizer.h
#	debug_new.cpp

if ENABLE_ASYNC_DNS
SRCS += NameResolver.cc NameResolver.h
endif # ENABLE_ASYNC_DNS

if ENABLE_BITTORRENT
SRCS += MetaEntry.h\
	Data.cc Data.h\
	Dictionary.cc Dictionary.h\
	List.cc List.h\
	MetaFileUtil.cc MetaFileUtil.h\
	MetaEntryVisitor.h\
	ShaVisitor.cc ShaVisitor.h\
	PeerConnection.cc PeerConnection.h\
	PeerMessageUtil.cc PeerMessageUtil.h\
	PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	Peer.cc Peer.h\
	TorrentDownloadEngine.cc TorrentDownloadEngine.h\
	TorrentConsoleDownloadEngine.cc TorrentConsoleDownloadEngine.h\
	PeerListenCommand.cc PeerListenCommand.h\
	Piece.cc Piece.h\
	RequestSlot.cc RequestSlot.h\
	TorrentAutoSaveCommand.cc TorrentAutoSaveCommand.h\
	Directory.cc Directory.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	MultiDiskWriter.cc MultiDiskWriter.h\
	DiskAdaptor.cc DiskAdaptor.h\
	CopyDiskAdaptor.cc CopyDiskAdaptor.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	FileEntry.cc FileEntry.h\
	TrackerUpdateCommand.cc TrackerUpdateCommand.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	PeerChokeCommand.cc PeerChokeCommand.h\
	HaveEraseCommand.cc HaveEraseCommand.h\
	TorrentRequestInfo.cc TorrentRequestInfo.h\
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
	PieceStorage.h\
	DefaultPieceStorage.cc DefaultPieceStorage.h\
	DefaultPeerStorage.cc DefaultPeerStorage.h\
	BtAnnounce.h\
	DefaultBtAnnounce.cc DefaultBtAnnounce.h\
	BtRegistry.cc BtRegistry.h\
	BtRuntime.h\
	BtProgressInfoFile.h\
	DefaultBtProgressInfoFile.cc DefaultBtProgressInfoFile.h\
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
	PeerObject.h
endif # ENABLE_BITTORRENT

if ENABLE_METALINK
SRCS += Metalinker.cc Metalinker.h\
	MetalinkEntry.cc MetalinkEntry.h\
	MetalinkResource.cc MetalinkResource.h\
	MetalinkProcessor.h\
	Xml2MetalinkProcessor.cc Xml2MetalinkProcessor.h\
	MetalinkRequestInfo.cc MetalinkRequestInfo.h
endif # ENABLE_METALINK

noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a @LIBINTL@ @ALLOCA@ @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@ @LIBARES_LIBS@\
	@LIBCARES_LIBS@
#aria2c_LDFLAGS = -pg
AM_CPPFLAGS =  -Wall\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	@LIBARES_CPPFLAGS@ @LIBCARES_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@ # -pg