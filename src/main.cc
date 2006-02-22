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
#include "DownloadEngine.h"
#include "SegmentMan.h"
#include "SimpleLogger.h"
#include "common.h"
#include "DefaultDiskWriter.h"
#include "Util.h"
#include "InitiateConnectionCommandFactory.h"
#include "prefs.h"
#include <vector>
#include <algorithm>
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

using namespace std;

void clearRequest(Request* req) {
  delete(req);
}

DownloadEngine* e;

void handler(int signal) {
  cout << "\nSIGINT signal received." << endl;
  e->segmentMan->save();
  if(e->diskWriter != NULL) {
    e->diskWriter->closeFile();
  }
  exit(0);
}

void addCommand(int cuid, const char* url, string referer, vector<Request*> requests) {
  Request* req = new Request();
  req->setReferer(referer);
  if(req->setUrl(url)) {
    e->commands.push(InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, e));
    requests.push_back(req);
  } else {
    cerr << "Unrecognized URL or unsupported protocol: " << req->getUrl() << endl;
    delete(req);
  }
}

void showVersion() {
  cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << endl;
  cout << "Copyright (C) 2006 Tatsuhiro Tsujikawa" << endl;
  cout << endl;
  cout << "This program is free software; you can redistribute it and/or modify" << endl;
  cout << "it under the terms of the GNU General Public License as published by" << endl;
  cout << "the Free Software Foundation; either version 2 of the License, or" << endl;
  cout << "(at your option) any later version." << endl;
  cout << endl;
  cout << "This program is distributed in the hope that it will be useful," << endl;
  cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl;
  cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl;
  cout << "GNU General Public License for more details." << endl;
  cout << endl;
  cout << "You should have received a copy of the GNU General Public License" << endl;
  cout << "along with this program; if not, write to the Free Software" << endl;
  cout << "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA" << endl;
  cout << endl;
  cout << "Contact Info: Tasuhiro Tsujikawa <tujikawa at users dot sourceforge dot net>" << endl;
  cout << endl;

}

void showUsage() {
  cout << endl;
  cout << "Usage: " << PACKAGE_NAME << " [options] URL ..." << endl;
  cout << endl;
  cout << "Options:" << endl;
  cout << " -d, --dir=DIR                The directory to store downloaded file." << endl;
  cout << " -o, --out=FILE               The file name for downloaded file." << endl;
  cout << " -l, --log=LOG                The file path to store log. If '-' is specified," << endl;
  cout << "                              log is written to stdout." << endl;
  cout << " -D, --daemon                 Run as daemon." << endl;
  cout << " -s, --split=N                Download a file using s connections. s must be" << endl;
  cout << "                              between 1 and 5. This option affects all URLs." << endl;
  cout << "                              Thus, aria2 connects to each URL with N connections." << endl;
  cout << " --retry-wait=SEC             Set amount of time in second between requests" << endl;
  cout << "                              for errors. Specify a value between 0 and 60." << endl;
  cout << "                              Default: 5" << endl;
  cout << " -t, --timeout=SEC            Set timeout in second. Default: 60" << endl;
  cout << " -m, --max-tries=N            Set number of tries. 0 means unlimited." << endl;
  cout << "                              Default: 5" << endl;
  cout << " --min-segment-size=SIZE[K|M] Set minimum segment size. You can append" << endl;
  cout << "                              K or M(1K = 1024, 1M = 1024K)." << endl;
  cout << " --http-proxy=HOST:PORT       Use HTTP proxy server. This affects to all" << endl;
  cout << "                              URLs." << endl;
  cout << " --http-user=USER             Set HTTP user. This affects to all URLs." << endl;
  cout << " --http-passwd=PASSWD         Set HTTP password. This affects to all URLs." << endl;
  cout << " --http-proxy-user=USER       Set HTTP proxy user. This affects to all URLs" << endl;
  cout << " --http-proxy-passwd=PASSWD   Set HTTP proxy password. This affects to all URLs." << endl;
  cout << " --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic" << endl;
  cout << "                              is the only supported scheme. You MUST specify" << endl;
  cout << "                              this option in order to use HTTP authentication" << endl;
  cout << "                              as well as --http-proxy option." << endl;
  cout << " --referer=REFERER            Set Referer. This affects to all URLs." << endl;
  cout << " --ftp-user=USER              Set FTP user. This affects to all URLs." << endl;
  cout << "                              Default: anonymous" << endl;
  cout << " --ftp-passwd=PASSWD          Set FTP password. This affects to all URLs." << endl;
  cout << "                              Default: ARIA2USER@" << endl;
  cout << " --ftp-type=TYPE              Set FTP transfer type. TYPE is either 'binary'" << endl;
  cout << "                              or 'ascii'." << endl;
  cout << "                              Default: binary" << endl;
  cout << " -p, --ftp-pasv               Use passive mode in FTP." << endl;
  cout << " --ftp-via-http-proxy=WAY     Use HTTP proxy in FTP. WAY is either 'get' or" << endl;
  cout << "                              'tunnel'." << endl;
  cout << " -v, --version                Print the version number and exit." << endl;
  cout << " -h, --help                   Print this message and exit." << endl;
  cout << endl;
  cout << "URL:" << endl;
  cout << " You can specify multiple URLs. All URLs must point to the same file" << endl;
  cout << " or downloading fails." << endl;
  cout << endl;
  cout << "Examples:" << endl;
  cout << " Download a file by 1 connection:" << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip" << endl;
  cout << " Download a file by 2 connections:" << endl;
  cout << "  aria2c -s 2 http://AAA.BBB.CCC/file.zip" << endl;
  cout << " Download a file by 2 connections, each connects to a different server:" << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip http://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << " You can mix up different protocols:" << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip ftp://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << endl;
  cout << "Reports bugs to <tujikawa at rednoah dot com>" << endl;
}

int main(int argc, char* argv[]) {
  bool stdoutLog = false;
  string logfile;
  string dir;
  string ufilename;
  int split = 1;
  bool daemonMode = false;
  string referer;

  int c;
  Option* op = new Option();
  op->put(PREF_RETRY_WAIT, "5");
  op->put(PREF_TIMEOUT, "60");
  op->put(PREF_MIN_SEGMENT_SIZE, "1048576");// 1M
  op->put(PREF_MAX_TRIES, "5");
  op->put(PREF_FTP_USER, "anonymous");
  op->put(PREF_FTP_PASSWD, "ARIA2USER@");
  op->put(PREF_FTP_TYPE, V_BINARY);
  op->put(PREF_FTP_PASV_ENABLED, V_TRUE);
  op->put(PREF_FTP_VIA_HTTP_PROXY, V_TUNNEL);

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
      { "max-retries", required_argument, NULL, 'm' },
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
      { "version", no_argument, NULL, 'v' },
      { "help", no_argument, NULL, 'h' },
      { 0, 0, 0, 0 }
    };
    c = getopt_long(argc, argv, "Dd:o:l:s:pt:m:vh", longOpts, &optIndex);
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
	  cerr << "unrecognized proxy format" << endl;
	  showUsage();
	  exit(1);
	}
	op->put(PREF_HTTP_PROXY_HOST, proxy.first);
	op->put(PREF_HTTP_PROXY_PORT, Util::itos(port));
	op->put(PREF_HTTP_PROXY_ENABLED, V_TRUE);
	break;
      }
      case 2:
	op->put(PREF_HTTP_USER, optarg);
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
	  cerr << "Currently, supported authentication scheme is basic." << endl;
	}
	break;
      case 7:
	referer = optarg;
	break;
      case 8: {
	int wait = (int)strtol(optarg, NULL, 10);
	if(!(0 <= wait && wait <= 60)) {
	  cerr << "retry-wait must be between 0 and 60." << endl;
	  showUsage();
	  exit(1);
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
	  cerr << "ftp-type must be either 'binary' or 'ascii'." << endl;
	  showUsage();
	  exit(1);
	}
	break;
      case 12:
	if(string(optarg) == V_GET || string(optarg) == V_TUNNEL) {
	  op->put(PREF_FTP_VIA_HTTP_PROXY, optarg);
	} else {
	  cerr << "ftp-via-http-proxy must be either 'get' or 'tunnel'." << endl;
	  showUsage();
	  exit(1);
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
	if(size <= 0) {
	  cerr << "min-segment-size invalid" << endl;
	  showUsage();
	  exit(1);
	}
	op->put(PREF_MIN_SEGMENT_SIZE, Util::llitos(size));
	break;
      }
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
	cerr << "split must be between 1 and 5." << endl;
	showUsage();
	exit(1);
      }
      break;
    case 't': {
      int timeout = (int)strtol(optarg, NULL, 10);
      if(1 <= timeout && timeout <= 600) {
	op->put(PREF_TIMEOUT, Util::itos(timeout));
      } else {
	cerr << "timeout must be between 1 and 600" << endl;
	showUsage();
	exit(1);
      }
      break;
    }
    case 'm': {
      int retries = (int)strtol(optarg, NULL, 10);
      if(retries < 0) {
	cerr << "max-retires invalid" << endl;
	showUsage();
	exit(1);
      }
      op->put(PREF_MAX_TRIES, Util::itos(retries));
      break;
    }
    case 'p':
      op->put(PREF_FTP_PASV_ENABLED, V_TRUE);
      break;
    case 'v':
      showVersion();
      exit(0);
    case 'h':
      showUsage();
      exit(0);
    default:
      showUsage();
      exit(1);
    }
  }
  if(optind == argc) {
    cerr << "specify at least one URL" << endl;
    showUsage();
    exit(1);
  }
  if(daemonMode) {
    if(daemon(1, 1) < 0) {
      perror("daemon failed");
      exit(1);
    }
  }
#ifdef HAVE_LIBSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
#endif // HAVE_LIBSSL
  SimpleLogger* logger;
  if(stdoutLog) {
    logger = new SimpleLogger(stdout);
  } else if(logfile.size()) {
    logger = new SimpleLogger(logfile);
  } else {
    logger = new SimpleLogger("/dev/null");
  }
  e = new DownloadEngine();
  e->logger = logger;
  e->option = op;
  e->diskWriter = new DefaultDiskWriter();
  e->segmentMan = new SegmentMan();
  e->segmentMan->dir = dir;
  e->segmentMan->ufilename = ufilename;
  e->segmentMan->logger = logger;
  e->segmentMan->option = op;
  vector<Request*> requests;
  for(int i = 1; optind+i-1 < argc; i++) {
    for(int s = 1; s <= split; s++) {
      addCommand(split*(i-1)+s, argv[optind+i-1], referer, requests); 
    }
  }
  struct sigaction sigact;
  sigact.sa_handler = handler;
  sigact.sa_flags = 0;
  sigemptyset(&sigact.sa_mask);
  sigaction(SIGINT, &sigact, NULL);

  e->run();
  
  for_each(requests.begin(), requests.end(), clearRequest);
  requests.clear();
  delete(logger);
  delete(e->segmentMan);
  delete(e->option);
  delete(e->diskWriter);
  delete(e);
  return 0;
}
