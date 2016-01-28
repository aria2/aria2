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
#include "Platform.h"

#include <stdlib.h> /* _fmode */
#include <fcntl.h>  /*  _O_BINARY */

#include <locale.h> // For setlocale, LC_*

#include <iostream>

#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGCRYPT
#include <gcrypt.h>
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBGNUTLS
#include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

#ifdef ENABLE_ASYNC_DNS
#include <ares.h>
#endif // ENABLE_ASYNC_DNS

#ifdef HAVE_LIBSSH2
#include <libssh2.h>
#endif // HAVE_LIBSSH2

#include "a2netcompat.h"
#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"
#include "console.h"
#include "OptionParser.h"
#include "prefs.h"
#ifdef HAVE_LIBGMP
#include "a2gmp.h"
#endif // HAVE_LIBGMP
#include "LogFactory.h"
#include "util.h"

namespace aria2 {

#ifdef HAVE_LIBGNUTLS
namespace {
void gnutls_log_callback(int level, const char* str)
{
  using namespace aria2;
  // GnuTLS adds a newline. Drop it.
  std::string msg(str);
  msg.resize(msg.size() - 1);
  A2_LOG_DEBUG(fmt("GnuTLS: <%d> %s", level, msg.c_str()));
}
}
#endif // HAVE_LIBGNUTLS

bool Platform::initialized_ = false;

Platform::Platform() { setUp(); }

Platform::~Platform() { tearDown(); }

#ifdef __MINGW32__
namespace {
bool gainPrivilege(LPCTSTR privName)
{
  LUID luid;
  TOKEN_PRIVILEGES tp;

  if (!LookupPrivilegeValue(nullptr, privName, &luid)) {
    auto errNum = GetLastError();
    A2_LOG_WARN(fmt("Lookup for privilege name %s failed. cause: %s", privName,
                    util::formatLastError(errNum).c_str()));
    return false;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  HANDLE token;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    auto errNum = GetLastError();
    A2_LOG_WARN(fmt("Getting process token failed. cause: %s",
                    util::formatLastError(errNum).c_str()));
    return false;
  }

  auto tokenCloser = defer(token, CloseHandle);

  if (!AdjustTokenPrivileges(token, FALSE, &tp, 0, NULL, NULL)) {
    auto errNum = GetLastError();
    A2_LOG_WARN(fmt("Gaining privilege %s failed. cause: %s", privName,
                    util::formatLastError(errNum).c_str()));
    return false;
  }

  // Check privilege was really gained
  DWORD bufsize = 0;
  GetTokenInformation(token, TokenPrivileges, nullptr, 0, &bufsize);
  if (bufsize == 0) {
    A2_LOG_WARN("Checking privilege failed.");
    return false;
  }

  auto buf = make_unique<char[]>(bufsize);
  if (!GetTokenInformation(token, TokenPrivileges, buf.get(), bufsize,
                           &bufsize)) {
    auto errNum = GetLastError();
    A2_LOG_WARN(fmt("Checking privilege failed. cause: %s",
                    util::formatLastError(errNum).c_str()));
    return false;
  }

  auto privs = reinterpret_cast<TOKEN_PRIVILEGES*>(buf.get());
  for (size_t i = 0; i < privs->PrivilegeCount; ++i) {
    auto& priv = privs->Privileges[i];
    if (memcmp(&priv.Luid, &luid, sizeof(luid)) != 0) {
      continue;
    }
    if (priv.Attributes == SE_PRIVILEGE_ENABLED) {
      return true;
    }

    break;
  }

  A2_LOG_WARN(fmt("Gaining privilege %s failed.", privName));

  return false;
}
} // namespace
#endif // __MINGW32__

bool Platform::setUp()
{
  if (initialized_) {
    return false;
  }
  initialized_ = true;
#ifdef HAVE_LIBGMP
  global::initGmp();
#endif // HAVE_LIBGMP
#ifdef ENABLE_NLS
  setlocale(LC_CTYPE, "");
  setlocale(LC_MESSAGES, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif // ENABLE_NLS

#ifdef HAVE_OPENSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
  // Need this to "decrypt" p12 files.
  OpenSSL_add_all_algorithms();
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGCRYPT
  if (!gcry_check_version("1.2.4")) {
    throw DL_ABORT_EX("gcry_check_version() failed.");
  }
  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBGNUTLS
  {
    int r = gnutls_global_init();
    if (r != GNUTLS_E_SUCCESS) {
      throw DL_ABORT_EX(
          fmt("gnutls_global_init() failed, cause:%s", gnutls_strerror(r)));
    }

    gnutls_global_set_log_function(gnutls_log_callback);
    gnutls_global_set_log_level(0);
  }
#endif // HAVE_LIBGNUTLS

#ifdef CARES_HAVE_ARES_LIBRARY_INIT
  int aresErrorCode;
  if ((aresErrorCode = ares_library_init(ARES_LIB_INIT_ALL)) != 0) {
    global::cerr()->printf("ares_library_init() failed:%s\n",
                           ares_strerror(aresErrorCode));
  }
#endif // CARES_HAVE_ARES_LIBRARY_INIT

#ifdef HAVE_LIBSSH2
  {
    auto rv = libssh2_init(0);
    if (rv != 0) {
      throw DL_ABORT_EX(fmt("libssh2_init() failed, code: %d", rv));
    }
  }
#endif // HAVE_LIBSSH2

#ifdef HAVE_WINSOCK2_H
  WSADATA wsaData;
  memset(reinterpret_cast<char*>(&wsaData), 0, sizeof(wsaData));
  if (WSAStartup(MAKEWORD(1, 1), &wsaData)) {
    throw DL_ABORT_EX(MSG_WINSOCK_INIT_FAILD);
  }
#endif // HAVE_WINSOCK2_H

#ifdef __MINGW32__
  (void)_setmode(_fileno(stdin), _O_BINARY);
  (void)_setmode(_fileno(stdout), _O_BINARY);
  (void)_setmode(_fileno(stderr), _O_BINARY);

  // Windows build: --file-allocation=falloc uses SetFileValidData
  // which requires SE_MANAGE_VOLUME_NAME privilege.  SetFileValidData
  // has security implications (see
  // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365544%28v=vs.85%29.aspx).
  if (!gainPrivilege(SE_MANAGE_VOLUME_NAME)) {
    A2_LOG_WARN("--file-allocation=falloc will not work properly.");
  }

#endif // __MINGW32__

  return true;
}

bool Platform::tearDown()
{
  if (!initialized_) {
    return false;
  }
  initialized_ = false;

#ifdef HAVE_LIBGNUTLS
  gnutls_global_deinit();
#endif // HAVE_LIBGNUTLS

#ifdef CARES_HAVE_ARES_LIBRARY_CLEANUP
  ares_library_cleanup();
#endif // CARES_HAVE_ARES_LIBRARY_CLEANUP

#ifdef HAVE_LIBSSH2
  libssh2_exit();
#endif // HAVE_LIBSSH2

#ifdef HAVE_WINSOCK2_H
  WSACleanup();
#endif // HAVE_WINSOCK2_H
  // Deletes statically allocated resources. This is done to
  // distinguish memory leak from them. This is handy to use
  // valgrind.
  OptionParser::deleteInstance();
  option::deletePrefResource();
  return true;
}

bool Platform::isInitialized() { return initialized_; }

} // namespace aria2
