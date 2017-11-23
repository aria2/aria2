/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2015 Tatsuhiro Tsujikawa
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
#ifndef SSH_SESSION_H
#define SSH_SESSION_H

#include "common.h"
#include "a2netcompat.h"

#include <string>

#include <libssh2.h>
#include <libssh2_sftp.h>

namespace aria2 {

enum SSHDirection { SSH_WANT_READ = 1, SSH_WANT_WRITE };

enum SSHErrorCode {
  SSH_ERR_OK = 0,
  SSH_ERR_ERROR = -1,
  SSH_ERR_WOULDBLOCK = -2
};

class SSHSession {
public:
  SSHSession();

  // MUST deallocate all resources
  ~SSHSession();

  SSHSession(const SSHSession&) = delete;
  SSHSession& operator=(const SSHSession&) = delete;

  // Initializes SSH session. The |sockfd| is the underlying
  // transport socket. This function returns SSH_ERR_OK if it
  // succeeds, or SSH_ERR_ERROR.
  int init(sock_t sockfd);

  // Closes the SSH session. Don't close underlying transport
  // socket. This function returns SSH_ERR_OK if it succeeds, or
  // SSH_ERR_ERROR.
  int closeConnection();

  int gracefulShutdown();

  // Returns SSH_WANT_READ if SSH session needs more data from remote
  // endpoint to proceed, or SSH_WANT_WRITE if SSH session needs to
  // write more data to proceed. If SSH session needs neither read nor
  // write data at the moment, SSH_WANT_READ must be returned.
  int checkDirection();

  // Sends |data| with length |len|. This function returns the number
  // of bytes sent if it succeeds, or SSH_ERR_WOULDBLOCK if the
  // underlying transport blocks, or SSH_ERR_ERROR.
  ssize_t writeData(const void* data, size_t len);

  // Receives data into |data| with length |len|. This function
  // returns the number of bytes received if it succeeds, or
  // SSH_ERR_WOULDBLOCK if the underlying transport blocks, or
  // SSH_ERR_ERROR.
  ssize_t readData(void* data, size_t len);

  // Performs handshake. This function returns SSH_ERR_OK
  // if it succeeds, or SSH_ERR_WOULDBLOCK if the underlying transport
  // blocks, or SSH_ERR_ERROR.
  int handshake();

  // Returns message digest of host's public key.  |hashType| must be
  // either "sha-1" or "md5".
  std::string hostkeyMessageDigest(const std::string& hashType);

  // Performs authentication using username and password.  This
  // function returns SSH_ERR_OK if it succeeds, or SSH_ERR_WOULDBLOCK
  // if the underlying transport blocks, or SSH_ERR_ERROR.
  int authPassword(const std::string& user, const std::string& password);

  // Starts SFTP session and opens remote file |path|.  This function
  // returns SSH_ERR_OK if it succeeds, or SSH_ERR_WOULDBLOCK if the
  // underlying transport blocks, or SSH_ERR_ERROR.
  int sftpOpen(const std::string& path);

  // Closes remote file opened by sftpOpen().  This function returns
  // SSH_ERR_OK if it succeeds, or SSH_ERR_WOULDBLOCK if the
  // underlying transport blocks, or SSH_ERR_ERROR.
  int sftpClose();

  // Gets total length and modified time of opened file by sftpOpen().
  // On success, total length and modified time are assigned to
  // |totalLength| and |mtime|.  This function returns SSH_ERR_OK if
  // it succeeds, or SSH_ERR_WOULDBLOCK if the underlying transport
  // blocks, or SSH_ERR_ERROR.
  int sftpStat(int64_t& totalLength, time_t& mtime);

  // Moves file position to |pos|.
  void sftpSeek(int64_t pos);

  // Returns last error string
  std::string getLastErrorString();

private:
  LIBSSH2_SESSION* ssh2_;
  LIBSSH2_SFTP* sftp_;
  LIBSSH2_SFTP_HANDLE* sftph_;
  sock_t fd_;
};
} // namespace aria2

#endif // SSH_SESSION_H
