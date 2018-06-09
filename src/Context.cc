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
#include "Context.h"

#include <unistd.h>
#include <getopt.h>

#ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif // HAVE_SYS_RESOURCE_H

#include <numeric>
#include <vector>
#include <iostream>

#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "FeatureConfig.h"
#include "MultiUrlRequestInfo.h"
#include "SimpleRandomizer.h"
#include "File.h"
#include "message.h"
#include "prefs.h"
#include "Option.h"
#include "a2algo.h"
#include "a2io.h"
#include "a2time.h"
#include "Platform.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "Exception.h"
#include "ProtocolDetector.h"
#include "RecoverableException.h"
#include "SocketCore.h"
#include "DownloadContext.h"
#include "fmt.h"
#include "console.h"
#include "UriListParser.h"
#include "message_digest_helper.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
#  include "metalink_helper.h"
#  include "MetalinkEntry.h"
#endif // ENABLE_METALINK

extern char* optarg;
extern int optind, opterr, optopt;

namespace aria2 {

#ifdef ENABLE_BITTORRENT
namespace {
void showTorrentFile(const std::string& uri)
{
  auto op = std::make_shared<Option>();
  auto dctx = std::make_shared<DownloadContext>();
  bittorrent::load(uri, dctx, op);
  bittorrent::print(*global::cout(), dctx);
}
} // namespace
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
namespace {
void showMetalinkFile(const std::string& uri, const std::shared_ptr<Option>& op)
{
  auto fileEntries = MetalinkEntry::toFileEntry(
      metalink::parseAndQuery(uri, op.get(), op->get(PREF_METALINK_BASE_URI)));
  util::toStream(std::begin(fileEntries), std::end(fileEntries),
                 *global::cout());
  global::cout()->write("\n");
  global::cout()->flush();
}
} // namespace
#endif // ENABLE_METALINK

#if defined(ENABLE_BITTORRENT) || defined(ENABLE_METALINK)
namespace {
void showFiles(const std::vector<std::string>& uris,
               const std::shared_ptr<Option>& op)
{
  ProtocolDetector dt;
  for (const auto& uri : uris) {
    printf(">>> ");
    printf(MSG_SHOW_FILES, (uri).c_str());
    printf("\n");
    try {
#  ifdef ENABLE_BITTORRENT
      if (dt.guessTorrentFile(uri)) {
        showTorrentFile(uri);
      }
      else
#  endif // ENABLE_BITTORRENT
#  ifdef ENABLE_METALINK
          if (dt.guessMetalinkFile(uri)) {
        showMetalinkFile(uri, op);
      }
      else
#  endif // ENABLE_METALINK
      {
        printf("%s\n\n", MSG_NOT_TORRENT_METALINK);
      }
    }
    catch (RecoverableException& e) {
      global::cout()->printf("%s\n", e.stackTrace().c_str());
    }
  }
}
} // namespace
#endif // ENABLE_BITTORRENT || ENABLE_METALINK

extern error_code::Value option_processing(Option& option, bool standalone,
                                           std::vector<std::string>& uris,
                                           int argc, char** argv,
                                           const KeyVals& options);

Context::Context(bool standalone, int argc, char** argv, const KeyVals& options)
{
  std::vector<std::string> args;
  auto op = std::make_shared<Option>();
  error_code::Value rv;
  rv = option_processing(*op.get(), standalone, args, argc, argv, options);
  if (rv != error_code::FINISHED) {
    if (standalone) {
      exit(rv);
    }
    else {
      throw DL_ABORT_EX("Option processing failed");
    }
  }
#ifdef ENABLE_BITTORRENT
  bittorrent::generateStaticPeerId(op->get(PREF_PEER_ID_PREFIX));
  bittorrent::generateStaticPeerAgent(op->get(PREF_PEER_AGENT));
#endif // ENABLE_BITTORRENT
  LogFactory::setLogFile(op->get(PREF_LOG));
  LogFactory::setLogLevel(op->get(PREF_LOG_LEVEL));
  LogFactory::setConsoleLogLevel(op->get(PREF_CONSOLE_LOG_LEVEL));
  LogFactory::setColorOutput(op->getAsBool(PREF_ENABLE_COLOR));
  if (op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
  LogFactory::reconfigure();
  A2_LOG_INFO("<<--- --- --- ---");
  A2_LOG_INFO("  --- --- --- ---");
  A2_LOG_INFO("  --- --- --- --->>");
  A2_LOG_INFO(fmt("%s %s", PACKAGE, PACKAGE_VERSION));
  A2_LOG_INFO(usedCompilerAndPlatform());
  A2_LOG_INFO(getOperatingSystemInfo());
  A2_LOG_INFO(usedLibs());
  A2_LOG_INFO(MSG_LOGGING_STARTED);

#if defined(HAVE_SYS_RESOURCE_H) && defined(RLIMIT_NOFILE)
  rlimit r = {0, 0};
  if (getrlimit(RLIMIT_NOFILE, &r) >= 0 && r.rlim_cur != RLIM_INFINITY) {
    // Thanks portability, for making it easy :p
    auto rlim_new = r.rlim_cur; // So we get the right type for free.
    if (r.rlim_cur != RLIM_INFINITY) {
      rlim_new = op->getAsInt(PREF_RLIMIT_NOFILE);
      rlim_new = std::max(r.rlim_cur, rlim_new);
      if (r.rlim_max != RLIM_INFINITY) {
        rlim_new = std::min(r.rlim_max, rlim_new);
      }
    }
    if (rlim_new != r.rlim_cur) {
      if (setrlimit(RLIMIT_NOFILE, &r) != 0) {
        int errNum = errno;
        A2_LOG_WARN(fmt("Failed to set rlimit NO_FILE from %" PRIu64 " to "
                        "%" PRIu64 ": %s",
                        (uint64_t)r.rlim_cur, (uint64_t)rlim_new,
                        util::safeStrerror(errNum).c_str()));
      }
      else {
        A2_LOG_DEBUG(fmt("Set rlimit NO_FILE from %" PRIu64 " to %" PRIu64,
                         (uint64_t)r.rlim_cur, (uint64_t)rlim_new));
      }
    }
    else {
      rlim_new = op->getAsInt(PREF_RLIMIT_NOFILE);
      A2_LOG_DEBUG(fmt("Not setting rlimit NO_FILE: %" PRIu64 " >= %" PRIu64,
                       (uint64_t)r.rlim_cur, (uint64_t)rlim_new));
    }
  }
#endif // defined(HAVE_SYS_RESOURCE_H) && defined(RLIMIT_NOFILE)

  if (op->getAsBool(PREF_DISABLE_IPV6)) {
    SocketCore::setProtocolFamily(AF_INET);
  }
  SocketCore::setIpDscp(op->getAsInt(PREF_DSCP));
  SocketCore::setSocketRecvBufferSize(
      op->getAsInt(PREF_SOCKET_RECV_BUFFER_SIZE));
  net::checkAddrconfig();

  if (!net::getIPv4AddrConfigured() && !net::getIPv6AddrConfigured()) {
    // Get rid of AI_ADDRCONFIG. It causes name resolution error when
    // none of network interface has IPv4/v6 address.
    setDefaultAIFlags(0);
  }

  // Bind interface
  if (!op->get(PREF_INTERFACE).empty()) {
    std::string iface = op->get(PREF_INTERFACE);
    SocketCore::bindAddress(iface);
  }
  // Bind multiple interfaces
  if (!op->get(PREF_MULTIPLE_INTERFACE).empty() &&
      op->get(PREF_INTERFACE).empty()) {
    std::string ifaces = op->get(PREF_MULTIPLE_INTERFACE);
    SocketCore::bindAllAddress(ifaces);
  }
  std::vector<std::shared_ptr<RequestGroup>> requestGroups;
  std::shared_ptr<UriListParser> uriListParser;
#ifdef ENABLE_BITTORRENT
  if (!op->blank(PREF_TORRENT_FILE)) {
    if (op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
      showTorrentFile(op->get(PREF_TORRENT_FILE));
      return;
    }
    else {
      createRequestGroupForBitTorrent(requestGroups, op, args,
                                      op->get(PREF_TORRENT_FILE));
    }
  }
  else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if (!op->blank(PREF_METALINK_FILE)) {
    if (op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
      showMetalinkFile(op->get(PREF_METALINK_FILE), op);
      return;
    }
    else {
      createRequestGroupForMetalink(requestGroups, op);
    }
  }
  else
#endif // ENABLE_METALINK
      if (!op->blank(PREF_INPUT_FILE)) {
    if (op->getAsBool(PREF_DEFERRED_INPUT)) {
      uriListParser = openUriListParser(op->get(PREF_INPUT_FILE));
    }
    else {
      createRequestGroupForUriList(requestGroups, op);
    }
#if defined(ENABLE_BITTORRENT) || defined(ENABLE_METALINK)
  }
  else if (op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
    showFiles(args, op);
    return;
#endif // ENABLE_METALINK || ENABLE_METALINK
  }
  else {
    createRequestGroupForUri(requestGroups, op, args, false, false, true);
  }

  // Remove option values which is only valid for URIs specified in
  // command-line. If they are left, because op is used as a template
  // for new RequestGroup(such as created in RPC command), they causes
  // unintentional effect.
  op->remove(PREF_OUT);
  op->remove(PREF_FORCE_SEQUENTIAL);
  op->remove(PREF_INPUT_FILE);
  op->remove(PREF_INDEX_OUT);
  op->remove(PREF_SELECT_FILE);
  op->remove(PREF_PAUSE);
  op->remove(PREF_CHECKSUM);
  op->remove(PREF_GID);

  if (standalone && !op->getAsBool(PREF_ENABLE_RPC) && requestGroups.empty() &&
      !uriListParser) {
    global::cout()->printf("%s\n", MSG_NO_FILES_TO_DOWNLOAD);
  }
  else {
    if (!requestGroups.empty()) {
      A2_LOG_NOTICE(fmt("Downloading %" PRId64 " item(s)",
                        static_cast<uint64_t>(requestGroups.size())));
    }
    reqinfo = std::make_shared<MultiUrlRequestInfo>(std::move(requestGroups),
                                                    op, uriListParser);
  }
}

Context::~Context() = default;

} // namespace aria2
