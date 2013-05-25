/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "FeatureConfig.h"

#ifdef HAVE_ZLIB
# include <zlib.h>
#endif // HAVE_ZLIB
#ifdef HAVE_LIBXML2
# include <libxml/xmlversion.h>
#endif // HAVE_LIBXML2
#ifdef HAVE_LIBEXPAT
# include <expat.h>
#endif // HAVE_LIBEXPAT
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif // HAVE_SQLITE3
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS
#ifdef HAVE_OPENSSL
# include <openssl/opensslv.h>
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBGMP
# include <gmp.h>
#endif // HAVE_LIBGMP
#ifdef HAVE_LIBGCRYPT
# include <gcrypt.h>
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBCARES
# include <ares.h>
#endif // HAVE_LIBCARES

#include "util.h"

namespace aria2 {

uint16_t getDefaultPort(const std::string& protocol)
{
  if(protocol == "http") {
    return 80;
  } else if(protocol == "https") {
    return 443;
  } else if(protocol == "ftp") {
    return 21;
  } else {
    return 0;
  }
}

std::string featureSummary()
{
  std::string s;
  int first;
  for(first = 0; first < MAX_FEATURE && !strSupportedFeature(first); ++first);
  if(first < MAX_FEATURE) {
    s += strSupportedFeature(first);
    for(int i = first+1; i < MAX_FEATURE; ++i) {
      const char* name = strSupportedFeature(i);
      if(name) {
        s += ", ";
        s += name;
      }
    }
  }
  return s;
}

const char* strSupportedFeature(int feature)
{
  switch(feature) {
  case(FEATURE_ASYNC_DNS):
#ifdef ENABLE_ASYNC_DNS
    return "Async DNS";
#else // !ENABLE_ASYNC_DNS
    return 0;
#endif // !ENABLE_ASYNC_DNS
    break;

  case(FEATURE_BITTORRENT):
#ifdef ENABLE_BITTORRENT
    return "BitTorrent";
#else // !ENABLE_BITTORRENT
    return 0;
#endif // !ENABLE_BITTORRENT
    break;

  case(FEATURE_FF3_COOKIE):
#ifdef HAVE_SQLITE3
    return "Firefox3 Cookie";
#else // !HAVE_SQLITE3
    return 0;
#endif // !HAVE_SQLITE3
    break;

  case(FEATURE_GZIP):
#ifdef HAVE_ZLIB
    return "GZip";
#else // !HAVE_ZLIB
    return 0;
#endif // !HAVE_ZLIB
    break;

  case(FEATURE_HTTPS):
#ifdef ENABLE_SSL
    return "HTTPS";
#else // !ENABLE_SSL
    return 0;
#endif // !ENABLE_SSL
    break;

  case(FEATURE_MESSAGE_DIGEST):
#ifdef ENABLE_MESSAGE_DIGEST
    return "Message Digest";
#else // !ENABLE_MESSAGE_DIGEST
    return 0;
#endif // !ENABLE_MESSAGE_DIGEST
    break;

  case(FEATURE_METALINK):
#ifdef ENABLE_METALINK
    return "Metalink";
#else // !ENABLE_METALINK
    return 0;
#endif // !ENABLE_METALINK
    break;

  case(FEATURE_XML_RPC):
#ifdef ENABLE_XML_RPC
    return "XML-RPC";
#else // !ENABLE_XML_RPC
    return 0;
#endif // !ENABLE_XML_RPC
    break;

  default:
    return 0;
  }
}

std::string usedLibs()
{
  std::string res;
#ifdef HAVE_ZLIB
  res += "zlib/"ZLIB_VERSION" ";
#endif // HAVE_ZLIB
#ifdef HAVE_LIBXML2
  res += "libxml2/"LIBXML_DOTTED_VERSION" ";
#endif // HAVE_LIBXML2
#ifdef HAVE_LIBEXPAT
  res += fmt("expat/%d.%d.%d ",
             XML_MAJOR_VERSION, XML_MINOR_VERSION, XML_MICRO_VERSION);
#endif // HAVE_LIBEXPAT
#ifdef HAVE_SQLITE3
  res += "sqlite3/"SQLITE_VERSION" ";
#endif // HAVE_SQLITE3
#ifdef HAVE_APPLETLS
  res += "appleTLS ";
#endif // HAVE_APPLETLS
#ifdef HAVE_LIBGNUTLS
  res += "GnuTLS/"GNUTLS_VERSION" ";
#endif // HAVE_LIBGNUTLS
#ifdef HAVE_OPENSSL
  res += fmt("OpenSSL/%ld.%ld.%ld",
             OPENSSL_VERSION_NUMBER >> 28,
             (OPENSSL_VERSION_NUMBER >> 20) & 0xff,
             (OPENSSL_VERSION_NUMBER >> 12) & 0xff);
  if((OPENSSL_VERSION_NUMBER >> 4) & 0xff) {
    res += 'a' + ((OPENSSL_VERSION_NUMBER >> 4) & 0xff) - 1;
  }
  res += " ";
#endif // HAVE_OPENSSL
#ifdef HAVE_LIBNETTLE
  // No library version in header files.
  res += "nettle ";
#endif // HAVE_LIBNETTLE
#ifdef HAVE_LIBGMP
  res += fmt("GMP/%d.%d.%d ",
             __GNU_MP_VERSION, __GNU_MP_VERSION_MINOR,
             __GNU_MP_VERSION_PATCHLEVEL);
#endif // HAVE_LIBGMP
#ifdef HAVE_LIBGCRYPT
  res += "libgcrypt/"GCRYPT_VERSION" ";
#endif // HAVE_LIBGCRYPT
#ifdef HAVE_LIBCARES
  res += "c-ares/"ARES_VERSION_STR" ";
#endif // HAVE_LIBCARES
  if(!res.empty()) {
    res.erase(res.length()-1);
  }
  return res;
}

} // namespace aria2
