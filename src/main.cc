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

#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>

#include "SharedHandle.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "BitfieldManFactory.h"
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
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
# include "MetalinkHelper.h"
# include "MetalinkEntry.h"
#endif // ENABLE_METALINK
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

extern char* optarg;
extern int optind, opterr, optopt;

namespace aria2 {

// output stream to /dev/null
std::ofstream nullout(DEV_NULL);

SharedHandle<StatCalc> getStatCalc(const SharedHandle<Option>& op)
{
  SharedHandle<StatCalc> statCalc;
  if(op->getAsBool(PREF_QUIET)) {
    statCalc.reset(new NullStatCalc());
  } else {
    statCalc.reset(new ConsoleStatCalc(op->getAsInt(PREF_SUMMARY_INTERVAL),
                                       op->getAsBool(PREF_HUMAN_READABLE)));
  }
  return statCalc;
}

std::ostream& getSummaryOut(const SharedHandle<Option>& op)
{
  if(op->getAsBool(PREF_QUIET)) {
    return nullout;
  } else {
    return std::cout;
  }
}

#ifdef ENABLE_BITTORRENT
static void showTorrentFile(const std::string& uri)
{
  SharedHandle<DownloadContext> dctx(new DownloadContext());
  bittorrent::load(uri, dctx);
  bittorrent::print(std::cout, dctx);
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
static void showMetalinkFile
(const std::string& uri, const SharedHandle<Option>& op)
{
  std::deque<SharedHandle<MetalinkEntry> > metalinkEntries;
  MetalinkHelper::parseAndQuery(metalinkEntries, uri, op.get());
  std::deque<SharedHandle<FileEntry> > fileEntries;
  MetalinkEntry::toFileEntry(fileEntries, metalinkEntries);
  util::toStream(fileEntries.begin(), fileEntries.end(), std::cout);
  std::cout << std::endl;
}
#endif // ENABLE_METALINK

#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
static void showFiles
(const std::deque<std::string>& uris, const SharedHandle<Option>& op)
{
  ProtocolDetector dt;
  for(std::deque<std::string>::const_iterator i = uris.begin();
      i != uris.end(); ++i) {
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
      std::cout << e.stackTrace() << std::endl;
    }
  }
}
#endif // ENABLE_BITTORRENT || ENABLE_METALINK

extern void option_processing(Option& option, std::deque<std::string>& uris,
                              int argc, char* const argv[]);

downloadresultcode::RESULT main(int argc, char* argv[])
{
  std::deque<std::string> args;
  SharedHandle<Option> op(new Option());
  option_processing(*op.get(), args, argc, argv);

  SimpleRandomizer::init();
  BitfieldManFactory::setDefaultRandomizer(SimpleRandomizer::getInstance());
#ifdef ENABLE_BITTORRENT
  bittorrent::generateStaticPeerId(op->get(PREF_PEER_ID_PREFIX));
#endif // ENABLE_BITTORRENT
  if(op->get(PREF_LOG) == "-") {
    LogFactory::setLogFile(DEV_STDOUT);
  } else if(op->blank(PREF_LOG)) {
    LogFactory::setLogFile(DEV_NULL);
  } else {
    LogFactory::setLogFile(op->get(PREF_LOG));
  }
  LogFactory::setLogLevel(op->get(PREF_LOG_LEVEL));
  if(op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
#ifdef HAVE_EPOLL
  if(op->get(PREF_EVENT_POLL) == V_EPOLL) {
    SocketCore::useEpoll();
  } else
#endif // HAVE_EPOLL
    if(op->get(PREF_EVENT_POLL) == V_SELECT) {
      SocketCore::useSelect();
    }
  downloadresultcode::RESULT exitStatus = downloadresultcode::FINISHED;
  try {
    Logger* logger = LogFactory::getInstance();
    logger->info("<<--- --- --- ---");
    logger->info("  --- --- --- ---");
    logger->info("  --- --- --- --->>");
    logger->info("%s %s %s", PACKAGE, PACKAGE_VERSION, TARGET);
    logger->info(MSG_LOGGING_STARTED);

#ifdef ENABLE_MESSAGE_DIGEST
    MessageDigestHelper::staticSHA1DigestInit();
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
    std::deque<SharedHandle<RequestGroup> > requestGroups;
#ifdef ENABLE_BITTORRENT
    if(!op->blank(PREF_TORRENT_FILE)) {
      if(op->get(PREF_SHOW_FILES) == V_TRUE) {
        showTorrentFile(op->get(PREF_TORRENT_FILE));
        return exitStatus;
      } else {
        createRequestGroupForBitTorrent(requestGroups, op, args);
      }
    }
    else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if(!op->blank(PREF_METALINK_FILE)) {
        if(op->get(PREF_SHOW_FILES) == V_TRUE) {
          showMetalinkFile(op->get(PREF_METALINK_FILE), op);
          return exitStatus;
        } else {
          createRequestGroupForMetalink(requestGroups, op);
        }
      }
      else
#endif // ENABLE_METALINK
        if(!op->blank(PREF_INPUT_FILE)) {
          createRequestGroupForUriList(requestGroups, op);
#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
        } else if(op->get(PREF_SHOW_FILES) == V_TRUE) {
          showFiles(args, op);
          return exitStatus;
#endif // ENABLE_METALINK || ENABLE_METALINK
        } else {
          createRequestGroupForUri(requestGroups, op, args);
        }

    // Remove option values which is only valid for URIs specified in
    // command-line. If they are left, because op is used as a
    // template for new RequestGroup(such as created in XML-RPC
    // command), they causes unintentional effect.
    op->remove(PREF_OUT);
    op->remove(PREF_FORCE_SEQUENTIAL);
    op->remove(PREF_INPUT_FILE);
    op->remove(PREF_INDEX_OUT);
    op->remove(PREF_SELECT_FILE);
    if(
#ifdef ENABLE_XML_RPC
       !op->getAsBool(PREF_ENABLE_XML_RPC) &&
#endif // ENABLE_XML_RPC
       requestGroups.empty()) {
      std::cout << MSG_NO_FILES_TO_DOWNLOAD << std::endl;
    } else {
      exitStatus = MultiUrlRequestInfo(requestGroups, op, getStatCalc(op),
                                       getSummaryOut(op)).execute();
    }
  } catch(Exception& ex) {
    std::cerr << EX_EXCEPTION_CAUGHT << "\n" << ex.stackTrace() << std::endl;
    exitStatus = downloadresultcode::UNKNOWN_ERROR;
  }
  LogFactory::release();
  return exitStatus;
}

} // namespace aria2

int main(int argc, char* argv[]) {
  aria2::Platform platform;

  aria2::downloadresultcode::RESULT r = aria2::main(argc, argv);

  return r;
}
