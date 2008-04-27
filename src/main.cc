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
#include "SharedHandle.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include "BitfieldManFactory.h"
#include "AuthConfigFactory.h"
#include "CookieBoxFactory.h"
#include "FeatureConfig.h"
#include "MultiUrlRequestInfo.h"
#include "SimpleRandomizer.h"
#include "Netrc.h"
#include "FatalException.h"
#include "File.h"
#include "CUIDCounter.h"
#include "UriListParser.h"
#include "message.h"
#include "prefs.h"
#include "Option.h"
#include "a2algo.h"
#include "a2io.h"
#include "a2time.h"
#include "Platform.h"
#include "ParameterizedStringParser.h"
#include "PStringBuildVisitor.h"
#include "SingleFileDownloadContext.h"
#include "DefaultBtContext.h"
#include "FileEntry.h"
#include "RequestGroup.h"
#include "ProtocolDetector.h"
#include "ConsoleStatCalc.h"
#include "NullStatCalc.h"
#include "StringFormat.h"
#ifdef ENABLE_METALINK
# include "MetalinkHelper.h"
# include "Metalink2RequestGroup.h"
# include "MetalinkEntry.h"
#endif // ENABLE_METALINK
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <deque>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <numeric>
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

namespace aria2 {

// output stream to /dev/null
std::ofstream nullout(DEV_NULL);

std::deque<std::string> unfoldURI(const std::deque<std::string>& args)
{
  std::deque<std::string> nargs;
  ParameterizedStringParser p;
  PStringBuildVisitor v;
  for(std::deque<std::string>::const_iterator itr = args.begin(); itr != args.end();
      ++itr) {
    v.reset();
    p.parse(*itr)->accept(&v);
    nargs.insert(nargs.end(), v.getURIs().begin(), v.getURIs().end()); 
  }
  return nargs;
}

RequestGroupHandle createRequestGroup(const Option* op, const std::deque<std::string>& uris,
				      const std::string& ufilename = "")
{
  RequestGroupHandle rg(new RequestGroup(op, uris));
  SingleFileDownloadContextHandle dctx
    (new SingleFileDownloadContext(op->getAsInt(PREF_SEGMENT_SIZE),
				  0,
				  "",
				  ufilename));
  dctx->setDir(op->get(PREF_DIR));
  rg->setDownloadContext(dctx);
  return rg;
}

SharedHandle<StatCalc> getStatCalc(const Option* op)
{
  SharedHandle<StatCalc> statCalc;
  if(op->getAsBool(PREF_QUIET)) {
    statCalc.reset(new NullStatCalc());
  } else {
    statCalc.reset(new ConsoleStatCalc());
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

#ifdef ENABLE_BITTORRENT

SharedHandle<RequestGroup>
createBtRequestGroup(const std::string& torrentFilePath,
		     Option* op,
		     const std::deque<std::string>& auxUris)
{
  SharedHandle<RequestGroup> rg(new RequestGroup(op, auxUris));
  SharedHandle<DefaultBtContext> btContext(new DefaultBtContext());
  btContext->load(torrentFilePath);// may throw exception
  if(op->defined(PREF_PEER_ID_PREFIX)) {
    btContext->setPeerIdPrefix(op->get(PREF_PEER_ID_PREFIX));
  }
  btContext->setDir(op->get(PREF_DIR));
  rg->setDownloadContext(btContext);
  btContext->setOwnerRequestGroup(rg.get());
  return rg;
}

int32_t downloadBitTorrent(Option* op, const std::deque<std::string>& uri)
{
  std::deque<std::string> nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    nargs = unfoldURI(uri);
  } else {
    nargs = uri;
  }
  std::deque<std::string> xargs;
  ncopy(nargs.begin(), nargs.end(), op->getAsInt(PREF_SPLIT),
	std::back_inserter(xargs));
  
  RequestGroupHandle rg = createBtRequestGroup(op->get(PREF_TORRENT_FILE),
					       op, xargs);
  
  RequestGroups groups;
  groups.push_back(rg);
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
int32_t downloadMetalink(Option* op)
{
  RequestGroups groups = Metalink2RequestGroup(op).generate(op->get(PREF_METALINK_FILE));
  if(groups.empty()) {
    throw FatalException("No files to download.");
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}
#endif // ENABLE_METALINK

class AccRequestGroup {
private:
  ProtocolDetector _detector;
  Option* _op;
public:
  AccRequestGroup(Option* op):_op(op) {}

  std::deque<SharedHandle<RequestGroup> >&
  operator()(std::deque<SharedHandle<RequestGroup> >& groups,
	     const std::string& uri)
  {
    if(_detector.isStreamProtocol(uri)) {
      std::deque<std::string> xuris;
      for(size_t count = _op->getAsInt(PREF_SPLIT); count; --count) {
	xuris.push_back(uri);
      }
      RequestGroupHandle rg = createRequestGroup(_op, xuris);
      groups.push_back(rg);
    }
#ifdef ENABLE_BITTORRENT
    else if(_detector.guessTorrentFile(uri)) {
      try {
	groups.push_back(createBtRequestGroup(uri, _op,
					      std::deque<std::string>()));
      } catch(RecoverableException& e) {
	// error occurred while parsing torrent file.
	// We simply ignore it.	
	LogFactory::getInstance()->error(EX_EXCEPTION_CAUGHT, e);
      }
    }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
    else if(_detector.guessMetalinkFile(uri)) {
      try {
	std::deque<SharedHandle<RequestGroup> > metalinkGroups =
	  Metalink2RequestGroup(_op).generate(uri);
	groups.insert(groups.end(), metalinkGroups.begin(), metalinkGroups.end());
      } catch(RecoverableException& e) {
	// error occurred while parsing metalink file.
	// We simply ignore it.
	LogFactory::getInstance()->error(EX_EXCEPTION_CAUGHT, e);
      }
    }
#endif // ENABLE_METALINK
    else {
      LogFactory::getInstance()->error(MSG_UNRECOGNIZED_URI, (uri).c_str());
    }
    return groups;
  }
};

int32_t downloadUriList(Option* op, std::istream& in)
{
  UriListParser p;
  RequestGroups groups;
  while(in) {
    std::deque<std::string> uris = p.parseNext(in);
    if(uris.size() == 1 && op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
      std::deque<std::string> unfoldedURIs = unfoldURI(uris);
      std::deque<SharedHandle<RequestGroup> > thisGroups =
	std::accumulate(unfoldedURIs.begin(), unfoldedURIs.end(),
			std::deque<SharedHandle<RequestGroup> >(),
			AccRequestGroup(op));
      groups.insert(groups.end(), thisGroups.begin(), thisGroups.end());
    } else if(uris.size() == 1) {
      std::deque<SharedHandle<RequestGroup> > thisGroups =
	std::accumulate(uris.begin(), uris.end(),
			std::deque<SharedHandle<RequestGroup> >(),
			AccRequestGroup(op));
      groups.insert(groups.end(), thisGroups.begin(), thisGroups.end());
    } else if(uris.size() > 0) {
      std::deque<std::string> xuris;
      ncopy(uris.begin(), uris.end(), op->getAsInt(PREF_SPLIT),
	    std::back_inserter(xuris));
      SharedHandle<RequestGroup> rg = createRequestGroup(op, xuris);
      groups.push_back(rg);
    }
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}

int32_t downloadUriList(Option* op)
{
  if(op->get(PREF_INPUT_FILE) == "-") {
    return downloadUriList(op, std::cin);
  } else {
    if(!File(op->get(PREF_INPUT_FILE)).isFile()) {
      throw FatalException
	(StringFormat(EX_FILE_OPEN, op->get(PREF_INPUT_FILE).c_str(),
		      "No such file").str());
    }
    std::ifstream f(op->get(PREF_INPUT_FILE).c_str());
    return downloadUriList(op, f);
  }
}

class StreamProtocolFilter {
private:
  ProtocolDetector _detector;
public:
  bool operator()(const std::string& uri) {
    return _detector.isStreamProtocol(uri);
  }
};

int32_t downloadUri(Option* op, const std::deque<std::string>& uris)
{
  std::deque<std::string> nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    nargs = unfoldURI(uris);
  } else {
    nargs = uris;
  }
  RequestGroups groups;
  if(op->get(PREF_FORCE_SEQUENTIAL) == V_TRUE) {
    groups = std::accumulate(nargs.begin(), nargs.end(),
			     std::deque<SharedHandle<RequestGroup> >(),
			     AccRequestGroup(op));
  } else {
    std::deque<std::string>::iterator strmProtoEnd =
      std::stable_partition(nargs.begin(), nargs.end(), StreamProtocolFilter());
    // let's process http/ftp protocols first.
    std::deque<std::string> xargs;
    ncopy(nargs.begin(), strmProtoEnd, op->getAsInt(PREF_SPLIT),
	  std::back_inserter(xargs));
    if(xargs.size()) {
      RequestGroupHandle rg = createRequestGroup(op, xargs, op->get(PREF_OUT));
      groups.push_back(rg);
    }
    // process remaining URIs(local metalink, BitTorrent files)
    std::deque<SharedHandle<RequestGroup> > remGroups =
      std::accumulate(strmProtoEnd, nargs.end(),
		      std::deque<SharedHandle<RequestGroup> >(),
		      AccRequestGroup(op));
    groups.insert(groups.end(), remGroups.begin(), remGroups.end());
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}

int main(int argc, char* argv[])
{
  Option* op = option_processing(argc, argv);
  std::deque<std::string> args(argv+optind, argv+argc);

  SimpleRandomizer::init();
  BitfieldManFactory::setDefaultRandomizer(SimpleRandomizer::getInstance());
  if(op->getAsBool(PREF_STDOUT_LOG)) {
    LogFactory::setLogFile(DEV_STDOUT);
  } else if(op->get(PREF_LOG).size()) {
    LogFactory::setLogFile(op->get(PREF_LOG));
  } else {
    LogFactory::setLogFile(DEV_NULL);
  }
  if(op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
  int32_t exitStatus = EXIT_SUCCESS;
  try {
    Logger* logger = LogFactory::getInstance();
    logger->info("%s %s %s", PACKAGE, PACKAGE_VERSION, TARGET);
    logger->info(MSG_LOGGING_STARTED);

    AuthConfigFactoryHandle authConfigFactory(new AuthConfigFactory(op));
    File netrccf(op->get(PREF_NETRC_PATH));
    if(!op->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
      mode_t mode = netrccf.mode();
      if(mode&(S_IRWXG|S_IRWXO)) {
	logger->notice(MSG_INCORRECT_NETRC_PERMISSION,
		       op->get(PREF_NETRC_PATH).c_str());
      } else {
	NetrcHandle netrc(new Netrc());
	netrc->parse(op->get(PREF_NETRC_PATH));
	authConfigFactory->setNetrc(netrc);
      }
    }

    CookieBoxFactoryHandle cookieBoxFactory(new CookieBoxFactory());
    CookieBoxFactorySingletonHolder::instance(cookieBoxFactory);
    if(op->defined(PREF_LOAD_COOKIES)) {
      File cookieFile(op->get(PREF_LOAD_COOKIES));
      if(cookieFile.isFile()) {
	std::ifstream in(op->get(PREF_LOAD_COOKIES).c_str());
	CookieBoxFactorySingletonHolder::instance()->loadDefaultCookie(in);
      } else {
	logger->error(MSG_LOADING_COOKIE_FAILED, op->get(PREF_LOAD_COOKIES).c_str());
	exit(EXIT_FAILURE);
      }
    }

    AuthConfigFactorySingleton::instance(authConfigFactory);
    CUIDCounterHandle cuidCounter(new CUIDCounter());
    CUIDCounterSingletonHolder::instance(cuidCounter);
#ifdef ENABLE_MESSAGE_DIGEST
    MessageDigestHelper::staticSHA1DigestInit();
#endif // ENABLE_MESSAGE_DIGEST

#ifdef SIGPIPE
    Util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);
#endif
    int32_t returnValue = 0;
#ifdef ENABLE_BITTORRENT
    if(op->defined(PREF_TORRENT_FILE)) {
      if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	DefaultBtContextHandle btContext(new DefaultBtContext());
	btContext->load(op->get(PREF_TORRENT_FILE));
	std::cout << btContext << std::endl;
      } else {
	returnValue = downloadBitTorrent(op, args);
      }
    }
    else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if(op->defined(PREF_METALINK_FILE)) {
	if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	  Util::toStream(std::cout, MetalinkEntry::toFileEntry(MetalinkHelper::parseAndQuery(op->get(PREF_METALINK_FILE), op)));
	} else {
	  returnValue = downloadMetalink(op);
	}
      }
      else
#endif // ENABLE_METALINK
	if(op->defined(PREF_INPUT_FILE)) {
	  returnValue = downloadUriList(op);
	} else {
	  returnValue = downloadUri(op, args);
	}
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
#ifdef HAVE_WINSOCK2_H
  aria2::Platform platform;
#endif // HAVE_WINSOCK2_H

#ifdef ENABLE_NLS
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif // ENABLE_NLS
  
#ifdef HAVE_LIBSSL
  // for SSL initialization
  SSL_load_error_strings();
  SSL_library_init();
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  gnutls_global_init();
#endif // HAVE_LIBGNUTLS

  int r = aria2::main(argc, argv);

#ifdef HAVE_LIBGNUTLS
  gnutls_global_deinit();
#endif // HAVE_LIBGNUTLS

  return r;
}
