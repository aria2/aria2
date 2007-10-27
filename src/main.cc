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
#include "LogFactory.h"
#include "Util.h"
#include "FeatureConfig.h"
#include "MultiUrlRequestInfo.h"
#include "TorrentRequestInfo.h"
#include "BitfieldManFactory.h"
#include "SimpleRandomizer.h"
#include "ConsoleFileAllocationMonitor.h"
#include "Netrc.h"
#include "RequestFactory.h"
#include "OptionParser.h"
#include "OptionHandlerFactory.h"
#include "FatalException.h"
#include "File.h"
#include "CUIDCounter.h"
#include "FileUriListParser.h"
#include "StreamUriListParser.h"
#include "CookieBoxFactory.h"
#include "a2algo.h"
#include "message.h"
#include "a2io.h"
#include "a2time.h"
#include "Platform.h"
#include "prefs.h"
#include "ParameterizedStringParser.h"
#include "PStringBuildVisitor.h"
#include <deque>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <utility>
#include <fstream>
#include <sstream>
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
#ifdef ENABLE_MESSAGE_DIGEST
  cout << "message digest algorithms: " << MessageDigestContext::getSupportedAlgoString() << endl;
#endif // ENABLE_MESSAGE_DIGEST
  cout << endl;
  cout << "Copyright (C) 2006, 2007 Tatsuhiro Tsujikawa" << endl;
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
  cout << _("Contact Info:") << endl;
  cout << "Tatsuhiro Tsujikawa <tujikawa at users dot sourceforge dot net>" << endl;
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
  cout << _(" -d, --dir=DIR                The directory to store the downloaded file.") << endl;
  cout << _(" -o, --out=FILE               The file name of the downloaded file.") << endl;
  cout << _(" -l, --log=LOG                The file name of the log file. If '-' is\n"
	    "                              specified, log is written to stdout.") << endl;
#ifdef HAVE_DAEMON
  cout << _(" -D, --daemon                 Run as daemon.") << endl;
#endif // HAVE_DAEMON
  cout << _(" -s, --split=N                Download a file using N connections. N must be\n"
	    "                              between 1 and 5. This option affects all URLs.\n"
	    "                              Thus, aria2 connects to each URL with\n"
	    "                              N connections.\n"
	    "                              Default: 1") << endl;
  cout << _(" --retry-wait=SEC             Set the seconds to wait to retry after an error\n"
	    "                              has occured. Specify a value between 0 and 60.\n"
	    "                              Default: 5") << endl;
  cout << _(" -t, --timeout=SEC            Set timeout in seconds. Default: 60") << endl;
  cout << _(" -m, --max-tries=N            Set number of tries. 0 means unlimited.\n"
	    "                              Default: 5") << endl;
  /*
  cout << _(" --min-segment-size=SIZE[K|M] Set minimum segment size. You can append\n"
	    "                              K or M(1K = 1024, 1M = 1024K). This\n"
	    "                              value must be greater than or equal to\n"
	    "                              1024. Default: 1M") << endl;
  */
  cout << _(" --http-proxy=HOST:PORT       Use HTTP proxy server. This affects all URLs.") << endl;
  cout << _(" --http-user=USER             Set HTTP user. This affects all URLs.") << endl;
  cout << _(" --http-passwd=PASSWD         Set HTTP password. This affects all URLs.") << endl;
  cout << _(" --http-proxy-user=USER       Set HTTP proxy user. This affects all URLs.") << endl;
  cout << _(" --http-proxy-passwd=PASSWD   Set HTTP proxy password. This affects all URLs.") << endl;
  cout << _(" --http-proxy-method=METHOD   Set the method to use in proxy request.\n"
	    "                              METHOD is either 'get' or 'tunnel'.\n"
	    "                              Default: tunnel") << endl;
  cout << _(" --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic\n"
	    "                              is the only supported scheme.\n"
	    "                              Default: basic") << endl;
  cout << _(" --referer=REFERER            Set Referer. This affects all URLs.") << endl;
  cout << _(" --ftp-user=USER              Set FTP user. This affects all URLs.\n"
	    "                              Default: anonymous") << endl;
  cout << _(" --ftp-passwd=PASSWD          Set FTP password. This affects all URLs.\n"
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
	    "                              0 means aria2 does not have a lowest speed limit.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"

	    "                              This option does not affect BitTorrent downloads.\n"
	    "                              Default: 0") << endl;
  cout << _(" --max-download-limit=SPEED   Set max download speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --file-allocation=METHOD     Specify file allocation method. METHOD is either\n"
	    "                              'none' or 'prealloc'. 'none' doesn't pre-allocate\n"
	    "                              file space. 'prealloc' pre-allocates file space\n"
	    "                              before download begins. This may take some time\n"
	    "                              depending on the size of the file.\n"
	    "                              Default: prealloc") << endl;
  cout << _(" --no-file-allocation-limit=SIZE No file allocation is made for files whose\n"
	    "                              size is smaller than SIZE.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              BitTorrent downloads ignore this option.\n"
	    "                              Default: 5M") << endl;
  cout << _(" --allow-overwrite=true|false If false, aria2 doesn't download a file which\n"
  		"                              already exists but the corresponding .aria2 file\n"
  		"                              doesn't exist.\n"
            "                              Default: false") << endl;
  cout << _(" -Z, --force-sequential[=true|false] Fetch URIs in the command-line sequentially\n"
	    "                              and download each URI in a separate session, like\n"
	    "                              the usual command-line download utilities.\n"
	    "                              Default: false") << endl;
  cout << _(" --auto-file-renaming[=true|false] Rename file name if the same file already\n"
	    "                              exists. This option works only in http(s)/ftp\n"
	    "                              download.\n"
	    "                              The new file name has a dot and a number(1..9999)\n"
	    "                              appended.\n"
	    "                              Default: true") << endl;
  cout << _(" -P, --parameterized-uri[=true|false] Enable parameterized URI support.\n"
	    "                              You can specify set of parts:\n"
	    "                              http://{sv1,sv2,sv3}/foo.iso\n"
	    "                              Also you can specify numeric sequences with step\n"
	    "                              counter:\n"
	    "                              http://host/image[000-100:2].img\n"
	    "                              A step counter can be omitted.\n"
	    "                              If all URIs do not point to the same file, such\n"
	    "                              as the second example above, -Z option is\n"
	    "                              required.\n"
	    "                              Default: false") << endl;
#ifdef ENABLE_MESSAGE_DIGEST
  cout << _(" --check-integrity=true|false  Check file integrity by validating piece hash.\n"
	    "                              This option only affects in BitTorrent downloads\n"
	    "                              and Metalink downloads with chunk checksums.\n"
	    "                              Use this option to re-download a damaged portion\n"
	    "                              of a file.\n"
	    "                              You may need to specify --allow-overwrite=true\n"
	    "                              if the .aria2 file doesn't exist.\n"
	    "                              Default: false") << endl;
  cout << _(" --realtime-chunk-checksum=true|false  Validate chunk checksum while\n"
	    "                              downloading a file in Metalink mode. This option\n"
	    "                              on affects Metalink mode with chunk checksums.\n"
	    "                              Default: true") << endl;
#endif // ENABLE_MESSAGE_DIGEST
  cout << _(" -c, --continue               Continue downloading a partially downloaded\n"
	    "                              file. Use this option to resume a download\n"
	    "                              started by a web browser or another program\n"
	    "                              which downloads files sequentially from the\n"
	    "                              beginning. Currently this option is only\n"
	    "                              applicable to http(s)/ftp downloads.") << endl;
  cout << _(" -U, --user-agent=USER_AGENT  Set user agent for http(s) downloads.") << endl;
  cout << _(" -n, --no-netrc               Disables netrc support.") << endl;
  cout << _(" -i, --input-file=FILE        Downloads URIs found in FILE. You can specify\n"
	    "                              multiple URIs for a single entity: separate\n"
	    "                              URIs on a single line using the TAB character.\n"
	    "                              Reads input from stdin when '-' is specified.") << endl;
  cout << _(" -j, --max-concurrent-downloads=N Set maximum number of concurrent downloads.\n"
	    "                              It should be used with the -i option.\n"
	    "                              Default: 5") << endl;
  cout << _(" --load-cookies=FILE          Load cookies from FILE. The format of FILE is\n"
	    "                              the same used by Netscape and Mozilla.") << endl;
#if defined ENABLE_BITTORRENT || ENABLE_METALINK
  cout << _(" -S, --show-files             Print file listing of .torrent or .metalink file\n"
	    "                              and exit.") << endl;
  cout << _(" --select-file=INDEX...       Set file to download by specifing its index.\n"
	    "                              You can find the file index using the\n"
	    "                              --show-files option. Multiple indexes can be\n"
	    "                              specified by using ',', for example: \"3,6\".\n"
	    "                              You can also use '-' to specify a range: \"1-5\".\n"
	    "                              ',' and '-' can be used together.\n"
	    "                              When used with the -M option, index may vary\n"
	    "                              depending on the query(see --metalink-* options).") << endl;
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
  cout << _(" -T, --torrent-file=TORRENT_FILE  The path to the .torrent file.") << endl;
  cout << _(" --follow-torrent=true|false  Set to false to prevent aria2 from\n"
	    "                              entering BitTorrent mode even if the filename of\n"
	    "                              the downloaded file ends with .torrent.\n"
	    "                              Default: true") << endl;
  cout << _(" --direct-file-mapping=true|false Directly read from and write to each file\n"
	    "                              mentioned in .torrent file.\n"
	    "                              Default: true") << endl;
  cout << _(" --listen-port=PORT           Set TCP port number for BitTorrent downloads.\n"
	    "                              Default: 6881-6999") << endl;
  cout << _(" --max-upload-limit=SPEED     Set max upload speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --seed-time=MINUTES          Specify seeding time in minutes. Also see the\n"
	    "                              --seed-ratio option.") << endl;
  cout << _(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents\n"
	    "                              until share ratio reaches RATIO. 1.0 is\n"
	    "                              encouraged. If --seed-time option is specified\n"
	    "                              along with this option, seeding ends when at\n"
	    "                              least one of the conditions is satisfied.") << endl;
  cout << _(" --peer-id-prefix=PEERI_ID_PREFIX Specify the prefix of peer ID. The peer ID in\n"
	    "                              in BitTorrent is 20 byte length. If more than 20\n"
	    "                              bytes are specified, only first 20\n"
	    "                              bytes are used. If less than 20 bytes are\n"
	    "                              specified, the random alphabet characters are\n"
	    "                              added to make it's length 20 bytes.\n"
	    "                              Default: -aria2-") << endl;
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  cout << _(" -M, --metalink-file=METALINK_FILE The file path to the .metalink file.") << endl;
  cout << _(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n"
	    "                              simultaneously.\n"
	    "                              Default: 5") << endl;
  cout << _(" --metalink-version=VERSION   The version of the file to download.") << endl;
  cout << _(" --metalink-language=LANGUAGE The language of the file to download.") << endl;
  cout << _(" --metalink-os=OS             The operating system of the file to download.") << endl;
  cout << _(" --metalink-location=LOCATION The location of the prefered server.") << endl;
  cout << _(" --follow-metalink=true|false Set to false to prevent aria2 from\n"
	    "                              entering Metalink mode even if the filename of\n"
	    "                              the downloaded file ends with .metalink.\n"
	    "                              Default: true") << endl;
#endif // ENABLE_METALINK
  cout << _(" -v, --version                Print the version number and exit.") << endl;
  cout << _(" -h, --help                   Print this message and exit.") << endl;
  cout << endl;
  cout << "URL:" << endl;
  cout << _(" You can specify multiple URLs. All URLs must point to the same file\n"
	    " or downloading will fail.") << endl;
  cout << endl;
#ifdef ENABLE_BITTORRENT
  cout << "FILE:" << endl;
  cout << _(" Specify files in multi-file torrent to download. Use in conjunction with the\n"
	    " -T option. This argument is ignored if you specify the --select-file option.") << endl;
  cout << endl;
#endif // ENABLE_BITTORRENT
  cout << _("Examples:") << endl;
  cout << _(" Download a file using 1 connection:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file using 2 connections:") << endl;
  cout << "  aria2c -s 2 http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file using 2 connections, each connects to a different server:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip http://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << _(" You can mix up different protocols:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip ftp://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << _(" Parameterized URI:") << endl;
  cout << "  aria2c -P http://{server1,server2,server3}/file.iso" << endl;
  cout << _(" Parameterized URI. -Z option is required in this case:") << endl;
  cout << "  aria2c -P -Z http://host/file[001-100:2].img" << endl;
#ifdef ENABLE_BITTORRENT
  cout << endl;
  cout << _(" Download a torrent:") << endl;
  cout << "  aria2c -o test.torrent http://AAA.BBB.CCC/file.torrent" << endl;
  cout << _(" Download a torrent using a local .torrent file:") << endl;
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
  cout << _(" Download only selected files:") << endl;
  cout << "  aria2c -M test.metalink --metalink-language=en-US dir/file1.zip dir/file2.zip" << endl;
  cout << _(" Download only selected files using index:") << endl;
  cout << "  aria2c -M test.metalink --metalink-language=en-US --select-file 1,3-5" << endl;
  cout << _(" Print file listing of .metalink file:") << endl;
  cout << "  aria2c -M test.metalink -S --metalink-language=en-US" << endl;
#endif // ENABLE_METALINK
  cout << endl;
  printf(_("Report bugs to %s"), "<tujikawa at users dot sourceforge dot net>");
  cout << endl;
}

Strings unfoldURI(const Strings& args)
{
  Strings nargs;
  ParameterizedStringParser p;
  PStringBuildVisitorHandle v = new PStringBuildVisitor();
  for(Strings::const_iterator itr = args.begin(); itr != args.end();
      ++itr) {
    v->reset();
    p.parse(*itr)->accept(v);
    nargs.insert(nargs.end(), v->getURIs().begin(), v->getURIs().end()); 
  }
  return nargs;
}

string toBoolArg(const char* optarg)
{
  string arg;
  if(!optarg || string(optarg) == "") {
    arg = V_TRUE;
  } else {
    arg = optarg;
  }
  return arg;
}

int main(int argc, char* argv[]) {
#ifdef HAVE_WINSOCK2_H
  Platform platform;
#endif // HAVE_WINSOCK2_H

#ifdef ENABLE_NLS
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif // ENABLE_NLS
  stringstream cmdstream;
  int32_t c;
  Option* op = new Option();
  op->put(PREF_STDOUT_LOG, V_FALSE);
  op->put(PREF_DIR, ".");
  op->put(PREF_SPLIT, "1");
  op->put(PREF_DAEMON, V_FALSE);
  op->put(PREF_SEGMENT_SIZE, Util::itos((int32_t)(1024*1024)));
  op->put(PREF_HTTP_KEEP_ALIVE, V_FALSE);
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
  op->put(PREF_DIRECT_DOWNLOAD_TIMEOUT, "15");
  op->put(PREF_FORCE_SEQUENTIAL, V_FALSE);
  op->put(PREF_AUTO_FILE_RENAMING, V_TRUE);
  op->put(PREF_PARAMETERIZED_URI, V_FALSE);
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
      { "no-file-allocation-limit", required_argument, &lopt, 207 },
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
	cmdstream << PREF_NO_FILE_ALLOCATION_LIMIT << "=" << optarg << "\n";
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
    LogFactory::setLogFile(DEV_STDOUT);
  } else if(op->get(PREF_LOG).size()) {
    LogFactory::setLogFile(op->get(PREF_LOG));
  } else {
    LogFactory::setLogFile(DEV_NULL);
  }
  int32_t exitStatus = EXIT_SUCCESS;
  try {
    Logger* logger = LogFactory::getInstance();
    logger->info("%s %s", PACKAGE, PACKAGE_VERSION);
    logger->info(MSG_LOGGING_STARTED);

    RequestFactoryHandle requestFactory = new RequestFactory();
    requestFactory->setOption(op);
    File netrccf(op->get(PREF_NETRC_PATH));
    if(!op->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
      mode_t mode = netrccf.mode();
      if(mode&(S_IRWXG|S_IRWXO)) {
	logger->notice(MSG_INCORRECT_NETRC_PERMISSION,
		       op->get(PREF_NETRC_PATH).c_str());
      } else {
	NetrcHandle netrc = new Netrc();
	netrc->parse(op->get(PREF_NETRC_PATH));
	requestFactory->setNetrc(netrc);
      }
    }

    CookieBoxFactoryHandle cookieBoxFactory = new CookieBoxFactory();
    CookieBoxFactorySingletonHolder::instance(cookieBoxFactory);
    if(op->defined(PREF_LOAD_COOKIES)) {
      File cookieFile(op->get(PREF_LOAD_COOKIES));
      if(cookieFile.isFile()) {
	ifstream in(op->get(PREF_LOAD_COOKIES).c_str());
	CookieBoxFactorySingletonHolder::instance()->loadDefaultCookie(in);
      } else {
	logger->error(MSG_LOADING_COOKIE_FAILED, op->get(PREF_LOAD_COOKIES).c_str());
	exit(EXIT_FAILURE);
      }
    }

    RequestFactorySingletonHolder::instance(requestFactory);
    CUIDCounterHandle cuidCounter = new CUIDCounter();
    CUIDCounterSingletonHolder::instance(cuidCounter);
#ifdef SIGPIPE
    Util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);
#endif

    RequestInfo* firstReqInfo;
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
	Strings targetFiles;
	if(op->defined(PREF_METALINK_FILE) && !args.empty()) {
	  targetFiles = args;
	}
	((MetalinkRequestInfo*)firstReqInfo)->setTargetFiles(targetFiles);
      }
      else
#endif // ENABLE_METALINK
	if(op->defined(PREF_INPUT_FILE)) {
	  SharedHandle<UriListParser> flparser(0);
	  if(op->get(PREF_INPUT_FILE) == "-") {
	    flparser = new StreamUriListParser(cin);
	  } else {
	    if(!File(op->get(PREF_INPUT_FILE)).isFile()) {
	      throw new FatalException(EX_FILE_OPEN, op->get(PREF_INPUT_FILE).c_str(), "No such file");
	    }
	    flparser = new FileUriListParser(op->get(PREF_INPUT_FILE));
	  }
	  RequestGroups groups;
	  while(flparser->hasNext()) {
	    Strings uris = flparser->next();
	    if(uris.size() == 1 && op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
	      Strings unfoldedURIs = unfoldURI(uris);
	      for(Strings::const_iterator itr = unfoldedURIs.begin();
		  itr != unfoldedURIs.end(); ++itr) {
		Strings xuris;
		ncopy(itr, itr+1, op->getAsInt(PREF_SPLIT),
		    back_inserter(xuris));
		RequestGroupHandle rg = new RequestGroup(xuris, op);
		groups.push_back(rg);
	      }
	    } else if(uris.size() > 0) {
	      Strings xuris;
	      ncopy(uris.begin(), uris.end(), op->getAsInt(PREF_SPLIT),
		    back_inserter(xuris));
	      RequestGroupHandle rg = new RequestGroup(xuris, op);
	      groups.push_back(rg);
	    }
	  }
	  firstReqInfo = new MultiUrlRequestInfo(groups, op);
	}
	else
	  {
	    Strings nargs;
	    if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
	      nargs = unfoldURI(args);
	    } else {
	      nargs = args;
	    }
	    if(op->get(PREF_FORCE_SEQUENTIAL) == V_TRUE) {
	      RequestGroups groups;
	      for(Strings::const_iterator itr = nargs.begin();
		  itr != nargs.end(); ++itr) {
		Strings xuris;
		ncopy(itr, itr+1, op->getAsInt(PREF_SPLIT),
		    back_inserter(xuris));
		RequestGroupHandle rg = new RequestGroup(xuris, op);
		groups.push_back(rg);
	      }
	      firstReqInfo = new MultiUrlRequestInfo(groups, op);
	    } else {
	      Strings xargs;
	      ncopy(nargs.begin(), nargs.end(), op->getAsInt(PREF_SPLIT),
		    back_inserter(xargs));
	      firstReqInfo = new MultiUrlRequestInfo(xargs, op);
	    }
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
      /*
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
      */
    }
  } catch(Exception* ex) {
    cerr << EX_EXCEPTION_CAUGHT << "\n" << ex->getMsg() << endl;
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
