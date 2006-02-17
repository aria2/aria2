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
	HttpConnection.cc HttpConnection.h\
	SleepCommand.cc SleepCommand.h\
	DownloadEngine.cc DownloadEngine.h\
	Segment.h\
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
	AbstractDiskWriter.cc AbstractDiskWriter.h\
	File.cc File.h\
	Option.cc Option.h\
	Base64.cc Base64.h\
	CookieBox.cc CookieBox.h
noinst_LIBRARIES = libaria2c.a
libaria2c_a_SOURCES = $(SRCS)
aria2c_LDADD = libaria2c.a
FLAGS = -Wall -D_FILE_OFFSET_BITS=64
libaria2c_a_CXXFLAGS = $(FLAGS)
aria2c_CXXFLAGS = $(FLAGS)
