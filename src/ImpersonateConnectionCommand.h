// Copyright (C) 2026 aria2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef D_IMPERSONATE_CONNECTION_COMMAND_H
#define D_IMPERSONATE_CONNECTION_COMMAND_H

#include "AbstractCommand.h"

namespace aria2 {

class ImpersonateConnectionCommand : public AbstractCommand {
public:
  ImpersonateConnectionCommand(
      cuid_t cuid, const std::shared_ptr<Request>& request,
      const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* group,
      DownloadEngine* engine);

protected:
  bool executeInternal() CXX11_OVERRIDE;
};

} // namespace aria2
#endif
