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

extern char* optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

namespace aria2 {

extern void showVersion();
extern void showUsage(const std::string& keyword, const OptionParser& oparser);

static std::string toBoolArg(const char* optarg)
{
  std::string arg;
  if(!optarg || strlen(optarg) == 0) {
    arg = V_TRUE;
  } else {
    arg = optarg;
  }
  return arg;
}

static void overrideWithEnv(Option* op, const OptionParser& optionParser,
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

Option* option_processing(int argc, char* const argv[])
{
  std::stringstream cmdstream;
  int32_t c;
  Option* op = new Option();

  // following options are not parsed by OptionHandler and not stored in Option.
  bool noConf = false;
  std::string ucfname;

  OptionParser oparser;
  oparser.setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
  try {
    oparser.parseDefaultValues(op);
  } catch(Exception& e) {
    std::cerr << e.stackTrace();
    exit(EXIT_FAILURE);
  }

  while(1) {
    int optIndex = 0;
    int lopt;
    static struct option longOpts[] = {
#ifdef HAVE_DAEMON
      { PREF_DAEMON.c_str(), no_argument, NULL, 'D' },
#endif // HAVE_DAEMON
      { PREF_DIR.c_str(), required_argument, NULL, 'd' },
      { PREF_OUT.c_str(), required_argument, NULL, 'o' },
      { PREF_LOG.c_str(), required_argument, NULL, 'l' },
      { PREF_SPLIT.c_str(), required_argument, NULL, 's' },
      { PREF_TIMEOUT.c_str(), required_argument, NULL, 't' },
      { PREF_MAX_TRIES.c_str(), required_argument, NULL, 'm' },
      { PREF_HTTP_PROXY.c_str(), required_argument, &lopt, 1 },
      { PREF_HTTP_USER.c_str(), required_argument, &lopt, 2 },
      { PREF_HTTP_PASSWD.c_str(), required_argument, &lopt, 3 },
      { "http-proxy-user", required_argument, &lopt, 4 },
      { "http-proxy-passwd", required_argument, &lopt, 5 },
      { PREF_HTTP_AUTH_SCHEME.c_str(), required_argument, &lopt, 6 },
      { PREF_REFERER.c_str(), required_argument, &lopt, 7 },
      { PREF_RETRY_WAIT.c_str(), required_argument, &lopt, 8 },
      { PREF_FTP_USER.c_str(), required_argument, &lopt, 9 },
      { PREF_FTP_PASSWD.c_str(), required_argument, &lopt, 10 },
      { PREF_FTP_TYPE.c_str(), required_argument, &lopt, 11 },
      { PREF_FTP_PASV.c_str(), optional_argument, 0, 'p' },
      { "ftp-via-http-proxy", required_argument, &lopt, 12 },
      { "http-proxy-method", required_argument, &lopt, 14 },
      { PREF_LOWEST_SPEED_LIMIT.c_str(), required_argument, &lopt, 200 },
      { PREF_MAX_DOWNLOAD_LIMIT.c_str(), required_argument, &lopt, 201 },
      { PREF_FILE_ALLOCATION.c_str(), required_argument, 0, 'a' },
      { PREF_ALLOW_OVERWRITE.c_str(), required_argument, &lopt, 202 },
#ifdef ENABLE_MESSAGE_DIGEST
      { PREF_CHECK_INTEGRITY.c_str(), optional_argument, 0, 'V' },
      { PREF_REALTIME_CHUNK_CHECKSUM.c_str(), required_argument, &lopt, 204 },
#endif // ENABLE_MESSAGE_DIGEST
      { PREF_CONTINUE.c_str(), no_argument, 0, 'c' },
      { PREF_USER_AGENT.c_str(), required_argument, 0, 'U' },
      { PREF_NO_NETRC.c_str(), no_argument, 0, 'n' },
      { PREF_INPUT_FILE.c_str(), required_argument, 0, 'i' },
      { PREF_MAX_CONCURRENT_DOWNLOADS.c_str(), required_argument, 0, 'j' },
      { PREF_LOAD_COOKIES.c_str(), required_argument, &lopt, 205 },
      { PREF_FORCE_SEQUENTIAL.c_str(), optional_argument, 0, 'Z' },
      { PREF_AUTO_FILE_RENAMING.c_str(), optional_argument, &lopt, 206 },
      { PREF_PARAMETERIZED_URI.c_str(), optional_argument, 0, 'P' },
      { PREF_ENABLE_HTTP_KEEP_ALIVE.c_str(), optional_argument, &lopt, 207 },
      { PREF_ENABLE_HTTP_PIPELINING.c_str(), optional_argument, &lopt, 208 },
      { PREF_NO_FILE_ALLOCATION_LIMIT.c_str(), required_argument, &lopt, 209 },
#ifdef ENABLE_DIRECT_IO
      { PREF_ENABLE_DIRECT_IO.c_str(), optional_argument, &lopt, 210 },
#endif // ENABLE_DIRECT_IO
      { PREF_ALLOW_PIECE_LENGTH_CHANGE.c_str(), required_argument, &lopt, 211 },
      { PREF_NO_CONF.c_str(), no_argument, &lopt, 212 },
      { PREF_CONF_PATH.c_str(), required_argument, &lopt, 213 },
      { PREF_STOP.c_str(), required_argument, &lopt, 214 },
      { PREF_HEADER.c_str(), required_argument, &lopt, 215 },
      { PREF_QUIET.c_str(), optional_argument, 0, 'q' },
#ifdef ENABLE_ASYNC_DNS
      { PREF_ASYNC_DNS.c_str(), optional_argument, &lopt, 216 },
#endif // ENABLE_ASYNC_DNS
      { PREF_FTP_REUSE_CONNECTION.c_str(), optional_argument, &lopt, 217 },
      { PREF_SUMMARY_INTERVAL.c_str(), required_argument, &lopt, 218 },
      { PREF_LOG_LEVEL.c_str(), required_argument, &lopt, 219 },
      { PREF_URI_SELECTOR.c_str(), required_argument, &lopt, 220 },
      { PREF_SERVER_STAT_IF.c_str(), required_argument, &lopt, 221 },
      { PREF_SERVER_STAT_OF.c_str(), required_argument, &lopt, 222 },
      { PREF_SERVER_STAT_TIMEOUT.c_str(), required_argument, &lopt, 223 },
      { PREF_REMOTE_TIME.c_str(), optional_argument, 0, 'R' },
      { PREF_CONNECT_TIMEOUT.c_str(), required_argument, &lopt, 224 },
      { PREF_MAX_FILE_NOT_FOUND.c_str(), required_argument, &lopt, 225 },
      { PREF_AUTO_SAVE_INTERVAL.c_str(), required_argument, &lopt, 226 },
      { PREF_HTTPS_PROXY.c_str(), required_argument, &lopt, 227 },
      { PREF_FTP_PROXY.c_str(), required_argument, &lopt, 228 },
      { PREF_ALL_PROXY.c_str(), required_argument, &lopt, 229 },
      { PREF_PROXY_METHOD.c_str(), required_argument, &lopt, 230 },
      { PREF_CERTIFICATE.c_str(), required_argument, &lopt, 231 },
      { PREF_PRIVATE_KEY.c_str(), required_argument, &lopt, 232 },
      { PREF_CA_CERTIFICATE.c_str(), optional_argument, &lopt, 233 },
      { PREF_CHECK_CERTIFICATE.c_str(), optional_argument, &lopt, 234 },
      { PREF_NO_PROXY.c_str(), required_argument, &lopt, 235 },
      { PREF_USE_HEAD.c_str(), optional_argument, &lopt, 236 },
#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
      { PREF_SHOW_FILES.c_str(), no_argument, NULL, 'S' },
      { PREF_SELECT_FILE.c_str(), required_argument, &lopt, 21 },
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
      { PREF_TORRENT_FILE.c_str(), required_argument, NULL, 'T' },
      { PREF_LISTEN_PORT.c_str(), required_argument, &lopt, 15 },
      { PREF_FOLLOW_TORRENT.c_str(), required_argument, &lopt, 16 },
      { PREF_DIRECT_FILE_MAPPING.c_str(), required_argument, &lopt, 19 },
      // TODO remove upload-limit.
      //{ "upload-limit".c_str(), required_argument, &lopt, 20 },
      { PREF_SEED_TIME.c_str(), required_argument, &lopt, 22 },
      { PREF_SEED_RATIO.c_str(), required_argument, &lopt, 23 },
      { PREF_MAX_UPLOAD_LIMIT.c_str(), required_argument, 0, 'u' },
      { PREF_PEER_ID_PREFIX.c_str(), required_argument, &lopt, 25 },
      { PREF_ENABLE_PEER_EXCHANGE.c_str(), optional_argument, &lopt, 26 },
      { PREF_ENABLE_DHT.c_str(), optional_argument, &lopt, 27 },
      { PREF_DHT_LISTEN_PORT.c_str(), required_argument, &lopt, 28 },
      { PREF_DHT_ENTRY_POINT.c_str(), required_argument, &lopt, 29 },
      { PREF_BT_MIN_CRYPTO_LEVEL.c_str(), required_argument, &lopt, 30 },
      { PREF_BT_REQUIRE_CRYPTO.c_str(), required_argument, &lopt, 31 },
      { PREF_BT_REQUEST_PEER_SPEED_LIMIT.c_str(), required_argument, &lopt, 32 },
      { PREF_BT_MAX_OPEN_FILES.c_str(), required_argument, &lopt, 33 },
      { PREF_BT_SEED_UNVERIFIED.c_str(), optional_argument, &lopt, 34 },
      { PREF_DHT_FILE_PATH.c_str(), required_argument, &lopt, 35 },
      { PREF_MAX_OVERALL_UPLOAD_LIMIT.c_str(), required_argument, &lopt, 36 },
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      { PREF_METALINK_FILE.c_str(), required_argument, NULL, 'M' },
      { PREF_METALINK_SERVERS.c_str(), required_argument, NULL, 'C' },
      { PREF_METALINK_VERSION.c_str(), required_argument, &lopt, 100 },
      { PREF_METALINK_LANGUAGE.c_str(), required_argument, &lopt, 101 },
      { PREF_METALINK_OS.c_str(), required_argument, &lopt, 102 },
      { PREF_FOLLOW_METALINK.c_str(), required_argument, &lopt, 103 },
      { PREF_METALINK_LOCATION.c_str(), required_argument, &lopt, 104 },
      { PREF_METALINK_PREFERRED_PROTOCOL.c_str(), required_argument, &lopt, 105 },
      { PREF_METALINK_ENABLE_UNIQUE_PROTOCOL.c_str(), optional_argument, &lopt, 106 },
#endif // ENABLE_METALINK
      { "version", no_argument, NULL, 'v' },
      { "help", optional_argument, NULL, 'h' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, 
		    "Dd:o:l:s:p::t:m:vh::ST:M:C:a:cU:ni:j:Z::P::q::R::V::u:",
		    longOpts, &optIndex);
    if(c == -1) {
      break;
    }
    switch(c) {
    case 0:{
      switch(lopt) {
      case 1:
	cmdstream << PREF_HTTP_PROXY << "=" << optarg << "\n";
	break;
      case 2:
	cmdstream << PREF_HTTP_USER << "=" << optarg << "\n";
	break;
      case 3:
	cmdstream << PREF_HTTP_PASSWD << "=" << optarg << "\n";
	break;
      case 4:
	std::cout << "--http-proxy-user was deprecated. See --http-proxy,"
		  << " --https-proxy, --ftp-proxy, --all-proxy options."
		  << std::endl;
	exit(EXIT_FAILURE);
      case 5: 
	std::cout << "--http-proxy-passwd was deprecated. See --http-proxy,"
		  << " --https-proxy, --ftp-proxy, --all-proxy options."
		  << std::endl;
	exit(EXIT_FAILURE);
      case 6:
	cmdstream << PREF_HTTP_AUTH_SCHEME << "=" << optarg << "\n";
	break;
      case 7:
	cmdstream << PREF_REFERER << "=" << optarg << "\n";
	break;
      case 8:
	cmdstream << PREF_RETRY_WAIT << "=" << optarg << "\n";
	break;
      case 9:
	cmdstream << PREF_FTP_USER << "=" << optarg << "\n";
	break;
      case 10:
	cmdstream << PREF_FTP_PASSWD << "=" << optarg << "\n";
	break;
      case 11:
	cmdstream << PREF_FTP_TYPE << "=" << optarg << "\n";
	break;
      case 12:
	std::cout << "--ftp-via-http-proxy was deprecated."
		  << " Use --http-proxy-method option instead."
		  << std::endl;
	exit(EXIT_FAILURE);
      case 14:
	std::cout << "--http-proxy-method was deprecated."
		  << " Use --proxy-method option instead."
		  << std::endl;
	exit(EXIT_FAILURE);
      case 15:
	cmdstream << PREF_LISTEN_PORT << "=" << optarg << "\n";
	break;
      case 16:
	cmdstream << PREF_FOLLOW_TORRENT << "=" << optarg << "\n";
	break;
      case 19:
	cmdstream << PREF_DIRECT_FILE_MAPPING << "=" << optarg << "\n";
	break;
      case 21:
	cmdstream << PREF_SELECT_FILE << "=" << optarg << "\n";
	break;
      case 22:
	cmdstream << PREF_SEED_TIME << "=" << optarg << "\n";
	break;
      case 23:
	cmdstream << PREF_SEED_RATIO << "=" << optarg << "\n";
	break;
      case 25:
	cmdstream << PREF_PEER_ID_PREFIX << "=" << optarg << "\n";
	break;
      case 26:
	cmdstream << PREF_ENABLE_PEER_EXCHANGE << "=" << toBoolArg(optarg) << "\n";
	break;
      case 27:
	cmdstream << PREF_ENABLE_DHT << "=" << toBoolArg(optarg) << "\n";
	break;
      case 28:
	cmdstream << PREF_DHT_LISTEN_PORT << "=" << optarg << "\n";
	break;
      case 29:
	cmdstream << PREF_DHT_ENTRY_POINT << "=" << optarg << "\n";
	break;
      case 30:
	cmdstream << PREF_BT_MIN_CRYPTO_LEVEL << "=" << optarg << "\n";
	break;
      case 31:
	cmdstream << PREF_BT_REQUIRE_CRYPTO << "=" << optarg << "\n";
	break;
      case 32:
	cmdstream << PREF_BT_REQUEST_PEER_SPEED_LIMIT << "=" << optarg << "\n";
	break;
      case 33:
	cmdstream << PREF_BT_MAX_OPEN_FILES << "=" << optarg << "\n";
	break;
      case 34:
	cmdstream << PREF_BT_SEED_UNVERIFIED << "=" << toBoolArg(optarg)
		  << "\n";
	break;
      case 35:
	cmdstream << PREF_DHT_FILE_PATH << "=" << optarg << "\n";
	break;
      case 36:
	cmdstream << PREF_MAX_OVERALL_UPLOAD_LIMIT << "=" << optarg << "\n";
	break;
      case 100:
	cmdstream << PREF_METALINK_VERSION << "=" << optarg << "\n";
	break;
      case 101:
	cmdstream << PREF_METALINK_LANGUAGE << "=" << optarg << "\n";
	break;
      case 102:
	cmdstream << PREF_METALINK_OS << "=" << optarg << "\n";
	break;
      case 103:
	cmdstream << PREF_FOLLOW_METALINK << "=" << optarg << "\n";
	break;
      case 104:
	cmdstream << PREF_METALINK_LOCATION << "=" << optarg << "\n";
	break;
      case 105:
	cmdstream << PREF_METALINK_PREFERRED_PROTOCOL << "=" << optarg << "\n";
	break;
      case 106:
	cmdstream << PREF_METALINK_ENABLE_UNIQUE_PROTOCOL << "=" << toBoolArg(optarg) << "\n";
	break;
      case 200:
	cmdstream << PREF_LOWEST_SPEED_LIMIT << "=" << optarg << "\n";
	break;
      case 201:
	cmdstream << PREF_MAX_DOWNLOAD_LIMIT << "=" << optarg << "\n";
	break;
      case 202:
	cmdstream << PREF_ALLOW_OVERWRITE << "=" << optarg << "\n";
	break;
      case 204:
	cmdstream << PREF_REALTIME_CHUNK_CHECKSUM << "=" << optarg << "\n";
	break;
      case 205:
	cmdstream << PREF_LOAD_COOKIES << "=" << optarg << "\n";
	break;
      case 206:
	cmdstream << PREF_AUTO_FILE_RENAMING << "=" << toBoolArg(optarg) << "\n";
	break;
      case 207:
	cmdstream << PREF_ENABLE_HTTP_KEEP_ALIVE << "=" << toBoolArg(optarg) << "\n";
	break;
      case 208:
	cmdstream << PREF_ENABLE_HTTP_PIPELINING << "=" << toBoolArg(optarg) << "\n";
	break;
      case 209:
	cmdstream << PREF_NO_FILE_ALLOCATION_LIMIT << "=" << optarg << "\n";
	break;
      case 210:
	cmdstream << PREF_ENABLE_DIRECT_IO << "=" << toBoolArg(optarg) << "\n";
	break;
      case 211:
	cmdstream << PREF_ALLOW_PIECE_LENGTH_CHANGE << "=" << optarg << "\n";
	break;
      case 212:
	noConf = true;
	break;
      case 213:
	ucfname = optarg;
	break;
      case 214:
	cmdstream << PREF_STOP << "=" << optarg << "\n";
	break;
      case 215:
	cmdstream << PREF_HEADER << "=" << optarg << "\n";
	break;
#ifdef ENABLE_ASYNC_DNS
      case 216:
	cmdstream << PREF_ASYNC_DNS << "=" << toBoolArg(optarg) << "\n";
	break;
#endif // ENABLE_ASYNC_DNS
      case 217:
	cmdstream << PREF_FTP_REUSE_CONNECTION << "=" << toBoolArg(optarg) << "\n";
	break;
      case 218:
	cmdstream << PREF_SUMMARY_INTERVAL << "=" << optarg << "\n";
	break;
      case 219:
	cmdstream << PREF_LOG_LEVEL << "=" << optarg << "\n";
	break;
      case 220:
	cmdstream << PREF_URI_SELECTOR << "=" << optarg << "\n";
	break;
      case 221:
	cmdstream << PREF_SERVER_STAT_IF << "=" << optarg << "\n";
	break;
      case 222:
	cmdstream << PREF_SERVER_STAT_OF << "=" << optarg << "\n";
	break;
      case 223:
	cmdstream << PREF_SERVER_STAT_TIMEOUT << "=" << optarg << "\n";
	break;
      case 224:
	cmdstream << PREF_CONNECT_TIMEOUT << "=" << optarg << "\n";
	break;
      case 225:
	cmdstream << PREF_MAX_FILE_NOT_FOUND << "=" << optarg << "\n";
	break;
      case 226:
	cmdstream << PREF_AUTO_SAVE_INTERVAL << "=" << optarg << "\n";
	break;
      case 227:
	cmdstream << PREF_HTTPS_PROXY << "=" << optarg << "\n";
	break;
      case 228:
	cmdstream << PREF_FTP_PROXY << "=" << optarg << "\n";
	break;
      case 229:
	cmdstream << PREF_ALL_PROXY << "=" << optarg << "\n";
	break;
      case 230:
	cmdstream << PREF_PROXY_METHOD << "=" << optarg << "\n";
	break;
      case 231:
	cmdstream << PREF_CERTIFICATE << "=" << optarg << "\n";
	break;
      case 232:
	cmdstream << PREF_PRIVATE_KEY << "=" << optarg << "\n";
	break;
      case 233:
	cmdstream << PREF_CA_CERTIFICATE << "=" << optarg << "\n";
	break;
      case 234:
	cmdstream << PREF_CHECK_CERTIFICATE << "=" << toBoolArg(optarg) << "\n";
	break;
      case 235:
	cmdstream << PREF_NO_PROXY << "=" << optarg << "\n";
	break;
      case 236:
	cmdstream << PREF_USE_HEAD << "=" << toBoolArg(optarg) << "\n";
	break;
      }
      break;
    }
#ifdef HAVE_DAEMON
    case 'D':
      cmdstream << PREF_DAEMON << "=" << V_TRUE << "\n";
      break;
#endif // HAVE_DAEMON
    case 'd':
      cmdstream << PREF_DIR << "=" << optarg << "\n";
      break;
    case 'o':
      cmdstream << PREF_OUT << "=" << optarg << "\n";
      break;
    case 'l':
      cmdstream << PREF_LOG << "=" << optarg << "\n";
      break;
    case 's':
      cmdstream << PREF_SPLIT << "=" << optarg << "\n";
      break;
    case 't':
      cmdstream << PREF_TIMEOUT << "=" << optarg << "\n";
      break;
    case 'm':
      cmdstream << PREF_MAX_TRIES << "=" << optarg << "\n";
      break;
    case 'p':
      cmdstream << PREF_FTP_PASV << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'S':
      cmdstream << PREF_SHOW_FILES << "=" << V_TRUE << "\n";
      break;
    case 'T':
      cmdstream << PREF_TORRENT_FILE << "=" << optarg << "\n";
      break;
    case 'M':
      cmdstream << PREF_METALINK_FILE << "=" << optarg << "\n";
      break;
    case 'C':
      cmdstream << PREF_METALINK_SERVERS << "=" << optarg << "\n";
      break;
    case 'a':
      cmdstream << PREF_FILE_ALLOCATION << "=" << optarg << "\n";
      break;
    case 'c':
      cmdstream << PREF_CONTINUE << "=" << V_TRUE << "\n";
      break;
    case 'U':
      cmdstream << PREF_USER_AGENT << "=" << optarg << "\n";
      break;
    case 'n':
      cmdstream << PREF_NO_NETRC << "=" << V_TRUE << "\n";
      break;
    case 'i':
      cmdstream << PREF_INPUT_FILE << "=" << optarg << "\n";
      break;
    case 'j':
      cmdstream << PREF_MAX_CONCURRENT_DOWNLOADS << "=" << optarg << "\n";
      break;
    case 'Z':
      cmdstream << PREF_FORCE_SEQUENTIAL << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'P':
      cmdstream << PREF_PARAMETERIZED_URI << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'q':
      cmdstream << PREF_QUIET << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'R':
      cmdstream << PREF_REMOTE_TIME << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'V':
      cmdstream << PREF_CHECK_INTEGRITY << "=" << toBoolArg(optarg) << "\n";
      break;
    case 'u':
      cmdstream << PREF_MAX_UPLOAD_LIMIT << "=" << optarg << "\n";
      break;
    case 'v':
      showVersion();
      exit(EXIT_SUCCESS);
    case 'h':
      {
	std::string category;
	if(optarg == 0 || strlen(optarg) == 0) {
	  category = TAG_BASIC;
	} else {
	  category = optarg;
	}
	showUsage(category, oparser);
	exit(EXIT_SUCCESS);
      }
    default:
      showUsage(TAG_HELP, oparser);
      exit(EXIT_FAILURE);
    }
  }

  {
    if(!noConf) {
      std::string cfname = 
	ucfname.empty() ?
	oparser.findByName(PREF_CONF_PATH)->getDefaultValue():
	ucfname;

      if(File(cfname).isFile()) {
	std::ifstream cfstream(cfname.c_str());
	try {
	  oparser.parse(op, cfstream);
	} catch(OptionHandlerException& e) {
	  std::cerr << "Parse error in " << cfname << "\n"
		    << e.stackTrace() << "\n"
		    << "Usage:" << "\n"
		    << oparser.findByName(e.getOptionName())->getDescription()
		    << std::endl;
	  exit(EXIT_FAILURE);
	} catch(Exception& e) {
	  std::cerr << "Parse error in " << cfname << "\n"
		    << e.stackTrace() << std::endl;
	  exit(EXIT_FAILURE);
	}
      } else if(!ucfname.empty()) {
	std::cerr << StringFormat("Configuration file %s is not found.",
				  cfname.c_str())
		  << "\n";
	showUsage(TAG_HELP, oparser);
	exit(EXIT_FAILURE);
      }
    }
    // Override configuration with environment variables.
    overrideWithEnv(op, oparser, PREF_HTTP_PROXY, "http_proxy");
    overrideWithEnv(op, oparser, PREF_HTTPS_PROXY, "https_proxy");
    overrideWithEnv(op, oparser, PREF_FTP_PROXY, "ftp_proxy");
    overrideWithEnv(op, oparser, PREF_ALL_PROXY, "all_proxy");
    overrideWithEnv(op, oparser, PREF_NO_PROXY, "no_proxy");

    try {
      oparser.parse(op, cmdstream);
    } catch(OptionHandlerException& e) {
      std::cerr << e.stackTrace() << "\n"
		<< "Usage:" << "\n"
		<< oparser.findByName(e.getOptionName())->getDescription()
		<< std::endl;
      exit(EXIT_FAILURE);
    } catch(Exception& e) {
      std::cerr << e.stackTrace() << std::endl;
      showUsage(TAG_HELP, oparser);
      exit(EXIT_FAILURE);
    }
  }
  if(
#ifdef ENABLE_BITTORRENT
     op->blank(PREF_TORRENT_FILE) &&
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
     op->blank(PREF_METALINK_FILE) &&
#endif // ENABLE_METALINK
     op->blank(PREF_INPUT_FILE)) {
    if(optind == argc) {
      std::cerr << MSG_URI_REQUIRED << std::endl;
      showUsage(TAG_HELP, oparser);
      exit(EXIT_FAILURE);
    }
  }
#ifdef HAVE_DAEMON
  if(op->getAsBool(PREF_DAEMON)) {
    if(daemon(1, 1) < 0) {
      perror(MSG_DAEMON_FAILED);
      exit(EXIT_FAILURE);
    }
  }
#endif // HAVE_DAEMON
  return op;
}

} // namespace aria2
