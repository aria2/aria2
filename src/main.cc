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
#include "HttpInitiateConnectionCommand.h"
#include "ConsoleDownloadEngine.h"
#include "SegmentMan.h"
#include "LogFactory.h"
#include "common.h"
#include "DefaultDiskWriter.h"
#include "Util.h"
#include "InitiateConnectionCommandFactory.h"
#include "prefs.h"
#include "FeatureConfig.h"
#include "DownloadEngineFactory.h"
#include "UrlRequestInfo.h"
#include "TorrentRequestInfo.h"
#include "BitfieldManFactory.h"
#include "SimpleRandomizer.h"
#include "ConsoleFileAllocationMonitor.h"
#include <deque>
#include <algorithm>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <utility>
extern char* optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

#ifdef ENABLE_METALINK
#include "MetalinkRequestInfo.h"
#include "Xml2MetalinkProcessor.h"
#endif

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/err.h>
# include <openssl/ssl.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

using namespace std;

void showVersion() {
  cout << PACKAGE << _(" version ") << PACKAGE_VERSION << endl;
  cout << "**Configuration**" << endl;
  cout << FeatureConfig::getInstance()->getConfigurationSummary();
  cout << endl;
  cout << "Copyright (C) 2006 Tatsuhiro Tsujikawa" << endl;
  cout << endl;
  cout <<
    _("This program is free software; you can redistribute it and/or modify\n"
      "it under the terms of the GNU General Public License as published by\n"
      "the Free Software Foundation; either version 2 of the License, or\n"
      "(at your option) any later version.\n"
      "\n"
      "This program is distributed in the hope that it will be useful,\n"
      "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
      "GNU General Public License for more details.\n"
      "\n"
      "You should have received a copy of the GNU General Public License\n"
      "along with this program; if not, write to the Free Software\n"
      "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n");
  cout << endl;
  printf(_("Contact Info: %s\n"), "Tatsuhiro Tsujikawa <tujikawa at users dot sourceforge dot net>");
  cout << endl;

}

void showUsage() {
  printf(_("Usage: %s [options] URL ...\n"), PACKAGE_NAME);
#ifdef ENABLE_BITTORRENT
  printf(_("       %s [options] -T TORRENT_FILE FILE ...\n"), PACKAGE_NAME);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  printf(_("       %s [options] -M METALINK_FILE\n"), PACKAGE_NAME);
#endif // ENABLE_METALINK
  cout << endl;
  cout << _("Options:") << endl;
  cout << _(" -d, --dir=DIR                The directory to store downloaded file.") << endl;
  cout << _(" -o, --out=FILE               The file name for downloaded file.") << endl;
  cout << _(" -l, --log=LOG                The file path to store log. If '-' is specified,\n"
	    "                              log is written to stdout.") << endl;
  cout << _(" -D, --daemon                 Run as daemon.") << endl;
  cout << _(" -s, --split=N                Download a file using N connections. N must be\n"
	    "                              between 1 and 5. This option affects all URLs.\n"
	    "                              Thus, aria2 connects to each URL with\n"
	    "                              N connections.\n"
	    "                              Default: 1") << endl;
  cout << _(" --retry-wait=SEC             Set amount of time in second between requests\n"
	    "                              for errors. Specify a value between 0 and 60.\n"
	    "                              Default: 5") << endl;
  cout << _(" -t, --timeout=SEC            Set timeout in second. Default: 60") << endl;
  cout << _(" -m, --max-tries=N            Set number of tries. 0 means unlimited.\n"
	    "                              Default: 5") << endl;
  /*
  cout << _(" --min-segment-size=SIZE[K|M] Set minimum segment size. You can append\n"
	    "                              K or M(1K = 1024, 1M = 1024K). This\n"
	    "                              value must be greater than or equal to\n"
	    "                              1024. Default: 1M") << endl;
  */
  cout << _(" --http-proxy=HOST:PORT       Use HTTP proxy server. This affects to all\n"
	    "                              URLs.") << endl;
  cout << _(" --http-user=USER             Set HTTP user. This affects to all URLs.") << endl;
  cout << _(" --http-passwd=PASSWD         Set HTTP password. This affects to all URLs.") << endl;
  cout << _(" --http-proxy-user=USER       Set HTTP proxy user. This affects to all URLs") << endl;
  cout << _(" --http-proxy-passwd=PASSWD   Set HTTP proxy password. This affects to all URLs.") << endl;
  cout << _(" --http-proxy-method=METHOD   Set the method to use in proxy request.\n"
	    "                              METHOD is either 'get' or 'tunnel'.\n"
	    "                              Default: tunnel") << endl;
  cout << _(" --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic\n"
	    "                              is the only supported scheme.\n"
	    "                              Default: basic") << endl;
  cout << _(" --referer=REFERER            Set Referer. This affects to all URLs.") << endl;
  cout << _(" --ftp-user=USER              Set FTP user. This affects to all URLs.\n"
	    "                              Default: anonymous") << endl;
  cout << _(" --ftp-passwd=PASSWD          Set FTP password. This affects to all URLs.\n"
	    "                              Default: ARIA2USER@") << endl;
  cout << _(" --ftp-type=TYPE              Set FTP transfer type. TYPE is either 'binary'\n"
	    "                              or 'ascii'.\n"
	    "                              Default: binary") << endl;
  cout << _(" -p, --ftp-pasv               Use passive mode in FTP.") << endl;
  cout << _(" --ftp-via-http-proxy=METHOD  Use HTTP proxy in FTP. METHOD is either 'get' or\n"
	    "                              'tunnel'.\n"
	    "                              Default: tunnel") << endl;
  cout << _(" --lowest-speed-limit=SPEED   Close connection if download speed is lower than\n"
	    "                              or equal to this value(bytes per sec).\n"
	    "                              0 means aria2 does not care lowest speed limit.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"

	    "                              This option does not affect BitTorrent download.\n"
	    "                              Default: 0") << endl;
  cout << _(" --max-download-limit=SPEED   Set max download speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --file-allocation=METHOD     Specify file allocation method. METHOD is either\n"
	    "                              'none' or 'prealloc'.\n"
	    "                              'none' doesn't pre-allocate file space. 'prealloc'\n"
	    "                              pre-allocates file space before download begins.\n"
	    "                              This may take some time depending on the size of\n"
	    "                              file.\n"
	    "                              Default: 'none'") << endl;
#ifdef ENABLE_BITTORRENT
  cout << _(" -T, --torrent-file=TORRENT_FILE  The file path to .torrent file.") << endl;
  cout << _(" --follow-torrent=true|false  Setting this option to false prevents aria2 to\n"
	    "                              enter BitTorrent mode even if the filename of\n"
	    "                              downloaded file ends with .torrent.\n"
	    "                              Default: true") << endl;
  cout << _(" -S, --show-files             Print file listing of .torrent file and exit.") << endl;
  cout << _(" --direct-file-mapping=true|false Directly read from and write to each file\n"
	    "                              mentioned in .torrent file.\n"
	    "                              Default: true") << endl;
  cout << _(" --listen-port=PORT           Set port number to listen to for peer connection.") << endl;
  cout << _(" --max-upload-limit=SPEED     Set max upload speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --select-file=INDEX...       Set file to download by specifing its index.\n"
	    "                              You can know file index through --show-files\n"
	    "                              option. Multiple indexes can be specified by using\n"
	    "                              ',' like \"3,6\".\n"
	    "                              You can also use '-' to specify rangelike \"1-5\".\n"
	    "                              ',' and '-' can be used together.") << endl;
  cout << _(" --seed-time=MINUTES          Specify seeding time in minutes. See also\n"
	    "                              --seed-ratio option.") << endl;
  cout << _(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents until\n"
	    "                              share ratio reaches RATIO. 1.0 is encouraged.\n"
	    "                              If --seed-time option is specified along with\n"
	    "                              this option, seeding ends when at least one of\n"
	    "                              the conditions is satisfied.") << endl;
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  cout << _(" -M, --metalink-file=METALINK_FILE The file path to .metalink file.") << endl;
  cout << _(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n"
	    "                              simultaneously. If more than one connection per\n"
	    "                              server is required, use -s option.\n"
	    "                              Default: 15") << endl;
  cout << _(" --metalink-version=VERSION   The version of file to download.") << endl;
  cout << _(" --metalink-language=LANGUAGE The language of file to download.") << endl;
  cout << _(" --metalink-os=OS             The operating system the file is targeted.") << endl;
  cout << _(" --metalink-location=LOCATION The location of the prefered server.") << endl;
  cout << _(" --follow-metalink=true|false  Setting this option to false prevents aria2 to\n"
	    "                              enter Metalink mode even if the filename of\n"
	    "                              downloaded file ends with .metalink.\n"
	    "                              Default: true") << endl;
#endif // ENABLE_METALINK
  cout << _(" -v, --version                Print the version number and exit.") << endl;
  cout << _(" -h, --help                   Print this message and exit.") << endl;
  cout << endl;
  cout << "URL:" << endl;
  cout << _(" You can specify multiple URLs. All URLs must point to the same file\n"
	    " or downloading fails.") << endl;
  cout << endl;
#ifdef ENABLE_BITTORRENT
  cout << "FILE:" << endl;
  cout << _(" Specify files in multi-file torrent to download. Use conjunction with\n"
	    " -T option. This arguments are ignored if you specify --select-file option.") << endl;
  cout << endl;
#endif // ENABLE_BITTORRENT
  cout << _("Examples:") << endl;
  cout << _(" Download a file by 1 connection:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file by 2 connections:") << endl;
  cout << "  aria2c -s 2 http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file by 2 connections, each connects to a different server:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip http://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << _(" You can mix up different protocols:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip ftp://DDD.EEE.FFF/GGG/file.zip" << endl;
#ifdef ENABLE_BITTORRENT
  cout << endl;
  cout << _(" Download a torrent:") << endl;
  cout << "  aria2c -o test.torrent http://AAA.BBB.CCC/file.torrent" << endl;
  cout << _(" Download a torrent using local .torrent file:") << endl;
  cout << "  aria2c -T test.torrent" << endl;
  cout << _(" Download only selected files:") << endl;
  cout << "  aria2c -T test.torrent dir/file1.zip dir/file2.zip" << endl;
  cout << _(" Print file listing of .torrent file:") << endl;
  cout << "  aria2c -T test.torrent -S" << endl;  
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  cout << endl;
  cout << _(" Metalink downloading:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.metalink" << endl;
  cout << _(" Download a file using local .metalink file:") << endl;
  cout << "  aria2c -M test.metalink" << endl;
  cout << _(" Metalink downloading with preferences:") << endl;
  cout << "  aria2c -M test.metalink --metalink-version=1.1.1 --metalink-language=en-US" << endl;
#endif // ENABLE_METALINK
  cout << endl;
  printf(_("Report bugs to %s"), "<tujikawa at users dot sourceforge dot net>");
  cout << endl;
}

long long int getRealSize(char* optarg) {
  string::size_type p = string(optarg).find_first_of("KM");
  int mult = 1;
  if(p != string::npos) {
    if(optarg[p] == 'K') {
      mult = 1024;
    } else if(optarg[p] == 'M') {
      mult = 1024*1024;
    }
    optarg[p] = '\0';
  }
  long long int size = strtoll(optarg, NULL, 10)*mult;
  return size;
}

int main(int argc, char* argv[]) {
#ifdef ENABLE_NLS
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif // ENABLE_NLS

  int c;
  Option* op = new Option();
  op->put(PREF_STDOUT_LOG, V_FALSE);
  op->put(PREF_DIR, ".");
  op->put(PREF_SPLIT, "1");
  op->put(PREF_DAEMON, V_FALSE);
  op->put(PREF_SEGMENT_SIZE, Util::itos(1024*1024));
  op->put(PREF_HTTP_KEEP_ALIVE, V_FALSE);
  op->put(PREF_LISTEN_PORT, "-1");
  op->put(PREF_METALINK_SERVERS, "15");
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
  op->put(PREF_FTP_USER, "anonymous");
  op->put(PREF_FTP_PASSWD, "ARIA2USER@");
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
  while(1) {
    int optIndex = 0;
    int lopt;
    static struct option longOpts[] = {
      { "daemon", no_argument, NULL, 'D' },
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
#ifdef ENABLE_BITTORRENT
      { "torrent-file", required_argument, NULL, 'T' },
      { "listen-port", required_argument, &lopt, 15 },
      { "follow-torrent", required_argument, &lopt, 16 },
      { "show-files", no_argument, NULL, 'S' },
      { "no-preallocation", no_argument, &lopt, 18 },
      { "direct-file-mapping", required_argument, &lopt, 19 },
      // TODO remove upload-limit.
      { "upload-limit", required_argument, &lopt, 20 },
      { "select-file", required_argument, &lopt, 21 },
      { "seed-time", required_argument, &lopt, 22 },
      { "seed-ratio", required_argument, &lopt, 23 },
      { "max-upload-limit", required_argument, &lopt, 24 },
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
    c = getopt_long(argc, argv, "Dd:o:l:s:pt:m:vhST:M:C:a:", longOpts, &optIndex);
    if(c == -1) {
      break;
    }
    switch(c) {
    case 0:{
      switch(lopt) {
      case 1: {
	pair<string, string> proxy;
	Util::split(proxy, optarg, ':');
	int port = (int)strtol(proxy.second.c_str(), NULL, 10);
	if(proxy.first.empty() || proxy.second.empty() ||
	   !(0 < port && port <= 65535)) {
	  cerr << _("unrecognized proxy format") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_HTTP_PROXY_HOST, proxy.first);
	op->put(PREF_HTTP_PROXY_PORT, Util::itos(port));
	op->put(PREF_HTTP_PROXY_ENABLED, V_TRUE);
	break;
      }
      case 2:
	op->put(PREF_HTTP_USER, optarg);
	op->put(PREF_HTTP_AUTH_ENABLED, V_TRUE);
	break;
      case 3:
	op->put(PREF_HTTP_PASSWD, optarg);
	break;
      case 4:
	op->put(PREF_HTTP_PROXY_USER, optarg);
	op->put(PREF_HTTP_PROXY_AUTH_ENABLED, V_TRUE);
	break;
      case 5: 
	op->put(PREF_HTTP_PROXY_PASSWD, optarg);
	break;
      case 6:
	if(string(V_BASIC) == optarg) {
	  op->put(PREF_HTTP_AUTH_SCHEME, V_BASIC);
	} else {
	  cerr << _("Currently, supported authentication scheme is basic.") << endl;
	}
	break;
      case 7:
	op->put(PREF_REFERER, optarg);
	break;
      case 8: {
	int wait = (int)strtol(optarg, NULL, 10);
	if(!(0 <= wait && wait <= 60)) {
	  cerr << _("retry-wait must be between 0 and 60.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_RETRY_WAIT, Util::itos(wait));
	break;
      }
      case 9:
	op->put(PREF_FTP_USER, optarg);
	break;
      case 10:
	op->put(PREF_FTP_PASSWD, optarg);
	break;
      case 11:
	if(string(optarg) == V_BINARY || string(optarg) == V_ASCII) {
	  op->put(PREF_FTP_TYPE, optarg);
	} else {
	  cerr << _("ftp-type must be either 'binary' or 'ascii'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 12:
	if(string(optarg) == V_GET || string(optarg) == V_TUNNEL) {
	  op->put(PREF_FTP_VIA_HTTP_PROXY, optarg);
	} else {
	  cerr << _("ftp-via-http-proxy must be either 'get' or 'tunnel'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 13: {
	long long int size = getRealSize(optarg);
	if(size < 1024) {
	  cerr << _("min-segment-size invalid") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_MIN_SEGMENT_SIZE, Util::llitos(size));
	break;
      }
      case 14:
	if(string(optarg) == V_GET || string(optarg) == V_TUNNEL) {
	  op->put(PREF_HTTP_PROXY_METHOD, optarg);
	} else {
	  cerr << _("http-proxy-method must be either 'get' or 'tunnel'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 15: {
	int listenPort = (int)strtol(optarg, NULL, 10);
	if(!(1024 <= listenPort && listenPort <= 65535)) {
	  cerr << _("listen-port must be between 1024 and 65535.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_LISTEN_PORT, Util::itos(listenPort));
	break;
      }
      case 16:
	if(string(optarg) == "true") {
	  op->put(PREF_FOLLOW_TORRENT, V_TRUE);
	} else if(string(optarg) == "false") {
	  op->put(PREF_FOLLOW_TORRENT, V_FALSE);
	} else {
	  cerr << _("follow-torrent must be either 'true' or 'false'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 18:
	op->put(PREF_NO_PREALLOCATION, V_TRUE);
	break;
      case 19:
	if(string(optarg) == "true") {
	  op->put(PREF_DIRECT_FILE_MAPPING, V_TRUE);
	} else if(string(optarg) == "false") {
	  op->put(PREF_DIRECT_FILE_MAPPING, V_FALSE);
	} else {
	  cerr << _("direct-file-mapping must be either 'true' or 'false'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 20: {
	cerr << "Warning: upload-limit will be deprecated in the future release.\n"
	  "Use max-upload-limit instead. Because there is a difference between them,\n"
	  "take a look at the description of max-upload-limit option." << endl;
	int uploadSpeed = strtol(optarg, NULL, 10)*1024;
	if(0 > uploadSpeed) {
	  cerr << _("upload-limit must be greater than or equal to 0.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_MAX_UPLOAD_LIMIT, Util::itos(uploadSpeed));
	break;
      }
      case 21:
	op->put(PREF_SELECT_FILE, optarg);
	break;
      case 22: {
	int seedTime = (int)strtol(optarg, NULL, 10);
	if(seedTime < 0) {
	  cerr << _("seed-time must be greater than or equal to 0.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_SEED_TIME, Util::itos(seedTime));
	break;
      }
      case 23: {
	double ratio = (int)strtod(optarg, NULL);
	if(ratio < 0.0) {
	  cerr << _("seed-ratio must be greater than or equal to 0.0.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_SEED_RATIO, optarg);
	break;
      }
      case 24: {
	int limit = getRealSize(optarg);
	if(limit < 0) {
	  cerr << _("max-upload-limit must be greater than or equal to 0") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_MAX_UPLOAD_LIMIT, Util::itos(limit));
	break;
      }
      case 100:
	op->put(PREF_METALINK_VERSION, optarg);
	break;
      case 101:
	op->put(PREF_METALINK_LANGUAGE, optarg);
	break;
      case 102:
	op->put(PREF_METALINK_OS, optarg);
	break;
      case 103:
	if(string(optarg) == "true") {
	  op->put(PREF_FOLLOW_METALINK, V_TRUE);
	} else if(string(optarg) == "false") {
	  op->put(PREF_FOLLOW_METALINK, V_FALSE);
	} else {
	  cerr << _("follow-metalink must be either 'true' or 'false'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 104:
	op->put(PREF_METALINK_LOCATION, optarg);
	break;
      case 200: {
	int limit = getRealSize(optarg);
	if(limit < 0) {
	  cerr << _("lowest-speed-limit must be greater than or equal to 0") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_LOWEST_SPEED_LIMIT, Util::itos(limit));
	break;
      }
      case 201: {
	int limit = getRealSize(optarg);
	if(limit < 0) {
	  cerr << _("max-download-limit must be greater than or equal to 0") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_MAX_DOWNLOAD_LIMIT, Util::itos(limit));
	break;
      }
      }
      break;
    }
    case 'D':
      op->put(PREF_DAEMON, V_TRUE);
      break;
    case 'd':
      op->put(PREF_DIR, optarg);
      break;
    case 'o':
      op->put(PREF_OUT, optarg);
      break;
    case 'l':
      if(strcmp("-", optarg) == 0) {
	op->put(PREF_STDOUT_LOG, V_TRUE);
      } else {
	op->put(PREF_LOG, optarg);
      }
      break;
    case 's': {
      int split = (int)strtol(optarg, NULL, 10);
      if(!(1 <= split && split <= 5)) {
	cerr << _("split must be between 1 and 5.") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      op->put(PREF_SPLIT, Util::itos(split));
      break;
    }
    case 't': {
      int timeout = (int)strtol(optarg, NULL, 10);
      if(1 <= timeout && timeout <= 600) {
	op->put(PREF_TIMEOUT, Util::itos(timeout));
      } else {
	cerr << _("timeout must be between 1 and 600") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      break;
    }
    case 'm': {
      int retries = (int)strtol(optarg, NULL, 10);
      if(retries < 0) {
	cerr << _("max-tries invalid") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      op->put(PREF_MAX_TRIES, Util::itos(retries));
      break;
    }
    case 'p':
      op->put(PREF_FTP_PASV_ENABLED, V_TRUE);
      break;
    case 'S':
      op->put(PREF_SHOW_FILES, V_TRUE);
      break;
    case 'T':
      op->put(PREF_TORRENT_FILE, optarg);
      break;
    case 'M':
      op->put(PREF_METALINK_FILE, optarg);
      break;
    case 'C': {
      int metalinkServers = (int)strtol(optarg, NULL, 10);
      if(metalinkServers <= 0) {
	cerr << _("metalink-servers must be greater than 0.") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      op->put(PREF_METALINK_SERVERS, Util::itos(metalinkServers));
      break;
    }
    case 'a': {
      string value = string(optarg);
      if(value == V_NONE || value == V_PREALLOC) {
	op->put(PREF_FILE_ALLOCATION, value);
      } else {
	cerr << _("file-allocation must be either 'none' or 'prealloc'.") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      break;
    }
    case 'v':
      showVersion();
      exit(EXIT_SUCCESS);
    case 'h':
      showUsage();
      exit(EXIT_SUCCESS);
    default:
      showUsage();
      exit(EXIT_FAILURE);
    }
  }
  if(!op->defined(PREF_TORRENT_FILE) && !op->defined(PREF_METALINK_FILE)) {
    if(optind == argc) {
      cerr << _("specify at least one URL") << endl;
      showUsage();
      exit(EXIT_FAILURE);
    }
  }
  if(op->getAsBool(PREF_DAEMON)) {
    if(daemon(1, 1) < 0) {
      perror(_("daemon failed"));
      exit(EXIT_FAILURE);
    }
  }
  Strings args(argv+optind, argv+argc);
  
#ifdef HAVE_LIBSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  gnutls_global_init();
#endif // HAVE_LIBGNUTLS
#ifdef ENABLE_METALINK
  xmlInitParser();
#endif // ENABLE_METALINK
  SimpleRandomizer::init();
  BitfieldManFactory::setDefaultRandomizer(SimpleRandomizer::getInstance());
  FileAllocationMonitorFactory::setFactory(new ConsoleFileAllocationMonitorFactory());
  if(op->getAsBool(PREF_STDOUT_LOG)) {
    LogFactory::setLogFile("/dev/stdout");
  } else if(op->get(PREF_LOG).size()) {
    LogFactory::setLogFile(op->get(PREF_LOG));
  } else {
    LogFactory::setLogFile("/dev/null");
  }
  int exitStatus = EXIT_SUCCESS;
  try {
    Logger* logger = LogFactory::getInstance();
    logger->info("%s %s", PACKAGE, PACKAGE_VERSION);
    logger->info("Logging started.");

    Util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);

    RequestInfo* firstReqInfo = 0;
#ifdef ENABLE_BITTORRENT
    if(op->defined(PREF_TORRENT_FILE)) {
      firstReqInfo = new TorrentRequestInfo(op->get(PREF_TORRENT_FILE),
					   op);
      Strings targetFiles;
      if(op->defined(PREF_TORRENT_FILE) && !args.empty()) {
	targetFiles = args;
      }
      ((TorrentRequestInfo*)firstReqInfo)->setTargetFiles(targetFiles);
    }
    else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if(op->defined(PREF_METALINK_FILE)) {
	firstReqInfo = new MetalinkRequestInfo(op->get(PREF_METALINK_FILE),
					      op);
      } else
#endif // ENABLE_METALINK
	{
	  firstReqInfo = new UrlRequestInfo(args, 0, op);
	}

    RequestInfos reqInfos;
    if(firstReqInfo) {
      reqInfos.push_front(firstReqInfo);
    }
    while(reqInfos.size()) {
      RequestInfoHandle reqInfo = reqInfos.front();
      reqInfos.pop_front();
      RequestInfos nextReqInfos = reqInfo->execute();
      copy(nextReqInfos.begin(), nextReqInfos.end(), front_inserter(reqInfos));
      if(reqInfo->isFail()) {
	exitStatus = EXIT_FAILURE;
      } else if(reqInfo->getFileInfo().checkReady()) {
	cout << _("Now verifying checksum.\n"
		  "This may take some time depending on your PC environment"
		  " and the size of file.") << endl;
	if(reqInfo->getFileInfo().check()) {
	  cout << _("checksum OK.") << endl;
	} else {
	  // TODO
	  cout << _("checksum ERROR.") << endl;
	  exitStatus = EXIT_FAILURE;
	}
      }
    }
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    delete ex;
    exit(EXIT_FAILURE);
  }
  delete op;
  LogFactory::release();
#ifdef HAVE_LIBGNUTLS
  gnutls_global_deinit();
#endif // HAVE_LIBGNUTLS
#ifdef ENABLE_METALINK
  xmlCleanupParser();
#endif // ENABLE_METALINK
  FeatureConfig::release();
  return exitStatus;
}
