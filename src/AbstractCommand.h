/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef _D_ABSTRACT_COMMAND_H_
#define _D_ABSTRACT_COMMAND_H_

#include "Command.h"
#include "SharedHandle.h"
#include "TimerA2.h"
#include "FileEntry.h"
#include "RequestGroup.h"

namespace aria2 {

class Request;
class DownloadEngine;
class Segment;
class Exception;
class SocketCore;
class Option;
#ifdef ENABLE_ASYNC_DNS
class AsyncNameResolver;
#endif // ENABLE_ASYNC_DNS

class AbstractCommand : public Command {
private:
  Timer _checkPoint;
  time_t _timeout;

  RequestGroup* _requestGroup;
  SharedHandle<Request> _req;
  SharedHandle<FileEntry> _fileEntry;
  DownloadEngine* _e;
  SharedHandle<SocketCore> _socket;
  std::vector<SharedHandle<Segment> > _segments;

#ifdef ENABLE_ASYNC_DNS
  SharedHandle<AsyncNameResolver> _asyncNameResolver;
#endif // ENABLE_ASYNC_DNS

  bool _checkSocketIsReadable;
  bool _checkSocketIsWritable;
  SharedHandle<SocketCore> _readCheckTarget;
  SharedHandle<SocketCore> _writeCheckTarget;
  bool _nameResolverCheck;

#ifdef ENABLE_ASYNC_DNS
  void setNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver);

  void disableNameResolverCheck
  (const SharedHandle<AsyncNameResolver>& resolver);

  bool nameResolveFinished() const;
#endif // ENABLE_ASYNC_DNS
protected:
  RequestGroup* getRequestGroup() const
  {
    return _requestGroup;
  }

  const SharedHandle<Request>& getRequest() const
  {
    return _req;
  }

  void setRequest(const SharedHandle<Request>& request)
  {
    _req = request;
  }

  const SharedHandle<FileEntry>& getFileEntry() const
  {
    return _fileEntry;
  }

  void setFileEntry(const SharedHandle<FileEntry>& fileEntry)
  {
    _fileEntry = fileEntry;
  }

  DownloadEngine* getDownloadEngine() const
  {
    return _e;
  }

  const SharedHandle<SocketCore>& getSocket() const
  {
    return _socket;
  }

  void setSocket(const SharedHandle<SocketCore>& s)
  {
    _socket = s;
  }

  void createSocket();

  const std::vector<SharedHandle<Segment> >& getSegments() const
  {
    return _segments;
  }

#ifdef ENABLE_ASYNC_DNS
  bool isAsyncNameResolverInitialized() const;

  void initAsyncNameResolver(const std::string& hostname);

  bool asyncResolveHostname();

  const std::vector<std::string>& getResolvedAddresses();
#endif // ENABLE_ASYNC_DNS

  // Resolves hostname.  The resolved addresses are stored in addrs
  // and first element is returned.  If resolve is not finished,
  // return empty string. In this case, call this function with same
  // arguments until resolved address is returned.  Exception is
  // thrown on error. port is used for retrieving cached addresses.
  std::string resolveHostname
  (std::vector<std::string>& addrs, const std::string& hostname, uint16_t port);

  void tryReserved();
  virtual bool prepareForRetry(time_t wait);
  virtual void onAbort();
  virtual bool executeInternal() = 0;

  void setReadCheckSocket(const SharedHandle<SocketCore>& socket);
  void setWriteCheckSocket(const SharedHandle<SocketCore>& socket);
  void disableReadCheckSocket();
  void disableWriteCheckSocket();

  /**
   * If pred == true, calls setReadCheckSocket(socket). Otherwise, calls
   * disableReadCheckSocket().
   */
  void setReadCheckSocketIf(const SharedHandle<SocketCore>& socket, bool pred);
  /**
   * If pred == true, calls setWriteCheckSocket(socket). Otherwise, calls
   * disableWriteCheckSocket().
   */
  void setWriteCheckSocketIf(const SharedHandle<SocketCore>& socket, bool pred);

  void setTimeout(time_t timeout) { _timeout = timeout; }

  void prepareForNextAction(Command* nextCommand = 0);

  // Check if socket is connected. If socket is not connected and
  // there are other addresses to try, command is created using
  // InitiateConnectionCommandFactory and it is pushed to
  // DownloadEngine and returns false. If no addresses left, DlRetryEx
  // exception is thrown.
  bool checkIfConnectionEstablished
  (const SharedHandle<SocketCore>& socket,
   const std::string& connectedHostname,
   const std::string& connectedAddr,
   uint16_t connectedPort);

  /*
   * Returns true if proxy for the procol indicated by Request::getProtocol()
   * is defined. Otherwise, returns false.
   */
  bool isProxyDefined() const;

  /*
   * Creates Request object for proxy URI and returns it.
   * If no valid proxy is defined, then returns SharedHandle<Request>().
   */
  SharedHandle<Request> createProxyRequest() const;

  // Returns proxy method for given protocol. Either V_GET or V_TUNNEL
  // is returned.  For HTTPS, always returns V_TUNNEL.
  const std::string& resolveProxyMethod(const std::string& protocol) const;

  const SharedHandle<Option>& getOption() const;

  const SharedHandle<DownloadContext>& getDownloadContext() const
  {
    return _requestGroup->getDownloadContext();
  }

  const SharedHandle<SegmentMan>& getSegmentMan() const
  {
    return _requestGroup->getSegmentMan();
  }

  const SharedHandle<PieceStorage>& getPieceStorage() const
  {
    return _requestGroup->getPieceStorage();
  }
public:
  AbstractCommand(cuid_t cuid, const SharedHandle<Request>& req,
                  const SharedHandle<FileEntry>& fileEntry,
                  RequestGroup* requestGroup, DownloadEngine* e,
                  const SharedHandle<SocketCore>& s = SharedHandle<SocketCore>());

  virtual ~AbstractCommand();
  bool execute();
};

} // namespace aria2

#endif // _D_ABSTRACT_COMMAND_H_
