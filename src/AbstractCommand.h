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
#ifndef D_ABSTRACT_COMMAND_H
#define D_ABSTRACT_COMMAND_H

#include "Command.h"

#include <vector>
#include <string>
#include <memory>

#include "TimerA2.h"

namespace aria2 {

class FileEntry;
class RequestGroup;
class CheckIntegrityEntry;
class DownloadContext;
class SegmentMan;
class PieceStorage;
class Request;
class DownloadEngine;
class Segment;
class SocketCore;
class Option;
class SocketRecvBuffer;
#ifdef ENABLE_ASYNC_DNS
class AsyncNameResolver;
class AsyncNameResolverMan;
#endif // ENABLE_ASYNC_DNS

class AbstractCommand : public Command {
private:
  std::shared_ptr<Request> req_;
  std::shared_ptr<FileEntry> fileEntry_;
  std::shared_ptr<SocketCore> socket_;
  std::shared_ptr<SocketRecvBuffer> socketRecvBuffer_;
  std::shared_ptr<SocketCore> readCheckTarget_;
  std::shared_ptr<SocketCore> writeCheckTarget_;

#ifdef ENABLE_ASYNC_DNS
  std::unique_ptr<AsyncNameResolverMan> asyncNameResolverMan_;
#endif // ENABLE_ASYNC_DNS

  RequestGroup* requestGroup_;
  DownloadEngine* e_;

  std::vector<std::shared_ptr<Segment>> segments_;

  Timer checkPoint_;
  Timer serverStatTimer_;
  std::chrono::seconds timeout_;

  bool checkSocketIsReadable_;
  bool checkSocketIsWritable_;

  bool incNumConnection_;

  int32_t calculateMinSplitSize() const;

  void useFasterRequest(const std::shared_ptr<Request>& fasterRequest);

  bool shouldProcess() const;

public:
  RequestGroup* getRequestGroup() const { return requestGroup_; }

  const std::shared_ptr<Request>& getRequest() const { return req_; }

  void setRequest(const std::shared_ptr<Request>& request);

  // Resets request_. This method is more efficient than
  // setRequest(std::shared_ptr<Request>());
  void resetRequest();

  const std::shared_ptr<FileEntry>& getFileEntry() const { return fileEntry_; }

  void setFileEntry(const std::shared_ptr<FileEntry>& fileEntry);

  DownloadEngine* getDownloadEngine() const { return e_; }

  const std::shared_ptr<SocketCore>& getSocket() const { return socket_; }

  std::shared_ptr<SocketCore>& getSocket() { return socket_; }

  void setSocket(const std::shared_ptr<SocketCore>& s);

  void createSocket();

  const std::shared_ptr<SocketRecvBuffer>& getSocketRecvBuffer() const
  {
    return socketRecvBuffer_;
  }

  const std::vector<std::shared_ptr<Segment>>& getSegments() const
  {
    return segments_;
  }

  // Resolves hostname.  The resolved addresses are stored in addrs
  // and first element is returned.  If resolve is not finished,
  // return empty string. In this case, call this function with same
  // arguments until resolved address is returned.  Exception is
  // thrown on error. port is used for retrieving cached addresses.
  std::string resolveHostname(std::vector<std::string>& addrs,
                              const std::string& hostname, uint16_t port);

  void tryReserved();

  void setReadCheckSocket(const std::shared_ptr<SocketCore>& socket);

  void setWriteCheckSocket(const std::shared_ptr<SocketCore>& socket);

  void disableReadCheckSocket();

  void disableWriteCheckSocket();

  /**
   * If pred == true, calls setReadCheckSocket(socket). Otherwise, calls
   * disableReadCheckSocket().
   */
  void setReadCheckSocketIf(const std::shared_ptr<SocketCore>& socket,
                            bool pred);
  /**
   * If pred == true, calls setWriteCheckSocket(socket). Otherwise, calls
   * disableWriteCheckSocket().
   */
  void setWriteCheckSocketIf(const std::shared_ptr<SocketCore>& socket,
                             bool pred);

  // Swaps socket_ with socket. This disables current read and write
  // check.
  void swapSocket(std::shared_ptr<SocketCore>& socket);

  std::chrono::seconds getTimeout() const { return timeout_; }

  void setTimeout(std::chrono::seconds timeout)
  {
    timeout_ = std::move(timeout);
  }

  void prepareForNextAction(std::unique_ptr<CheckIntegrityEntry> checkEntry);

  // Check if socket is connected. If socket is not connected and
  // there are other addresses to try, command is created using
  // InitiateConnectionCommandFactory and it is pushed to
  // DownloadEngine and returns false. If no addresses left, DlRetryEx
  // exception is thrown.
  bool checkIfConnectionEstablished(const std::shared_ptr<SocketCore>& socket,
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
   * If no valid proxy is defined, then returns std::shared_ptr<Request>().
   */
  std::shared_ptr<Request> createProxyRequest() const;

  // Returns proxy method for given protocol. Either V_GET or V_TUNNEL
  // is returned.  For HTTPS, always returns V_TUNNEL.
  const std::string& resolveProxyMethod(const std::string& protocol) const;

  const std::shared_ptr<Option>& getOption() const;

  const std::shared_ptr<DownloadContext>& getDownloadContext() const;
  const std::shared_ptr<SegmentMan>& getSegmentMan() const;
  const std::shared_ptr<PieceStorage>& getPieceStorage() const;

  Timer& getCheckPoint() { return checkPoint_; }

  void checkSocketRecvBuffer();

  void addCommandSelf();

protected:
  virtual bool prepareForRetry(time_t wait);

  virtual void onAbort();

  virtual bool executeInternal() = 0;

  // Returns true if the derived class wants to execute
  // executeInternal() unconditionally
  virtual bool noCheck() const { return false; }

public:
  AbstractCommand(
      cuid_t cuid, const std::shared_ptr<Request>& req,
      const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
      DownloadEngine* e, const std::shared_ptr<SocketCore>& s = nullptr,
      const std::shared_ptr<SocketRecvBuffer>& socketRecvBuffer = nullptr,
      bool incNumConnection = true);

  virtual ~AbstractCommand();

  virtual bool execute() CXX11_OVERRIDE;
};

// Returns proxy URI for given protocol.  If no proxy URI is defined,
// then returns an empty string.
std::string getProxyUri(const std::string& protocol, const Option* option);

} // namespace aria2

#endif // D_ABSTRACT_COMMAND_H
