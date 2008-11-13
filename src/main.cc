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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
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
#include "DefaultBtContext.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "ConsoleStatCalc.h"
#include "NullStatCalc.h"
#include "download_helper.h"
#include "Exception.h"
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

SharedHandle<StatCalc> getStatCalc(const Option* op)
{
  SharedHandle<StatCalc> statCalc;
  if(op->getAsBool(PREF_QUIET)) {
    statCalc.reset(new NullStatCalc());
  } else {
    statCalc.reset(new ConsoleStatCalc(op->getAsInt(PREF_SUMMARY_INTERVAL)));
  }
  return statCalc;
}

std::ostream& getSummaryOut(const Option* op)
{
  if(op->getAsBool(PREF_QUIET)) {
    return nullout;
  } else {
    return std::cout;
  }
}

extern Option* option_processing(int argc, char* const argv[]);

int main(int argc, char* argv[])
{
  Option* op = option_processing(argc, argv);
  std::deque<std::string> args(argv+optind, argv+argc);

  SimpleRandomizer::init();
  BitfieldManFactory::setDefaultRandomizer(SimpleRandomizer::getInstance());
  if(op->get(PREF_LOG) == "-") {
    LogFactory::setLogFile(DEV_STDOUT);
  } else if(!op->get(PREF_LOG).empty()) {
    LogFactory::setLogFile(op->get(PREF_LOG));
  } else {
    LogFactory::setLogFile(DEV_NULL);
  }
  LogFactory::setLogLevel(op->get(PREF_LOG_LEVEL));
  if(op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
  int32_t exitStatus = EXIT_SUCCESS;
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

#ifdef SIGPIPE
    Util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);
#endif
    int32_t returnValue = 0;
    std::deque<SharedHandle<RequestGroup> > requestGroups;
#ifdef ENABLE_BITTORRENT
    if(op->defined(PREF_TORRENT_FILE)) {
      if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	DefaultBtContextHandle btContext(new DefaultBtContext());
	btContext->load(op->get(PREF_TORRENT_FILE));
	std::cout << btContext << std::endl;
      } else {
	createRequestGroupForBitTorrent(requestGroups, op, args);
      }
    }
    else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if(op->defined(PREF_METALINK_FILE)) {
	if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	  std::deque<SharedHandle<MetalinkEntry> > metalinkEntries;
	  MetalinkHelper::parseAndQuery(metalinkEntries,
					op->get(PREF_METALINK_FILE), op);
	  std::deque<SharedHandle<FileEntry> > fileEntries;
	  MetalinkEntry::toFileEntry(fileEntries, metalinkEntries);
	  Util::toStream(std::cout, fileEntries);
	} else {
	  createRequestGroupForMetalink(requestGroups, op);
	}
      }
      else
#endif // ENABLE_METALINK
	if(op->defined(PREF_INPUT_FILE)) {
	  createRequestGroupForUriList(requestGroups, op);
	} else {
	  createRequestGroupForUri(requestGroups, op, args);
	}

    returnValue = MultiUrlRequestInfo(requestGroups, op, getStatCalc(op),
				      getSummaryOut(op)).execute();
    if(returnValue == 1) {
      exitStatus = EXIT_FAILURE;
    }
  } catch(Exception& ex) {
    std::cerr << EX_EXCEPTION_CAUGHT << "\n" << ex.stackTrace() << std::endl;
    exitStatus = EXIT_FAILURE;
  }
  delete op;
  LogFactory::release();
  return exitStatus;
}

} // namespace aria2

int main(int argc, char* argv[]) {
  aria2::Platform platform;

  int r = aria2::main(argc, argv);

  return r;
}
