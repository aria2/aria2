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
#include "Util.h"
#include "message.h"
#include <fstream>
#include <sstream>

extern char* optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

extern void showVersion();
extern void showUsage();

static string toBoolArg(const char* optarg)
{
  string arg;
  if(!optarg || string(optarg) == "") {
    arg = V_TRUE;
  } else {
    arg = optarg;
  }
  return arg;
}

Option* option_processing(int argc, char* const argv[])
{
  stringstream cmdstream;
  int32_t c;
  Option* op = new Option();
  op->put(PREF_STDOUT_LOG, V_FALSE);
  op->put(PREF_DIR, ".");
  op->put(PREF_SPLIT, "1");
  op->put(PREF_DAEMON, V_FALSE);
  op->put(PREF_SEGMENT_SIZE, Util::itos((int32_t)(1024*1024)));
  op->put(PREF_LISTEN_PORT, "-1");
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
  op->put(PREF_DNS_TIMEOUT, "10");
  op->put(PREF_PEER_CONNECTION_TIMEOUT, "60");
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
  op->put(PREF_FILE_ALLOCATION, V_NONE);
  op->put(PREF_ALLOW_OVERWRITE, V_FALSE);
  op->put(PREF_REALTIME_CHUNK_CHECKSUM, V_TRUE);
  op->put(PREF_CHECK_INTEGRITY, V_FALSE);
  op->put(PREF_NETRC_PATH, Util::getHomeDir()+"/.netrc");
  op->put(PREF_CONTINUE, V_FALSE);
  op->put(PREF_USER_AGENT, "aria2");
  op->put(PREF_NO_NETRC, V_FALSE);
  op->put(PREF_MAX_CONCURRENT_DOWNLOADS, "5");
  op->put(PREF_DIRECT_DOWNLOAD_TIMEOUT, "15");
  op->put(PREF_FORCE_SEQUENTIAL, V_FALSE);
  op->put(PREF_AUTO_FILE_RENAMING, V_TRUE);
  op->put(PREF_PARAMETERIZED_URI, V_FALSE);
  op->put(PREF_ENABLE_HTTP_KEEP_ALIVE, V_FALSE);
  op->put(PREF_ENABLE_HTTP_PIPELINING, V_FALSE);
  op->put(PREF_MAX_HTTP_PIPELINING, "2");
  while(1) {
    int optIndex = 0;
    int lopt;
    static struct option longOpts[] = {
#ifdef HAVE_DAEMON
      { "daemon", no_argument, NULL, 'D' },
#endif // HAVE_DAEMON
      { "dir", required_argument, NULL, 'd' },
      { "out", required_argument, NULL, 'o' },
      { "log", required_argument, NULL, 'l' },
      { "split", required_argument, NULL, 's' },
      { "timeout", required_argument, NULL, 't' },
      { "max-tries", required_argument, NULL, 'm' },
      { "http-proxy", required_argument, &lopt, 1 },
      { "http-user", required_argument, &lopt, 2 },
      { "http-passwd", required_argument, &lopt, 3 },
      { "http-proxy-user", required_argument, &lopt, 4 },
      { "http-proxy-passwd", required_argument, &lopt, 5 },
      { "http-auth-scheme", required_argument, &lopt, 6 },
      { "referer", required_argument, &lopt, 7 },
      { "retry-wait", required_argument, &lopt, 8 },
      { "ftp-user", required_argument, &lopt, 9 },
      { "ftp-passwd", required_argument, &lopt, 10 },
      { "ftp-type", required_argument, &lopt, 11 },
      { "ftp-pasv", no_argument, NULL, 'p' },
      { "ftp-via-http-proxy", required_argument, &lopt, 12 },
      //{ "min-segment-size", required_argument, &lopt, 13 },
      { "http-proxy-method", required_argument, &lopt, 14 },
      { "lowest-speed-limit", required_argument, &lopt, 200 },
      { "max-download-limit", required_argument, &lopt, 201 },
      { "file-allocation", required_argument, 0, 'a' },
      { "allow-overwrite", required_argument, &lopt, 202 },
#ifdef ENABLE_MESSAGE_DIGEST
      { "check-integrity", required_argument, &lopt, 203 },
      { "realtime-chunk-checksum", required_argument, &lopt, 204 },
#endif // ENABLE_MESSAGE_DIGEST
      { "continue", no_argument, 0, 'c' },
      { "user-agent", required_argument, 0, 'U' },
      { "no-netrc", no_argument, 0, 'n' },
      { "input-file", required_argument, 0, 'i' },
      { "max-concurrent-downloads", required_argument, 0, 'j' },
      { "load-cookies", required_argument, &lopt, 205 },
      { "force-sequential", optional_argument, 0, 'Z' },
      { "auto-file-renaming", optional_argument, &lopt, 206 },
      { "parameterized-uri", optional_argument, 0, 'P' },
      { "enable-http-keep-alive", optional_argument, &lopt, 207 },
      { "enable-http-pipelining", optional_argument, &lopt, 208 },
#if defined ENABLE_BITTORRENT || ENABLE_METALINK
      { "show-files", no_argument, NULL, 'S' },
      { "select-file", required_argument, &lopt, 21 },
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
      { "torrent-file", required_argument, NULL, 'T' },
      { "listen-port", required_argument, &lopt, 15 },
      { "follow-torrent", required_argument, &lopt, 16 },
      { "no-preallocation", no_argument, &lopt, 18 },
      { "direct-file-mapping", required_argument, &lopt, 19 },
      // TODO remove upload-limit.
      //{ "upload-limit", required_argument, &lopt, 20 },
      { "seed-time", required_argument, &lopt, 22 },
      { "seed-ratio", required_argument, &lopt, 23 },
      { "max-upload-limit", required_argument, &lopt, 24 },
      { "peer-id-prefix", required_argument, &lopt, 25 },
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      { "metalink-file", required_argument, NULL, 'M' },
      { "metalink-servers", required_argument, NULL, 'C' },
      { "metalink-version", required_argument, &lopt, 100 },
      { "metalink-language", required_argument, &lopt, 101 },
      { "metalink-os", required_argument, &lopt, 102 },
      { "follow-metalink", required_argument, &lopt, 103 },
      { "metalink-location", required_argument, &lopt, 104 },
#endif // ENABLE_METALINK
      { "version", no_argument, NULL, 'v' },
      { "help", no_argument, NULL, 'h' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, "Dd:o:l:s:pt:m:vhST:M:C:a:cU:ni:j:Z::P::", longOpts, &optIndex);
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
    case 'v':
      showVersion();
      exit(EXIT_SUCCESS);
    case 'h':
      showUsage();
      exit(EXIT_SUCCESS);
    default:
      exit(EXIT_FAILURE);
    }
  }

  {
    OptionParser oparser;
    oparser.setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
    string cfname = Util::getHomeDir()+"/.aria2/aria2.conf";
    ifstream cfstream(cfname.c_str());
    try {
      oparser.parse(op, cfstream);
    } catch(Exception* e) {
      cerr << "Parse error in " << cfname << endl;
      cerr << e->getMsg() << endl;
      delete e;
      exit(EXIT_FAILURE);
    }
    try {
      oparser.parse(op, cmdstream);
    } catch(Exception* e) {
      cerr << e->getMsg() << endl;
      delete e;
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
      cerr << MSG_URI_REQUIRED << endl;
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
