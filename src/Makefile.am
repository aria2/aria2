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
	Segment.h\
	SegmentMan.cc SegmentMan.h\
	SegmentSplitter.cc SegmentSplitter.h\
	SplitFirstSegmentSplitter.cc SplitFirstSegmentSplitter.h\
	SplitSlowestSegmentSplitter.cc SplitSlowestSegmentSplitter.h\
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
	Time.cc Time.h\
	SharedHandle.h\
	FeatureConfig.cc FeatureConfig.h\
	DownloadEngineFactory.cc DownloadEngineFactory.h\
	RequestInfo.h\
	UrlRequestInfo.cc UrlRequestInfo.h

if ENABLE_BITTORRENT
SRCS += MetaEntry.h\
	Data.cc Data.h\
	Dictionary.cc Dictionary.h\
	List.cc List.h\
	MetaFileUtil.cc MetaFileUtil.h\
	MetaEntryVisitor.h\
	ShaVisitor.cc ShaVisitor.h\
	TorrentMan.cc TorrentMan.h\
	PeerConnection.cc PeerConnection.h\
	PeerMessageUtil.cc PeerMessageUtil.h\
	PeerAbstractCommand.cc PeerAbstractCommand.h\
	PeerInitiateConnectionCommand.cc PeerInitiateConnectionCommand.h\
	PeerInteractionCommand.cc PeerInteractionCommand.h\
	Peer.cc Peer.h\
	BitfieldMan.cc BitfieldMan.h\
	TorrentDownloadEngine.cc TorrentDownloadEngine.h\
	TorrentConsoleDownloadEngine.cc TorrentConsoleDownloadEngine.h\
	PeerListenCommand.cc PeerListenCommand.h\
	PeerMessage.cc PeerMessage.h\
	HandshakeMessage.cc HandshakeMessage.h\
	Piece.cc Piece.h\
	RequestSlot.cc RequestSlot.h\
	TorrentAutoSaveCommand.cc TorrentAutoSaveCommand.h\
	Directory.cc Directory.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	PeerInteraction.cc PeerInteraction.h\
	MultiDiskWriter.cc MultiDiskWriter.h\
	DiskAdaptor.cc DiskAdaptor.h\
	CopyDiskAdaptor.cc CopyDiskAdaptor.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	FileEntry.h\
	TrackerUpdateCommand.cc TrackerUpdateCommand.h\
	ByteArrayDiskWriter.cc ByteArrayDiskWriter.h\
	PeerChokeCommand.cc PeerChokeCommand.h\
	ChokeMessage.cc ChokeMessage.h\
	UnchokeMessage.cc UnchokeMessage.h\
	InterestedMessage.cc InterestedMessage.h\
	NotInterestedMessage.cc NotInterestedMessage.h\
	HaveMessage.cc HaveMessage.h\
	BitfieldMessage.cc BitfieldMessage.h\
	RequestMessage.cc RequestMessage.h\
	PieceMessage.cc PieceMessage.h\
	CancelMessage.cc CancelMessage.h\
	KeepAliveMessage.cc KeepAliveMessage.h\
	PortMessage.cc PortMessage.h\
	HaveAllMessage.cc HaveAllMessage.h\
	HaveNoneMessage.cc HaveNoneMessage.h\
	RejectMessage.cc RejectMessage.h\
	AllowedFastMessage.cc AllowedFastMessage.h\
	SuggestPieceMessage.cc SuggestPieceMessage.h\
	SimplePeerMessage.cc SimplePeerMessage.h\
	PeerMessageFactory.cc PeerMessageFactory.h\
	HaveEraseCommand.cc HaveEraseCommand.h\
	TorrentRequestInfo.cc TorrentRequestInfo.h
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
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@ @XML_LIBS@
#aria2c_LDFLAGS = -pg
AM_CPPFLAGS =  -Wall\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@ @XML_CPPFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@# -pg