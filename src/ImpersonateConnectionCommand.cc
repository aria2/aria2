// Copyright (C) 2026 aria2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#include "ImpersonateConnectionCommand.h"

#include "DownloadEngine.h"
#include "HttpConnection.h"
#include "HttpRequestCommand.h"
#include "ImpersonateTransport.h"
#include "Request.h"
#include "SocketRecvBuffer.h"
#include "prefs.h"
#include "util.h"

namespace aria2 {

ImpersonateConnectionCommand::ImpersonateConnectionCommand(
    cuid_t cuid, const std::shared_ptr<Request>& request,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* group,
    DownloadEngine* engine)
    : AbstractCommand(cuid, request, fileEntry, group, engine)
{
  setStatusRealtime();
}

bool ImpersonateConnectionCommand::executeInternal()
{
  auto proxy = createProxyRequest();
  auto socket = ImpersonateTransport::createSocket(
      *getRequest(), *getOption(), proxy.get(),
      resolveProxyMethod(getRequest()->getProtocol()) == V_TUNNEL);
  auto connection = std::make_shared<HttpConnection>(
      getCuid(), socket, std::make_shared<SocketRecvBuffer>(socket));
  getDownloadEngine()->addCommand(make_unique<HttpRequestCommand>(
      getCuid(), getRequest(), getFileEntry(), getRequestGroup(), connection,
      getDownloadEngine(), socket));
  getDownloadEngine()->setNoWait(true);
  return true;
}

} // namespace aria2
