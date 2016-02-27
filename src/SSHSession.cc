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
#include "SSHSession.h"

#include <cassert>

#include "MessageDigest.h"

namespace aria2 {

SSHSession::SSHSession()
    : ssh2_(nullptr), sftp_(nullptr), sftph_(nullptr), fd_(-1)
{
}

SSHSession::~SSHSession() { closeConnection(); }

int SSHSession::closeConnection()
{
  if (sftph_) {
    // TODO this could return LIBSSH2_ERROR_EAGAIN
    libssh2_sftp_close(sftph_);
    sftph_ = nullptr;
  }
  if (sftp_) {
    // TODO this could return LIBSSH2_ERROR_EAGAIN
    libssh2_sftp_shutdown(sftp_);
    sftp_ = nullptr;
  }
  if (ssh2_) {
    // TODO this could return LIBSSH2_ERROR_EAGAIN
    libssh2_session_disconnect(ssh2_, "bye");
    libssh2_session_free(ssh2_);
    ssh2_ = nullptr;
  }
  return SSH_ERR_OK;
}

int SSHSession::gracefulShutdown()
{
  if (sftph_) {
    auto rv = libssh2_sftp_close(sftph_);
    if (rv == LIBSSH2_ERROR_EAGAIN) {
      return SSH_ERR_WOULDBLOCK;
    }
    if (rv != 0) {
      return SSH_ERR_ERROR;
    }
    sftph_ = nullptr;
  }
  if (sftp_) {
    auto rv = libssh2_sftp_shutdown(sftp_);
    if (rv == LIBSSH2_ERROR_EAGAIN) {
      return SSH_ERR_WOULDBLOCK;
    }
    if (rv != 0) {
      return SSH_ERR_ERROR;
    }
    sftp_ = nullptr;
  }
  if (ssh2_) {
    auto rv = libssh2_session_disconnect(ssh2_, "bye");
    if (rv == LIBSSH2_ERROR_EAGAIN) {
      return SSH_ERR_WOULDBLOCK;
    }
    if (rv != 0) {
      return SSH_ERR_ERROR;
    }
    libssh2_session_free(ssh2_);
    ssh2_ = nullptr;
  }
  return SSH_ERR_OK;
}

int SSHSession::sftpClose()
{
  if (!sftph_) {
    return SSH_ERR_OK;
  }

  auto rv = libssh2_sftp_close(sftph_);
  if (rv == LIBSSH2_ERROR_EAGAIN) {
    return SSH_ERR_WOULDBLOCK;
  }
  if (rv != 0) {
    return SSH_ERR_ERROR;
  }
  sftph_ = nullptr;
  return SSH_ERR_OK;
}

int SSHSession::init(sock_t sockfd)
{
  ssh2_ = libssh2_session_init();
  if (!ssh2_) {
    return SSH_ERR_ERROR;
  }
  libssh2_session_set_blocking(ssh2_, 0);
  fd_ = sockfd;
  return SSH_ERR_OK;
}

int SSHSession::checkDirection()
{
  auto dir = libssh2_session_block_directions(ssh2_);
  if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) {
    return SSH_WANT_WRITE;
  }
  return SSH_WANT_READ;
}

ssize_t SSHSession::writeData(const void* data, size_t len)
{
  // net implemented yet
  assert(0);
}

ssize_t SSHSession::readData(void* data, size_t len)
{
  auto nread = libssh2_sftp_read(sftph_, static_cast<char*>(data), len);
  if (nread == LIBSSH2_ERROR_EAGAIN) {
    return SSH_ERR_WOULDBLOCK;
  }
  if (nread < 0) {
    return SSH_ERR_ERROR;
  }
  return nread;
}

int SSHSession::handshake()
{
  auto rv = libssh2_session_handshake(ssh2_, fd_);
  if (rv == LIBSSH2_ERROR_EAGAIN) {
    return SSH_ERR_WOULDBLOCK;
  }
  if (rv != 0) {
    return SSH_ERR_ERROR;
  }
  return SSH_ERR_OK;
}

std::string SSHSession::hostkeyMessageDigest(const std::string& hashType)
{
  int h;
  if (hashType == "sha-1") {
    h = LIBSSH2_HOSTKEY_HASH_SHA1;
  }
  else if (hashType == "md5") {
    h = LIBSSH2_HOSTKEY_HASH_MD5;
  }
  else {
    return "";
  }
  auto fingerprint = libssh2_hostkey_hash(ssh2_, h);
  if (!fingerprint) {
    return "";
  }
  return std::string(fingerprint, MessageDigest::getDigestLength(hashType));
}

int SSHSession::authPassword(const std::string& user,
                             const std::string& password)
{
  auto rv = libssh2_userauth_password(ssh2_, user.c_str(), password.c_str());
  if (rv == LIBSSH2_ERROR_EAGAIN) {
    return SSH_ERR_WOULDBLOCK;
  }
  if (rv != 0) {
    return SSH_ERR_ERROR;
  }
  return SSH_ERR_OK;
}

int SSHSession::sftpOpen(const std::string& path)
{
  if (!sftp_) {
    sftp_ = libssh2_sftp_init(ssh2_);
    if (!sftp_) {
      if (libssh2_session_last_errno(ssh2_) == LIBSSH2_ERROR_EAGAIN) {
        return SSH_ERR_WOULDBLOCK;
      }
      return SSH_ERR_ERROR;
    }
  }
  if (!sftph_) {
    sftph_ = libssh2_sftp_open(sftp_, path.c_str(), LIBSSH2_FXF_READ, 0);
    if (!sftph_) {
      if (libssh2_session_last_errno(ssh2_) == LIBSSH2_ERROR_EAGAIN) {
        return SSH_ERR_WOULDBLOCK;
      }
      return SSH_ERR_ERROR;
    }
  }
  return SSH_ERR_OK;
}

int SSHSession::sftpStat(int64_t& totalLength, time_t& mtime)
{
  LIBSSH2_SFTP_ATTRIBUTES attrs;
  auto rv = libssh2_sftp_fstat_ex(sftph_, &attrs, 0);
  if (rv == LIBSSH2_ERROR_EAGAIN) {
    return SSH_ERR_WOULDBLOCK;
  }
  if (rv != 0) {
    return SSH_ERR_ERROR;
  }
  totalLength = attrs.filesize;
  mtime = attrs.mtime;
  return SSH_ERR_OK;
}

void SSHSession::sftpSeek(int64_t pos) { libssh2_sftp_seek64(sftph_, pos); }

std::string SSHSession::getLastErrorString()
{
  if (!ssh2_) {
    return "SSH session has not been initialized yet";
  }
  char* errmsg;
  libssh2_session_last_error(ssh2_, &errmsg, nullptr, 0);
  return errmsg;
}

} // namespace aria2
