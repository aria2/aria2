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
#ifndef FTP_SOCKS_PROXY_CONNECT_CHAIN_H
#define FTP_SOCKS_PROXY_CONNECT_CHAIN_H

#include "ControlChain.h"
#include "ConnectCommand.h"
#include "DownloadEngine.h"
#include "SocksProxyCommand.h"
#include "FtpNegotiationCommand.h"
#ifdef HAVE_LIBSSH2
#  include "SftpNegotiationCommand.h"
#endif // HAVE_LIBSSH2

namespace aria2 {

struct FtpSocksProxyConnectChain : public ControlChain<ConnectCommand*> {
  FtpSocksProxyConnectChain() {}
  virtual ~FtpSocksProxyConnectChain() {}
  virtual int run(ConnectCommand* t, DownloadEngine* e) CXX11_OVERRIDE
  {
    auto c = make_unique<SocksProxyCommand>(
        t->getCuid(), t->getRequest(), t->getFileEntry(), t->getRequestGroup(),
        e, t->getProxyRequest(), t->getSocket(),
        [](SocksProxyCommand* cmd) -> std::unique_ptr<Command> {
#ifdef HAVE_LIBSSH2
          if (cmd->getRequest()->getProtocol() == "sftp") {
            return make_unique<SftpNegotiationCommand>(
                cmd->getCuid(), cmd->getRequest(), cmd->getFileEntry(),
                cmd->getRequestGroup(), cmd->getDownloadEngine(),
                cmd->getSocket());
          }
#endif // HAVE_LIBSSH2
          return make_unique<FtpNegotiationCommand>(
              cmd->getCuid(), cmd->getRequest(), cmd->getFileEntry(),
              cmd->getRequestGroup(), cmd->getDownloadEngine(),
              cmd->getSocket());
        });
    c->setStatus(Command::STATUS_ONESHOT_REALTIME);
    e->setNoWait(true);
    e->addCommand(std::move(c));
    return 0;
  }
};

} // namespace aria2

#endif // FTP_SOCKS_PROXY_CONNECT_CHAIN_H
