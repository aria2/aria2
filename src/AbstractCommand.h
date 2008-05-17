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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "TimeA2.h"
#include "RequestGroupAware.h"

namespace aria2 {

class Request;
class DownloadEngine;
class RequestGroup;
class Segment;
class Exception;
class SocketCore;
#ifdef ENABLE_ASYNC_DNS
class AsyncNameResolver;
#endif // ENABLE_ASYNC_DNS

class AbstractCommand : public Command, public RequestGroupAware {
private:
  Time checkPoint;
  time_t timeout;
protected:
  SharedHandle<Request> req;
  DownloadEngine* e;
  SharedHandle<SocketCore> socket;
  std::deque<SharedHandle<Segment> > _segments;

#ifdef ENABLE_ASYNC_DNS
  SharedHandle<AsyncNameResolver> _asyncNameResolver;

  bool isAsyncNameResolverInitialized() const;

  void initAsyncNameResolver(const std::string& hostname);

  bool asyncResolveHostname();

  const std::deque<std::string>& getResolvedAddresses();
#endif // ENABLE_ASYNC_DNS

  void tryReserved();
  virtual bool prepareForRetry(time_t wait);
  virtual void onAbort();
  virtual bool executeInternal() = 0;

  void setReadCheckSocket(const SharedHandle<SocketCore>& socket);
  void setWriteCheckSocket(const SharedHandle<SocketCore>& socket);
  void disableReadCheckSocket();
  void disableWriteCheckSocket();

  void setTimeout(time_t timeout) { this->timeout = timeout; }

  void prepareForNextAction(Command* nextCommand = 0);

private:
  bool checkSocketIsReadable;
  bool checkSocketIsWritable;
  SharedHandle<SocketCore> readCheckTarget;
  SharedHandle<SocketCore> writeCheckTarget;
  bool nameResolverCheck;

#ifdef ENABLE_ASYNC_DNS

  void setNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver);

  void disableNameResolverCheck(const SharedHandle<AsyncNameResolver>& resolver);
  bool nameResolveFinished() const;

#endif // ENABLE_ASYNC_DNS
public:
  AbstractCommand(int32_t cuid, const SharedHandle<Request>& req,
		  RequestGroup* requestGroup, DownloadEngine* e,
		  const SharedHandle<SocketCore>& s = SharedHandle<SocketCore>());

  virtual ~AbstractCommand();
  bool execute();
};

} // namespace aria2

#endif // _D_ABSTRACT_COMMAND_H_
