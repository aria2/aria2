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
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

extern char* optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

namespace aria2 {

extern void showVersion();
extern void showUsage(const std::string& category, const Option* option);

static std::string toBoolArg(const char* optarg)
{
  std::string arg;
  if(!optarg || std::string(optarg) == "") {
    arg = V_TRUE;
  } else {
    arg = optarg;
  }
  return arg;
}

Option* createDefaultOption()
{
  Option* op = new Option();
  op->put(PREF_STDOUT_LOG, V_FALSE);
  op->put(PREF_DIR, ".");
  op->put(PREF_SPLIT, "1");
  op->put(PREF_DAEMON, V_FALSE);
  op->put(PREF_SEGMENT_SIZE, Util::itos((int32_t)(1024*1024)));
  op->put(PREF_LISTEN_PORT, "6881-6999");
  op->put(PREF_METALINK_SERVERS, "5");
  op->put(PREF_FOLLOW_TORRENT,
#ifdef ENABLE_BITTORRENT
	  V_TRUE
#else
	  V_FALSE
#endif // ENABLE_BITTORRENT
	  );
  op->put(PREF_FOLLOW_METALINK,
#ifdef ENABLE_METALINK
	  V_TRUE
#else
	  V_FALSE
#endif // ENABLE_METALINK
	  );
  op->put(PREF_RETRY_WAIT, "5");
  op->put(PREF_TIMEOUT, "60");
  op->put(PREF_DNS_TIMEOUT, "30");
  op->put(PREF_PEER_CONNECTION_TIMEOUT, "20");
  op->put(PREF_BT_TIMEOUT, "180");
  op->put(PREF_BT_REQUEST_TIMEOUT, "60");
  op->put(PREF_BT_KEEP_ALIVE_INTERVAL, "120");
  op->put(PREF_MIN_SEGMENT_SIZE, "1048576");// 1M
  op->put(PREF_MAX_TRIES, "5");
  op->put(PREF_HTTP_AUTH_SCHEME, V_BASIC);
  op->put(PREF_HTTP_PROXY_METHOD, V_TUNNEL);
  op->put(PREF_FTP_TYPE, V_BINARY);
  op->put(PREF_FTP_VIA_HTTP_PROXY, V_TUNNEL);
  op->put(PREF_AUTO_SAVE_INTERVAL, "60");
  op->put(PREF_DIRECT_FILE_MAPPING, V_TRUE);
  op->put(PREF_LOWEST_SPEED_LIMIT, "0");
  op->put(PREF_MAX_DOWNLOAD_LIMIT, "0");
  op->put(PREF_MAX_UPLOAD_LIMIT, "0");
  op->put(PREF_STARTUP_IDLE_TIME, "10");
  op->put(PREF_TRACKER_MAX_TRIES, "10");
  op->put(PREF_FILE_ALLOCATION, V_PREALLOC);
  op->put(PREF_NO_FILE_ALLOCATION_LIMIT, "5242880"); // 5MiB
  op->put(PREF_ALLOW_OVERWRITE, V_FALSE);
  op->put(PREF_REALTIME_CHUNK_CHECKSUM, V_TRUE);
  op->put(PREF_CHECK_INTEGRITY, V_FALSE);
  op->put(PREF_NETRC_PATH, Util::getHomeDir()+"/.netrc");
  op->put(PREF_CONTINUE, V_FALSE);
  op->put(PREF_USER_AGENT, "aria2");
  op->put(PREF_NO_NETRC, V_FALSE);
  op->put(PREF_MAX_CONCURRENT_DOWNLOADS, "5");
  op->put(PREF_DIRECT_DOWNLOAD_TIMEOUT, "300");
  op->put(PREF_FORCE_SEQUENTIAL, V_FALSE);
  op->put(PREF_AUTO_FILE_RENAMING, V_TRUE);
  op->put(PREF_PARAMETERIZED_URI, V_FALSE);
  op->put(PREF_ENABLE_HTTP_KEEP_ALIVE, V_FALSE);
  op->put(PREF_ENABLE_HTTP_PIPELINING, V_FALSE);
  op->put(PREF_MAX_HTTP_PIPELINING, "2");
  op->put(PREF_SEED_RATIO, "1.0");
  op->put(PREF_ENABLE_DIRECT_IO, V_FALSE);
  op->put(PREF_ALLOW_PIECE_LENGTH_CHANGE, V_FALSE);
  op->put(PREF_METALINK_PREFERRED_PROTOCOL, V_NONE);
  op->put(PREF_METALINK_ENABLE_UNIQUE_PROTOCOL, V_TRUE);
  op->put(PREF_ENABLE_PEER_EXCHANGE, V_TRUE);
  op->put(PREF_ENABLE_DHT, V_FALSE);
  op->put(PREF_DHT_LISTEN_PORT, "6881-6999");
  op->put(PREF_DHT_FILE_PATH, Util::getHomeDir()+"/.aria2/dht.dat");
  op->put(PREF_BT_MIN_CRYPTO_LEVEL, V_PLAIN);
  op->put(PREF_BT_REQUIRE_CRYPTO, V_FALSE);
  op->put(PREF_QUIET, V_FALSE);
  op->put(PREF_STOP, "0");
  return op;
}

Option* option_processing(int argc, char* const argv[])
{
  std::stringstream cmdstream;
  int32_t c;
  Option* op = createDefaultOption();

  // following options are not parsed by OptionHandler and not stored in Option.
  bool noConf = false;
  std::string defaultCfname = Util::getHomeDir()+"/.aria2/aria2.conf";
  std::string ucfname;

  while(1) {
    int optIndex = 0;
    int lopt;
    static struct option longOpts[] = {
#ifdef HAVE_DAEMON
      { PREF_DAEMON, no_argument, NULL, 'D' },
#endif // HAVE_DAEMON
      { PREF_DIR, required_argument, NULL, 'd' },
      { PREF_OUT, required_argument, NULL, 'o' },
      { PREF_LOG, required_argument, NULL, 'l' },
      { PREF_SPLIT, required_argument, NULL, 's' },
      { PREF_TIMEOUT, required_argument, NULL, 't' },
      { PREF_MAX_TRIES, required_argument, NULL, 'm' },
      { PREF_HTTP_PROXY, required_argument, &lopt, 1 },
      { PREF_HTTP_USER, required_argument, &lopt, 2 },
      { PREF_HTTP_PASSWD, required_argument, &lopt, 3 },
      { PREF_HTTP_PROXY_USER, required_argument, &lopt, 4 },
      { PREF_HTTP_PROXY_PASSWD, required_argument, &lopt, 5 },
      { PREF_HTTP_AUTH_SCHEME, required_argument, &lopt, 6 },
      { PREF_REFERER, required_argument, &lopt, 7 },
      { PREF_RETRY_WAIT, required_argument, &lopt, 8 },
      { PREF_FTP_USER, required_argument, &lopt, 9 },
      { PREF_FTP_PASSWD, required_argument, &lopt, 10 },
      { PREF_FTP_TYPE, required_argument, &lopt, 11 },
      { PREF_FTP_PASV, no_argument, NULL, 'p' },
      { PREF_FTP_VIA_HTTP_PROXY, required_argument, &lopt, 12 },
      //{ PREF_MIN_SEGMENT_SIZE, required_argument, &lopt, 13 },
      { PREF_HTTP_PROXY_METHOD, required_argument, &lopt, 14 },
      { PREF_LOWEST_SPEED_LIMIT, required_argument, &lopt, 200 },
      { PREF_MAX_DOWNLOAD_LIMIT, required_argument, &lopt, 201 },
      { PREF_FILE_ALLOCATION, required_argument, 0, 'a' },
      { PREF_ALLOW_OVERWRITE, required_argument, &lopt, 202 },
#ifdef ENABLE_MESSAGE_DIGEST
      { PREF_CHECK_INTEGRITY, required_argument, &lopt, 203 },
      { PREF_REALTIME_CHUNK_CHECKSUM, required_argument, &lopt, 204 },
#endif // ENABLE_MESSAGE_DIGEST
      { PREF_CONTINUE, no_argument, 0, 'c' },
      { PREF_USER_AGENT, required_argument, 0, 'U' },
      { PREF_NO_NETRC, no_argument, 0, 'n' },
      { PREF_INPUT_FILE, required_argument, 0, 'i' },
      { PREF_MAX_CONCURRENT_DOWNLOADS, required_argument, 0, 'j' },
      { PREF_LOAD_COOKIES, required_argument, &lopt, 205 },
      { PREF_FORCE_SEQUENTIAL, optional_argument, 0, 'Z' },
      { PREF_AUTO_FILE_RENAMING, optional_argument, &lopt, 206 },
      { PREF_PARAMETERIZED_URI, optional_argument, 0, 'P' },
      { PREF_ENABLE_HTTP_KEEP_ALIVE, optional_argument, &lopt, 207 },
      { PREF_ENABLE_HTTP_PIPELINING, optional_argument, &lopt, 208 },
      { PREF_NO_FILE_ALLOCATION_LIMIT, required_argument, &lopt, 209 },
#ifdef ENABLE_DIRECT_IO
      { PREF_ENABLE_DIRECT_IO, optional_argument, &lopt, 210 },
#endif // ENABLE_DIRECT_IO
      { PREF_ALLOW_PIECE_LENGTH_CHANGE, required_argument, &lopt, 211 },
      { PREF_NO_CONF, no_argument, &lopt, 212 },
      { PREF_CONF_PATH, required_argument, &lopt, 213 },
      { PREF_STOP, required_argument, &lopt, 214 },
      { PREF_HEADER, required_argument, &lopt, 215 },
      { PREF_QUIET, optional_argument, 0, 'q' },
#if defined ENABLE_BITTORRENT || ENABLE_METALINK
      { PREF_SHOW_FILES, no_argument, NULL, 'S' },
      { PREF_SELECT_FILE, required_argument, &lopt, 21 },
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
      { PREF_TORRENT_FILE, required_argument, NULL, 'T' },
      { PREF_LISTEN_PORT, required_argument, &lopt, 15 },
      { PREF_FOLLOW_TORRENT, required_argument, &lopt, 16 },
      { PREF_NO_PREALLOCATION, no_argument, &lopt, 18 },
      { PREF_DIRECT_FILE_MAPPING, required_argument, &lopt, 19 },
      // TODO remove upload-limit.
      //{ "upload-limit", required_argument, &lopt, 20 },
      { PREF_SEED_TIME, required_argument, &lopt, 22 },
      { PREF_SEED_RATIO, required_argument, &lopt, 23 },
      { PREF_MAX_UPLOAD_LIMIT, required_argument, &lopt, 24 },
      { PREF_PEER_ID_PREFIX, required_argument, &lopt, 25 },
      { PREF_ENABLE_PEER_EXCHANGE, optional_argument, &lopt, 26 },
      { PREF_ENABLE_DHT, optional_argument, &lopt, 27 },
      { PREF_DHT_LISTEN_PORT, required_argument, &lopt, 28 },
      { PREF_DHT_ENTRY_POINT, required_argument, &lopt, 29 },
      { PREF_BT_MIN_CRYPTO_LEVEL, required_argument, &lopt, 30 },
      { PREF_BT_REQUIRE_CRYPTO, required_argument, &lopt, 31 },
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      { PREF_METALINK_FILE, required_argument, NULL, 'M' },
      { PREF_METALINK_SERVERS, required_argument, NULL, 'C' },
      { PREF_METALINK_VERSION, required_argument, &lopt, 100 },
      { PREF_METALINK_LANGUAGE, required_argument, &lopt, 101 },
      { PREF_METALINK_OS, required_argument, &lopt, 102 },
      { PREF_FOLLOW_METALINK, required_argument, &lopt, 103 },
      { PREF_METALINK_LOCATION, required_argument, &lopt, 104 },
      { PREF_METALINK_PREFERRED_PROTOCOL, required_argument, &lopt, 105 },
      { PREF_METALINK_ENABLE_UNIQUE_PROTOCOL, optional_argument, &lopt, 106 },
#endif // ENABLE_METALINK
      { "version", no_argument, NULL, 'v' },
      { "help", optional_argument, NULL, 'h' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, "Dd:o:l:s:pt:m:vh::ST:M:C:a:cU:ni:j:Z::P::q::", longOpts, &optIndex);
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
	cmdstream << PREF_HTTP_PROXY_USER << "=" << optarg << "\n";
	break;
      case 5: 
	cmdstream << PREF_HTTP_PROXY_PASSWD << "=" << optarg << "\n";
	break;
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
	cmdstream << PREF_FTP_VIA_HTTP_PROXY << "=" << optarg << "\n";
	break;
      case 13:
	cmdstream << PREF_MIN_SEGMENT_SIZE << "=" << optarg << "\n";
	break;
      case 14:
	cmdstream << PREF_HTTP_PROXY_METHOD << "=" << optarg << "\n";
	break;
      case 15:
	cmdstream << PREF_LISTEN_PORT << "=" << optarg << "\n";
	break;
      case 16:
	cmdstream << PREF_FOLLOW_TORRENT << "=" << optarg << "\n";
	break;
      case 18:
	cmdstream << PREF_NO_PREALLOCATION << "=" << V_TRUE << "\n";
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
      case 24:
	cmdstream << PREF_MAX_UPLOAD_LIMIT << "=" << optarg << "\n";
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
      case 203:
	cmdstream << PREF_CHECK_INTEGRITY << "=" << optarg << "\n";
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
      cmdstream << PREF_FTP_PASV << "=" << V_TRUE << "\n";
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
    case 'v':
      showVersion();
      exit(EXIT_SUCCESS);
    case 'h':
      {
	std::string category;
	if(optarg == 0 || std::string(optarg) == "") {
	  category = TAG_BASIC;
	} else {
	  category = optarg;
	}
	showUsage(category, createDefaultOption());
	exit(EXIT_SUCCESS);
      }
    default:
      exit(EXIT_FAILURE);
    }
  }

  {
    OptionParser oparser;
    oparser.setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
    if(!noConf) {
      std::string cfname;
      if(ucfname.size()) {
	cfname = ucfname;
      } else {
	cfname = defaultCfname;
      }
      if(File(cfname).isFile()) {
	std::ifstream cfstream(cfname.c_str());
	try {
	  oparser.parse(op, cfstream);
	} catch(Exception& e) {
	  std::cerr << "Parse error in " << cfname << "\n"
		    << e.stackTrace() << std::endl;
	  exit(EXIT_FAILURE);
	}
      } else if(ucfname.size()) {
	std::cout << StringFormat("Configuration file %s is not found.", cfname.c_str())
		  << "\n";
      }
    }
    try {
      oparser.parse(op, cmdstream);
    } catch(Exception& e) {
      std::cerr << e.stackTrace() << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  if(op->defined(PREF_HTTP_USER)) {
    op->put(PREF_HTTP_AUTH_ENABLED, V_TRUE);
  }
  if(op->defined(PREF_HTTP_PROXY_USER)) {
    op->put(PREF_HTTP_PROXY_AUTH_ENABLED, V_TRUE);
  }
  if(
#ifdef ENABLE_BITTORRENT
     !op->defined(PREF_TORRENT_FILE) &&
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
     !op->defined(PREF_METALINK_FILE) &&
#endif // ENABLE_METALINK
     !op->defined(PREF_INPUT_FILE)) {
    if(optind == argc) {
      std::cerr << MSG_URI_REQUIRED << std::endl;
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
