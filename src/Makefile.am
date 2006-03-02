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
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	File.cc File.h\
	Option.cc Option.h\
	Base64.cc Base64.h\
	CookieBox.cc CookieBox.h
noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a
FLAGS = -Wall -D_FILE_OFFSET_BITS=64 -I../lib
libaria2c_a_CXXFLAGS = $(FLAGS)
aria2c_CXXFLAGS = $(FLAGS)
