/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "HttpInitiateConnectionCommand.h"
#include "ConsoleDownloadEngine.h"
#include "SegmentMan.h"
#include "SplitSlowestSegmentSplitter.h"
#include "LogFactory.h"
#include "common.h"
#include "DefaultDiskWriter.h"
#include "Util.h"
#include "InitiateConnectionCommandFactory.h"
#include "prefs.h"
#include "FeatureConfig.h"

#ifdef ENABLE_BITTORRENT
# include "TorrentConsoleDownloadEngine.h"
# include "TorrentMan.h"
# include "PeerListenCommand.h"
# include "TorrentAutoSaveCommand.h"
# include "TrackerWatcherCommand.h"
# include "TrackerUpdateCommand.h"
# include "HaveEraseCommand.h"
# include "ByteArrayDiskWriter.h"
# include "PeerChokeCommand.h"
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
# include "Xml2MetalinkProcessor.h"
#endif // ENABLE_METALINK

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

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/err.h>
# include <openssl/ssl.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

using namespace std;

bool readyToTorrentMode = false;
string downloadedTorrentFile;
bool readyToMetalinkMode = false;
string downloadedMetalinkFile;

void printDownloadCompeleteMessage(string filename) {
  printf(_("\nThe download was complete. <%s>\n"), filename.c_str());
}

void printDownloadCompeleteMessage() {
  printf("\nThe download was complete.\n");
}

void printDownloadAbortMessage() {
  printf(_("\nThe download was not complete because of errors. Check the log.\n"));
}

void setSignalHander(int signal, void (*handler)(int), int flags) {
  struct sigaction sigact;
  sigact.sa_handler = handler;
  sigact.sa_flags = flags;
  sigemptyset(&sigact.sa_mask);
  sigaction(signal, &sigact, NULL);
}

DownloadEngine* e;
#ifdef ENABLE_BITTORRENT
TorrentDownloadEngine* te;
#endif // ENABLE_BITTORRENT

void handler(int signal) {
  printf(_("\nstopping application...\n"));
  fflush(stdout);
  e->segmentMan->save();
  e->segmentMan->diskWriter->closeFile();
  e->cleanQueue();
  delete e;
  printf(_("done\n"));
  exit(EXIT_SUCCESS);
}

#ifdef ENABLE_BITTORRENT
void torrentHandler(int signal) {
  te->torrentMan->setHalt(true);
}
#endif // ENABLE_BITTORRENT

void createRequest(int cuid, const string& url, string referer, Requests& requests) {
  Request* req = new Request();
  req->setReferer(referer);
  if(req->setUrl(url)) {
    requests.push_back(req);
  } else {
    fprintf(stderr, _("Unrecognized URL or unsupported protocol: %s\n"), req->getUrl().c_str());
    delete(req);
  }
}

void showVersion() {
  cout << PACKAGE << _(" version ") << PACKAGE_VERSION << endl;
  cout << "Copyright (C) 2006 Tatsuhiro Tsujikawa" << endl;
  cout << endl;
  cout << "**Configuration**" << endl;
  cout << FeatureConfig::getConfigurationSummary();
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
      "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n");
  cout << endl;
  printf(_("Contact Info: %s\n"), "Tasuhiro Tsujikawa <tujikawa at users dot sourceforge dot net>");
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
	    "                              N connections.") << endl;
  cout << _(" --retry-wait=SEC             Set amount of time in second between requests\n"
	    "                              for errors. Specify a value between 0 and 60.\n"
	    "                              Default: 5") << endl;
  cout << _(" -t, --timeout=SEC            Set timeout in second. Default: 60") << endl;
  cout << _(" -m, --max-tries=N            Set number of tries. 0 means unlimited.\n"
	    "                              Default: 5") << endl;
  cout << _(" --min-segment-size=SIZE[K|M] Set minimum segment size. You can append\n"
	    "                              K or M(1K = 1024, 1M = 1024K). This\n"
	    "                              value must be greater than or equal to\n"
	    "                              1024.") << endl;
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
  cout << _(" --upload-limit=SPEED         Set upload speed limit in KB/sec. aria2 tries to\n"
	    "                              keep upload speed under SPEED. 0 means unlimited.") << endl;
  cout << _(" --select-file=INDEX...       Set file to download by specifing its index.\n"
	    "                              You can know file index through --show-files\n"
	    "                              option. Multiple indexes can be specified by using\n"
	    "                              ',' like \"3,6\".\n"
	    "                              You can also use '-' to specify rangelike \"1-5\".\n"
	    "                              ',' and '-' can be used together.") << endl;
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
  cout << "  aria2c -t 180 -o test.torrent http://AAA.BBB.CCC/file.torrent" << endl;
  cout << _(" Download a torrent using local .torrent file:") << endl;
  cout << "  aria2c -t 180 -T test.torrent" << endl;
  cout << _(" Download only selected files:") << endl;
  cout << "  aria2c -t 180 -T test.torrent dir/file1.zip dir/file2.zip" << endl;
  cout << _(" Print file listing of .torrent file:") << endl;
  cout << "  aria2c -t 180 -T test.torrent -S" << endl;  
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

bool normalDownload(const Requests& requests,
		    const Requests& reserved,
		    Option* op,
		    const string& dir,
		    const string& ufilename,
		    string& downloadedFilename) {
  setSignalHander(SIGINT, handler, 0);
  setSignalHander(SIGTERM, handler, 0);
  
  e = new ConsoleDownloadEngine();
  e->option = op;
  e->segmentMan = new SegmentMan();
  e->segmentMan->diskWriter = new DefaultDiskWriter();
  e->segmentMan->dir = dir;
  e->segmentMan->ufilename = ufilename;
  e->segmentMan->option = op;
  e->segmentMan->splitter = new SplitSlowestSegmentSplitter();
  e->segmentMan->splitter->setMinSegmentSize(op->getAsLLInt(PREF_MIN_SEGMENT_SIZE));
  e->segmentMan->reserved = reserved;
  
  int cuidCounter = 1;
  for(Requests::const_iterator itr = requests.begin();
      itr != requests.end();
      itr++, cuidCounter++) {
    e->commands.push_back(InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuidCounter, *itr, e));
  }
  e->run();
  bool success = false;
  if(e->segmentMan->finished()) {
    printDownloadCompeleteMessage(e->segmentMan->getFilePath());
    if(Util::endsWith(e->segmentMan->getFilePath(), ".torrent")) {
      downloadedTorrentFile = e->segmentMan->getFilePath();
      readyToTorrentMode = true;
    } else if(Util::endsWith(e->segmentMan->getFilePath(), ".metalink")) {
      downloadedMetalinkFile = e->segmentMan->getFilePath();
      readyToMetalinkMode = true;
    }
    downloadedFilename = e->segmentMan->getFilePath();
    success = true;
  } else {
    e->segmentMan->save();
    e->segmentMan->diskWriter->closeFile();
    printDownloadAbortMessage();
  }
  e->cleanQueue();
  delete e;

  return success;
}

int main(int argc, char* argv[]) {
#ifdef ENABLE_NLS
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif // ENABLE_NLS
  bool stdoutLog = false;
  string logfile;
  string dir = ".";
  string ufilename;
  int split = 1;
  bool daemonMode = false;
  string referer;
  string torrentFile;
  string metalinkFile;
  int listenPort = -1;
  string metalinkVersion;
  string metalinkLanguage;
  string metalinkOs;
  int metalinkServers = 15;
  Integers selectFileIndexes;
#ifdef ENABLE_BITTORRENT
  bool followTorrent = true;
#else
  bool followTorrent = false;
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  bool followMetalink = true;
#else
  bool followMetalink = false;
#endif // ENABLE_METALINK

  int c;
  Option* op = new Option();
  op->put(PREF_RETRY_WAIT, "5");
  op->put(PREF_TIMEOUT, "60");
  op->put(PREF_PEER_CONNECTION_TIMEOUT, "60");
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
  op->put(PREF_UPLOAD_LIMIT, "0");
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
      { "min-segment-size", required_argument, &lopt, 13 },
      { "http-proxy-method", required_argument, &lopt, 14 },
#ifdef ENABLE_BITTORRENT
      { "torrent-file", required_argument, NULL, 'T' },
      { "listen-port", required_argument, &lopt, 15 },
      { "follow-torrent", required_argument, &lopt, 16 },
      { "show-files", no_argument, NULL, 'S' },
      { "no-preallocation", no_argument, &lopt, 18 },
      { "direct-file-mapping", required_argument, &lopt, 19 },
      { "upload-limit", required_argument, &lopt, 20 },
      { "select-file", required_argument, &lopt, 21 },
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      { "metalink-file", required_argument, NULL, 'M' },
      { "metalink-servers", required_argument, NULL, 'C' },
      { "metalink-version", required_argument, &lopt, 100 },
      { "metalink-language", required_argument, &lopt, 101 },
      { "metalink-os", required_argument, &lopt, 102 },
      { "follow-metalink", required_argument, &lopt, 103 },
#endif // ENABLE_METALINK
      { "version", no_argument, NULL, 'v' },
      { "help", no_argument, NULL, 'h' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, "Dd:o:l:s:pt:m:vhST:M:C:", longOpts, &optIndex);
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
	referer = optarg;
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
      case 15:
	listenPort = (int)strtol(optarg, NULL, 10);
	if(!(1024 <= listenPort && listenPort <= 65535)) {
	  cerr << _("listen-port must be between 1024 and 65535.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      case 16:
	if(string(optarg) == "true") {
	  followTorrent = true;
	} else if(string(optarg) == "false") {
	  followTorrent = false;
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
	int uploadSpeed = (int)strtol(optarg, NULL, 10);
	if(0 > uploadSpeed) {
	  cerr << _("upload-limit must be greater than or equal to 0.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	op->put(PREF_UPLOAD_LIMIT, Util::itos(uploadSpeed));
	break;
      }
      case 21:
	Util::unfoldRange(optarg, selectFileIndexes);
	break;
      case 100:
	metalinkVersion = string(optarg);
	break;
      case 101:
	metalinkLanguage = string(optarg);
	break;
      case 102:
	metalinkOs = string(optarg);
	break;
      case 103:
	if(string(optarg) == "true") {
	  followMetalink = true;
	} else if(string(optarg) == "false") {
	  followMetalink = false;
	} else {
	  cerr << _("follow-metalink must be either 'true' or 'false'.") << endl;
	  showUsage();
	  exit(EXIT_FAILURE);
	}
	break;
      }
      break;
    }
    case 'D':
      daemonMode = true;
      break;
    case 'd':
      dir = optarg;
      break;
    case 'o':
      ufilename = optarg;
      break;
    case 'l':
      if(strcmp("-", optarg) == 0) {
	stdoutLog = true;
      } else {
	logfile = optarg;
      }
      break;
    case 's':
      split = (int)strtol(optarg, NULL, 10);
      if(!(1 <= split && split <= 5)) {
	cerr << _("split must be between 1 and 5.") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      break;
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
      torrentFile = string(optarg);
      break;
    case 'M':
      metalinkFile = string(optarg);
      break;
    case 'C':
      metalinkServers = (int)strtol(optarg, NULL, 10);
      if(metalinkServers <= 0) {
	cerr << _("metalink-servers must be greater than 0.") << endl;
	showUsage();
	exit(EXIT_FAILURE);
      }
      break;      
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
  if(torrentFile.empty() && metalinkFile.empty()) {
    if(optind == argc) {
      cerr << _("specify at least one URL") << endl;
      showUsage();
      exit(EXIT_FAILURE);
    }
  }
  if(daemonMode) {
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
  srandom(time(NULL));
  if(stdoutLog) {
    LogFactory::setLogFile("/dev/stdout");
  } else if(logfile.size()) {
    LogFactory::setLogFile(logfile);
  }
  // make sure logger is configured properly.
  try {
    LogFactory::getInstance();
  } catch(Exception* ex) {
    cerr << ex->getMsg() << endl;
    delete ex;
    exit(EXIT_FAILURE);
  }

  setSignalHander(SIGPIPE, SIG_IGN, 0);

  if(torrentFile.empty() && metalinkFile.empty()) {
    Requests requests;
    int cuidCounter = 1;
    for(Strings::const_iterator itr = args.begin(); itr != args.end(); itr++) {
      for(int s = 1; s <= split; s++) {
	createRequest(cuidCounter, *itr, referer, requests); 
	cuidCounter++;
      }
    }
    setSignalHander(SIGINT, handler, 0);
    setSignalHander(SIGTERM, handler, 0);

    Requests reserved;
    string downloadedFilename;
    normalDownload(requests, reserved, op, dir, ufilename, downloadedFilename);

    for_each(requests.begin(), requests.end(), Deleter());
    for_each(reserved.begin(), reserved.end(), Deleter());
    requests.clear();
  }
#ifdef ENABLE_METALINK
  if(!metalinkFile.empty() || followMetalink && readyToMetalinkMode) {
    string targetMetalinkFile = metalinkFile.empty() ?
      downloadedMetalinkFile : metalinkFile;
    Xml2MetalinkProcessor proc;
    Metalinker* metalinker = proc.parseFile(targetMetalinkFile);
    
    MetalinkEntry* entry = metalinker->queryEntry(metalinkVersion,
						  metalinkLanguage,
						  metalinkOs);
    if(entry == NULL) {
      printf("No file matched with your preference.\n");
      exit(EXIT_FAILURE);
    }
    entry->dropUnsupportedResource();
    entry->reorderResourcesByPreference();
    Requests requests;
    int cuidCounter = 1;
    for(MetalinkResources::const_iterator itr = entry->resources.begin();
	itr != entry->resources.end(); itr++) {
      MetalinkResource* resource = *itr;
      for(int s = 1; s <= split; s++) {
	createRequest(cuidCounter, resource->url, referer, requests); 
	cuidCounter++;
      }
    }
    Requests reserved;
    int maxConnection = metalinkServers*split;
    if((int)requests.size() > maxConnection) {
      copy(requests.begin()+maxConnection, requests.end(),
	   insert_iterator<Requests>(reserved, reserved.end()));
      requests.erase(requests.begin()+maxConnection, requests.end());
    }

    setSignalHander(SIGINT, handler, 0);
    setSignalHander(SIGTERM, handler, 0);

    string downloadedFilename;
    bool success = normalDownload(requests, reserved, op, dir, ufilename,
				  downloadedFilename);

    for_each(requests.begin(), requests.end(), Deleter());
    for_each(reserved.begin(), reserved.end(), Deleter());
    requests.clear();

    if(success) {
#ifdef ENABLE_MESSAGE_DIGEST
      if(entry->check(downloadedFilename)) {
	printf("checksum OK.\n");
      } else {
	printf("checksum ERROR.\n");
	exit(EXIT_FAILURE);
      }
#endif // ENABLE_MESSAGE_DIGEST
    }

    delete metalinker;
  }
#endif // ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
  if(!torrentFile.empty() || followTorrent && readyToTorrentMode) {
    try {
      //op->put(PREF_MAX_TRIES, "0");
      setSignalHander(SIGINT, torrentHandler, SA_RESETHAND);
      setSignalHander(SIGTERM, torrentHandler, SA_RESETHAND);

      Request* req = new Request();
      req->isTorrent = true;
      req->setTrackerEvent(Request::STARTED);
      te = new TorrentConsoleDownloadEngine();
      te->option = op;
      ByteArrayDiskWriter* byteArrayDiskWriter = new ByteArrayDiskWriter();
      te->segmentMan = new SegmentMan();
      te->segmentMan->diskWriter = byteArrayDiskWriter;
      te->segmentMan->option = op;
      te->segmentMan->splitter = new SplitSlowestSegmentSplitter();
      te->segmentMan->splitter->setMinSegmentSize(op->getAsLLInt(PREF_MIN_SEGMENT_SIZE));
      te->torrentMan = new TorrentMan();
      te->torrentMan->setStoreDir(dir);
      te->torrentMan->option = op;
      te->torrentMan->req = req;
      string targetTorrentFile = torrentFile.empty() ?
	downloadedTorrentFile : torrentFile;
      if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	FileEntries fileEntries =
	  te->torrentMan->readFileEntryFromMetaInfoFile(targetTorrentFile);
	cout << _("Files:") << endl;
	cout << "idx|path/length" << endl;
	cout << "===+===========================================================================" << endl;
	int count = 1;
	for(FileEntries::const_iterator itr = fileEntries.begin();
	    itr != fileEntries.end(); count++, itr++) {
	  printf("%3d|%s\n   |%s Bytes\n", count, itr->path.c_str(),
		 Util::llitos(itr->length, true).c_str());
	  cout << "---+---------------------------------------------------------------------------" << endl;
	}
	exit(EXIT_SUCCESS);
      } else {
	if(selectFileIndexes.empty()) {
	  Strings targetFiles;
	  if(!torrentFile.empty() && !args.empty()) {
	    targetFiles = args;
	  }
	  te->torrentMan->setup(targetTorrentFile, targetFiles);
	} else {
	  te->torrentMan->setup(targetTorrentFile, selectFileIndexes);
	}
      }
      PeerListenCommand* listenCommand =
	new PeerListenCommand(te->torrentMan->getNewCuid(), te);
      int port;
      if(listenPort == -1) {
	port = listenCommand->bindPort(6881, 6999);
      } else {
	port = listenCommand->bindPort(listenPort, listenPort);
      }
      if(port == -1) {
	printf(_("Errors occurred while binding port.\n"));
	exit(EXIT_FAILURE);
      }
      te->torrentMan->setPort(port);
      te->commands.push_back(listenCommand);

      te->commands.push_back(new TrackerWatcherCommand(te->torrentMan->getNewCuid(),
						       te,
						       te->torrentMan->minInterval));
      te->commands.push_back(new TrackerUpdateCommand(te->torrentMan->getNewCuid(),
						      te));
      te->commands.push_back(new TorrentAutoSaveCommand(te->torrentMan->getNewCuid(),
							te,
							op->getAsInt(PREF_AUTO_SAVE_INTERVAL)));
      te->commands.push_back(new PeerChokeCommand(te->torrentMan->getNewCuid(),
						  10, te));
      te->commands.push_back(new HaveEraseCommand(te->torrentMan->getNewCuid(),
						  te, 10));
      te->run();
      
      if(te->torrentMan->downloadComplete()) {
	printDownloadCompeleteMessage();
      }
      delete req;
      te->cleanQueue();
      delete te;
    } catch(Exception* ex) {
      cerr << ex->getMsg() << endl;
      delete ex;
      exit(EXIT_FAILURE);
    }
  }
#endif // ENABLE_BITTORRENT
  delete op;
  LogFactory::release();
#ifdef HAVE_LIBGNUTLS
  gnutls_global_deinit();
#endif // HAVE_LIBGNUTLS
#ifdef ENABLE_METALINK
  xmlCleanupParser();
#endif // ENABLE_METALINK
  return 0;
}
