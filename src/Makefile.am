bin_PROGRAMS = aria2c
aria2c_SOURCES = main.cc
SRCS =  Socket.cc Socket.h\
	SocketCore.cc SocketCore.h\
	Command.h\
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
	MetaEntry.h\
	Data.cc Data.h\
	Dictionary.cc Dictionary.h\
	List.cc List.h\
	MetaFileUtil.cc MetaFileUtil.h\
	MetaEntryVisitor.h\
	ShaVisitor.cc ShaVisitor.h\
	TrackerInitCommand.cc TrackerInitCommand.h\
	TrackerDownloadCommand.cc TrackerDownloadCommand.h\
	TrackerUpdateCommand.cc TrackerUpdateCommand.h\
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
	PendingMessage.cc PendingMessage.h\
	PeerMessage.cc PeerMessage.h\
	HandshakeMessage.h\
	Piece.cc Piece.h\
	RequestSlot.cc RequestSlot.h\
	RequestSlotMan.cc RequestSlotMan.h\
	TorrentAutoSaveCommand.cc TorrentAutoSaveCommand.h\
	Directory.cc Directory.h\
	TrackerWatcherCommand.cc TrackerWatcherCommand.h\
	messageDigest.h\
	SendMessageQueue.cc SendMessageQueue.h\
	MultiDiskWriter.cc MultiDiskWriter.h\
	DiskAdaptor.cc DiskAdaptor.h\
	CopyDiskAdaptor.cc CopyDiskAdaptor.h\
	DirectDiskAdaptor.cc DirectDiskAdaptor.h\
	MultiDiskAdaptor.cc MultiDiskAdaptor.h\
	FileEntry.h\
	LogFactory.cc LogFactory.h
noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a @LIBINTL@ @ALLOCA@ @LIBGNUTLS_LIBS@\
	@LIBGCRYPT_LIBS@ @OPENSSL_LIBS@
AM_CPPFLAGS =  -Wall\
	-I../lib -I../intl -I$(top_srcdir)/intl\
	@LIBGNUTLS_CFLAGS@ @LIBGCRYPT_CFLAGS@ @OPENSSL_CFLAGS@\
	-D_FILE_OFFSET_BITS=64 -DLOCALEDIR=\"$(localedir)\" @DEFS@
