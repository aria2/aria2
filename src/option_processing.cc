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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Option.h"
#include "prefs.h"
#include "OptionParser.h"
#include "OptionHandlerFactory.h"
#include "OptionHandler.h"
#include "Util.h"
#include "message.h"
#include "Exception.h"
#include "a2io.h"
#include "help_tags.h"
#include "File.h"
#include "StringFormat.h"
#include "OptionHandlerException.h"
#include "DownloadResult.h"
#include "SimpleRandomizer.h"
#include "bittorrent_helper.h"

namespace aria2 {

extern void showVersion();
extern void showUsage(const std::string& keyword, const OptionParser& oparser);

static void overrideWithEnv(Option& op, const OptionParser& optionParser,
			    const std::string& pref,
			    const std::string& envName)
{
  char* value = getenv(envName.c_str());
  if(value) {
    try {
      optionParser.findByName(pref)->parse(op, value);
    } catch(Exception& e) {
      std::cerr << "Caught Error while parsing environment variable"
		<< " '" << envName << "'"
		<< "\n"
		<< e.stackTrace();
    }
  }
}

void option_processing(Option& op, std::deque<std::string>& uris,
		       int argc, char* const argv[])
{
  OptionParser oparser;
  oparser.setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
  try {
    bool noConf = false;
    std::string ucfname;
    std::stringstream cmdstream;
    oparser.parseArg(cmdstream, uris, argc, argv);
    {
      // first evaluate --no-conf and --conf-path options.
      Option op;
      oparser.parse(op, cmdstream);
      noConf = op.getAsBool(PREF_NO_CONF);
      ucfname = op.get(PREF_CONF_PATH);

      if(op.defined("version")) {
	showVersion();
	exit(DownloadResult::FINISHED);
      }
      if(op.defined("help")) {
	std::string keyword;
	if(op.get("help").empty()) {
	  keyword = TAG_BASIC;
	} else {
	  keyword = op.get("help");
	  if(Util::startsWith(keyword, "--")) {
	    keyword = keyword.substr(2);
	  }
	  std::string::size_type eqpos = keyword.find("=");
	  if(eqpos != std::string::npos) {
	    keyword = keyword.substr(0, eqpos);
	  }
	}
	showUsage(keyword, oparser);
	exit(DownloadResult::FINISHED);
      }
    }

    oparser.parseDefaultValues(op);

    if(!noConf) {
      std::string cfname = 
	ucfname.empty() ?
	oparser.findByName(PREF_CONF_PATH)->getDefaultValue():
	ucfname;

      if(File(cfname).isFile()) {
	std::ifstream cfstream(cfname.c_str(), std::ios::binary);
	try {
	  oparser.parse(op, cfstream);
	} catch(OptionHandlerException& e) {
	  std::cerr << "Parse error in " << cfname << "\n"
		    << e.stackTrace() << "\n"
		    << "Usage:" << "\n"
		    << oparser.findByName(e.getOptionName())->getDescription()
		    << std::endl;
	  exit(DownloadResult::UNKNOWN_ERROR);
	} catch(Exception& e) {
	  std::cerr << "Parse error in " << cfname << "\n"
		    << e.stackTrace() << std::endl;
	  exit(DownloadResult::UNKNOWN_ERROR);
	}
      } else if(!ucfname.empty()) {
	std::cerr << StringFormat("Configuration file %s is not found.",
				  cfname.c_str())
		  << "\n";
	showUsage(TAG_HELP, oparser);
	exit(DownloadResult::UNKNOWN_ERROR);
      }
    }
    // Override configuration with environment variables.
    overrideWithEnv(op, oparser, PREF_HTTP_PROXY, "http_proxy");
    overrideWithEnv(op, oparser, PREF_HTTPS_PROXY, "https_proxy");
    overrideWithEnv(op, oparser, PREF_FTP_PROXY, "ftp_proxy");
    overrideWithEnv(op, oparser, PREF_ALL_PROXY, "all_proxy");
    overrideWithEnv(op, oparser, PREF_NO_PROXY, "no_proxy");

    // we must clear eof bit and seek to the beginning of the buffer.
    cmdstream.clear();
    cmdstream.seekg(0, std::ios::beg);
    // finaly let's parse and store command-iine options.
    oparser.parse(op, cmdstream);
  } catch(OptionHandlerException& e) {
    std::cerr << e.stackTrace() << "\n"
	      << "Usage:" << "\n"
	      << oparser.findByName(e.getOptionName())
	      << std::endl;
    exit(DownloadResult::UNKNOWN_ERROR);
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
    showUsage(TAG_HELP, oparser);
    exit(DownloadResult::UNKNOWN_ERROR);
  }
  if(
#ifdef ENABLE_XML_RPC
     !op.getAsBool(PREF_ENABLE_XML_RPC) &&
#endif // ENABLE_XML_RPC
#ifdef ENABLE_BITTORRENT
     op.blank(PREF_TORRENT_FILE) &&
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
     op.blank(PREF_METALINK_FILE) &&
#endif // ENABLE_METALINK
     op.blank(PREF_INPUT_FILE)) {
    if(uris.empty()) {
      std::cerr << MSG_URI_REQUIRED << std::endl;
      showUsage(TAG_HELP, oparser);
      exit(DownloadResult::UNKNOWN_ERROR);
    }
  }
#ifdef HAVE_DAEMON
  if(op.getAsBool(PREF_DAEMON)) {
    if(File::getCurrentDir() == ".") {
      std::cerr << "Failed to get the current working directory."
		<< " With -D option engaged,"
		<< " the default value of --dir option is /." << std::endl;
    }
    if(daemon(0, 0) < 0) {
      perror(MSG_DAEMON_FAILED);
      exit(DownloadResult::UNKNOWN_ERROR);
    }
  }
#endif // HAVE_DAEMON
#ifdef ENABLE_BITTORRENT
  bittorrent::generateStaticPeerId(op.get(PREF_PEER_ID_PREFIX),
				   SimpleRandomizer::getInstance());
#endif // ENABLE_BITTORRENT
}

} // namespace aria2
