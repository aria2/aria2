// Copyright (C) 2026 aria2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef D_IMPERSONATE_TRANSPORT_H
#define D_IMPERSONATE_TRANSPORT_H

#include "common.h"
#include <memory>

namespace aria2 {

class Option;
class Request;
class SocketCore;

// An in-process, streaming HTTP adapter. The engine uses one end of a
// socketpair; a libcurl worker owns the other end and the remote connection.
// Workers never access the engine, its options, cookies, or disk state.
class ImpersonateTransport {
public:
  struct State;

  static std::shared_ptr<SocketCore>
  createSocket(const Request& request, const Option& option,
               const Request* proxy, bool proxyTunnel);

  ~ImpersonateTransport();
  void cancel();
  void checkError() const;

private:
  explicit ImpersonateTransport(std::shared_ptr<State> state);
  std::shared_ptr<State> state_;
};

} // namespace aria2
#endif
