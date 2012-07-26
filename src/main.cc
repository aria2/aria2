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
#include "common.h"

#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include <numeric>
#include <vector>
#include <iostream>

#include "SharedHandle.h"
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
#include "ConsoleStatCalc.h"
#include "NullStatCalc.h"
#include "download_helper.h"
#include "Exception.h"
#include "ProtocolDetector.h"
#include "RecoverableException.h"
#include "SocketCore.h"
#include "DownloadContext.h"
#include "fmt.h"
#include "NullOutputFile.h"
#include "console.h"
#include "UriListParser.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
# include "metalink_helper.h"
# include "MetalinkEntry.h"
#endif // ENABLE_METALINK
#ifdef ENABLE_MESSAGE_DIGEST
# include "message_digest_helper.h"
#endif // ENABLE_MESSAGE_DIGEST

extern char* optarg;
extern int optind, opterr, optopt;

namespace aria2 {

SharedHandle<StatCalc> getStatCalc(const SharedHandle<Option>& op)
{
  SharedHandle<StatCalc> statCalc;
  if(op->getAsBool(PREF_QUIET)) {
    statCalc.reset(new NullStatCalc());
  } else {
    SharedHandle<ConsoleStatCalc> impl
      (new ConsoleStatCalc(op->getAsInt(PREF_SUMMARY_INTERVAL),
                           op->getAsBool(PREF_HUMAN_READABLE)));
    impl->setReadoutVisibility(op->getAsBool(PREF_SHOW_CONSOLE_READOUT));
    impl->setTruncate(op->getAsBool(PREF_TRUNCATE_CONSOLE_READOUT));
    statCalc = impl;
  }
  return statCalc;
}

SharedHandle<OutputFile> getSummaryOut(const SharedHandle<Option>& op)
{
  if(op->getAsBool(PREF_QUIET)) {
    return SharedHandle<OutputFile>(new NullOutputFile());
  } else {
    return global::cout();
  }
}

#ifdef ENABLE_BITTORRENT
namespace {
void showTorrentFile(const std::string& uri)
{
  SharedHandle<Option> op(new Option());
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  bittorrent::load(uri, dctx, op);
  bittorrent::print(*global::cout(), dctx);
}
} // namespace
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
namespace {
void showMetalinkFile
(const std::string& uri, const SharedHandle<Option>& op)
{
  std::vector<SharedHandle<MetalinkEntry> > metalinkEntries;
  metalink::parseAndQuery(metalinkEntries, uri, op.get(),
                          op->get(PREF_METALINK_BASE_URI));
  std::vector<SharedHandle<FileEntry> > fileEntries;
  MetalinkEntry::toFileEntry(fileEntries, metalinkEntries);
  util::toStream(fileEntries.begin(), fileEntries.end(), *global::cout());
  global::cout()->write("\n");
  global::cout()->flush();
}
} // namespace
#endif // ENABLE_METALINK

#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
namespace {
void showFiles
(const std::vector<std::string>& uris, const SharedHandle<Option>& op)
{
  ProtocolDetector dt;
  for(std::vector<std::string>::const_iterator i = uris.begin(),
        eoi = uris.end(); i != eoi; ++i) {
    printf(">>> ");
    printf(MSG_SHOW_FILES, (*i).c_str());
    printf("\n");
    try {
#ifdef ENABLE_BITTORRENT
      if(dt.guessTorrentFile(*i)) {
        showTorrentFile(*i);
      } else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
        if(dt.guessMetalinkFile(*i)) {
          showMetalinkFile(*i, op);
        } else
#endif // ENABLE_METALINK
          {
            printf(MSG_NOT_TORRENT_METALINK);
            printf("\n\n");
          }
    } catch(RecoverableException& e) {
      global::cout()->printf("%s\n", e.stackTrace().c_str());
    }
  }
}
} // namespace
#endif // ENABLE_BITTORRENT || ENABLE_METALINK

extern void option_processing(Option& option, std::vector<std::string>& uris,
                              int argc, char* argv[]);

error_code::Value main(int argc, char* argv[])
{
  std::vector<std::string> args;
  SharedHandle<Option> op(new Option());
  option_processing(*op.get(), args, argc, argv);

  SimpleRandomizer::init();
#ifdef ENABLE_BITTORRENT
  bittorrent::generateStaticPeerId(op->get(PREF_PEER_ID_PREFIX));
#endif // ENABLE_BITTORRENT
  LogFactory::setLogFile(op->get(PREF_LOG));
  LogFactory::setLogLevel(op->get(PREF_LOG_LEVEL));
  if(op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
  LogFactory::reconfigure();
  error_code::Value exitStatus = error_code::FINISHED;
  A2_LOG_INFO("<<--- --- --- ---");
  A2_LOG_INFO("  --- --- --- ---");
  A2_LOG_INFO("  --- --- --- --->>");
  A2_LOG_INFO(fmt("%s %s %s", PACKAGE, PACKAGE_VERSION, TARGET));
  A2_LOG_INFO(MSG_LOGGING_STARTED);

#ifdef ENABLE_MESSAGE_DIGEST
  message_digest::staticSHA1DigestInit();
#endif // ENABLE_MESSAGE_DIGEST

  if(op->getAsBool(PREF_DISABLE_IPV6)) {
    SocketCore::setProtocolFamily(AF_INET);
    // Get rid of AI_ADDRCONFIG. It causes name resolution error
    // when none of network interface has IPv4 address.
    setDefaultAIFlags(0);
  }
  // Bind interface
  if(!op->get(PREF_INTERFACE).empty()) {
    std::string iface = op->get(PREF_INTERFACE);
    SocketCore::bindAddress(iface);
  }

#ifdef SIGPIPE
  util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);
#endif
#ifdef SIGCHLD
  // Avoid to create zombie process when forked child processes are
  // died.
  util::setGlobalSignalHandler(SIGCHLD, SIG_IGN, 0);
#endif // SIGCHILD
  std::vector<SharedHandle<RequestGroup> > requestGroups;
  SharedHandle<UriListParser> uriListParser;
#ifdef ENABLE_BITTORRENT
  if(!op->blank(PREF_TORRENT_FILE)) {
    if(op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
      showTorrentFile(op->get(PREF_TORRENT_FILE));
      return exitStatus;
    } else {
      createRequestGroupForBitTorrent(requestGroups, op, args,
                                      op->get(PREF_TORRENT_FILE));
    }
  }
  else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    if(!op->blank(PREF_METALINK_FILE)) {
      if(op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
        showMetalinkFile(op->get(PREF_METALINK_FILE), op);
        return exitStatus;
      } else {
        createRequestGroupForMetalink(requestGroups, op);
      }
    }
    else
#endif // ENABLE_METALINK
      if(!op->blank(PREF_INPUT_FILE)) {
        if(op->getAsBool(PREF_DEFERRED_INPUT)) {
          uriListParser = openUriListParser(op->get(PREF_INPUT_FILE));
        } else {
          createRequestGroupForUriList(requestGroups, op);
        }
#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
      } else if(op->get(PREF_SHOW_FILES) == A2_V_TRUE) {
        showFiles(args, op);
        return exitStatus;
#endif // ENABLE_METALINK || ENABLE_METALINK
      } else {
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
  if(!op->getAsBool(PREF_ENABLE_RPC) && requestGroups.empty() &&
     !uriListParser) {
    global::cout()->printf("%s\n", MSG_NO_FILES_TO_DOWNLOAD);
  } else {
    exitStatus = MultiUrlRequestInfo(requestGroups, op, getStatCalc(op),
                                     getSummaryOut(op),
                                     uriListParser).execute();
  }
  return exitStatus;
}

} // namespace aria2

int main(int argc, char* argv[])
{
  aria2::error_code::Value r;
  try {
    aria2::Platform platform;
    r = aria2::main(argc, argv);
  } catch(aria2::Exception& ex) {
    aria2::global::cerr()->printf("%s\n%s\n",
                                  EX_EXCEPTION_CAUGHT,
                                  ex.stackTrace().c_str());
    r = ex.getErrorCode();
  }
  return r;
}
