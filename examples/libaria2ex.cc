/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
//
// Compile and link like this:
// $ g++ -Wall -O2 -g -std=c++11 -o libaria2ex libaria2ex.cc -laria2
#include <iostream>
#include <chrono>

#include <aria2/aria2.h>

int main()
{
  aria2::libraryInit();
  // session is actually singleton: 1 session per process
  aria2::Session* session;
  // Use default configuration
  aria2::SessionConfig config;
  session = aria2::sessionNew(aria2::KeyVals(), config);
  std::vector<std::string> uris = {
    "http://localhost/"
  };
  // Download files are stored in the current directory
  aria2::KeyVals options = { std::make_pair("dir", ".") };
  // Add URI
  aria2::A2Gid gid;
  aria2::addUri(session, gid, uris, options);
  auto start = std::chrono::steady_clock::now();
  for(;;) {
    int rv = aria2::run(session, aria2::RUN_ONCE);
    if(rv != 1) {
      break;
    }
    // the application can call aria2 API to add URI or query progress
    // here
    auto now = std::chrono::steady_clock::now();
    auto count = std::chrono::duration_cast<std::chrono::milliseconds>
      (now - start).count();
    if(count >= 500) {
      start = now;
      std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(session);
      std::cout << "== Active Download(s) ==" << std::endl;
      for(auto gid : gids) {
        aria2::DownloadHandle* dh = aria2::getDownloadHandle(session, gid);
        if(dh) {
          std::cout << "[" << aria2::gidToHex(gid) << "] "
                    << dh->getCompletedLength() << "/"
                    << dh->getTotalLength() << " D:"
                    << dh->getDownloadSpeed()/1024 << "KiB/s, U:"
                    << dh->getUploadSpeed()/1024 << "KiB/s"
                    << std::endl;
          aria2::deleteDownloadHandle(dh);
        }
      }
    }
  }
  int rv = aria2::sessionFinal(session);
  aria2::libraryDeinit();
  return rv;
}
