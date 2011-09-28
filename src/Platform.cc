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
#include <fcntl.h> /*  _O_BINARY */

#include <locale.h> // For setlocale, LC_*

#include <iostream>

#ifdef HAVE_WINSOCK2_H

#ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x501u
#endif // _WIN32_WINNT
#include <winsock2.h>
#undef ERROR
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif // HAVE_WS2TCPIP_H

#endif // HAVE_WINSOCK2_H

#ifdef HAVE_OPENSSL
# include <openssl/err.h>
# include <openssl/ssl.h>
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGCRYPT
# include <gcrypt.h>
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

#ifdef ENABLE_ASYNC_DNS
# include <ares.h>
#endif // ENABLE_ASYNC_DNS

#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"
#include "console.h"
#ifdef HAVE_LIBGMP
# include "a2gmp.h"
#endif // HAVE_LIBGMP

#define A2_MIN_GCRYPT_VERSION "1.2.4"

namespace aria2 {

bool Platform::initialized_ = false;

Platform::Platform()
{
  setUp();
}

Platform::~Platform()
{
  tearDown();
}

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
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif // ENABLE_NLS

#ifdef HAVE_OPENSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGCRYPT
  if(!gcry_check_version(A2_MIN_GCRYPT_VERSION)) {
    throw DL_ABORT_EX("gcry_check_version() failed.");
  }
  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBGNUTLS
  {
    int r = gnutls_global_init();
    if(r != GNUTLS_E_SUCCESS) {
      throw DL_ABORT_EX(fmt("gnutls_global_init() failed, cause:%s",
                            gnutls_strerror(r)));
    }
  }
#endif // HAVE_LIBGNUTLS

#ifdef CARES_HAVE_ARES_LIBRARY_INIT
  int aresErrorCode;
  if((aresErrorCode = ares_library_init(ARES_LIB_INIT_ALL)) != 0) {
    global::cerr()->printf("ares_library_init() failed:%s\n",
                           ares_strerror(aresErrorCode));
  }
#endif // CARES_HAVE_ARES_LIBRARY_INIT

#ifdef HAVE_WINSOCK2_H
  WSADATA wsaData;
  memset(reinterpret_cast<char*>(&wsaData), 0, sizeof(wsaData));
  if (WSAStartup(MAKEWORD(1, 1), &wsaData)) {
    throw DL_ABORT_EX(MSG_WINSOCK_INIT_FAILD);
  }
#endif // HAVE_WINSOCK2_H

#ifdef __MINGW32__
  unsigned int _CRT_fmode = _O_BINARY;
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

#ifdef HAVE_WINSOCK2_H
  WSACleanup();
#endif // HAVE_WINSOCK2_H
  
  return true;
}

bool Platform::isInitialized()
{
  return initialized_;
}

} // namespace aria2
