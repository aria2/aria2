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
#include "BitfieldManFactory.h"
#include "SimpleRandomizer.h"
#include "Netrc.h"
#include "AuthConfigFactory.h"
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
#include "SingleFileDownloadContext.h"
#include "DefaultBtContext.h"
#include "RequestGroup.h"
#include "Option.h"
#include "MetalinkHelper.h"
#include <deque>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <utility>
#include <fstream>
extern char* optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

#ifdef ENABLE_METALINK
#include "Xml2MetalinkProcessor.h"
#include "Metalink2RequestGroup.h"
#endif

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/err.h>
# include <openssl/ssl.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

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

RequestGroupHandle createRequestGroup(const Option* op, const Strings& uris,
				      const string& ufilename = "")
{
  RequestGroupHandle rg = new RequestGroup(op, uris);
  SingleFileDownloadContextHandle dctx =
    new SingleFileDownloadContext(op->getAsInt(PREF_SEGMENT_SIZE),
				  0,
				  "",
				  ufilename);
  dctx->setDir(op->get(PREF_DIR));
  rg->setDownloadContext(dctx);
  return rg;
}

extern Option* option_processing(int argc, char* const argv[]);

#ifdef ENABLE_BITTORRENT
void downloadBitTorrent(Option* op, const Strings& uri)
{
  Strings nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    nargs = unfoldURI(uri);
  } else {
    nargs = uri;
  }
  Strings xargs;
  ncopy(nargs.begin(), nargs.end(), op->getAsInt(PREF_SPLIT),
	back_inserter(xargs));
  
  RequestGroupHandle rg = new RequestGroup(op, xargs);
  DefaultBtContextHandle btContext = new DefaultBtContext();
  btContext->load(op->get(PREF_TORRENT_FILE));
  if(op->defined(PREF_PEER_ID_PREFIX)) {
    btContext->setPeerIdPrefix(op->get(PREF_PEER_ID_PREFIX));
  }
  btContext->setDir(op->get(PREF_DIR));
  rg->setDownloadContext(btContext);
  btContext->setOwnerRequestGroup(rg.get());
  
  RequestGroups groups;
  groups.push_back(rg);
  MultiUrlRequestInfo(groups, op).execute();
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void downloadMetalink(Option* op)
{
  RequestGroups groups = Metalink2RequestGroup(op).generate(op->get(PREF_METALINK_FILE));
  if(groups.empty()) {
    throw new FatalException("No files to download.");
  }
  MultiUrlRequestInfo(groups, op).execute();
}
#endif // ENABLE_METALINK

void downloadUriList(Option* op)
{
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
	ncopy(itr, itr+1, op->getAsInt(PREF_SPLIT), back_inserter(xuris));
	RequestGroupHandle rg = createRequestGroup(op, xuris);
	groups.push_back(rg);
      }
    } else if(uris.size() > 0) {
      Strings xuris;
      ncopy(uris.begin(), uris.end(), op->getAsInt(PREF_SPLIT),
	    back_inserter(xuris));
      RequestGroupHandle rg = createRequestGroup(op, xuris);
      groups.push_back(rg);
    }
  }
  MultiUrlRequestInfo(groups, op).execute();
}

void downloadUri(Option* op, const Strings& uris)
{
  Strings nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    nargs = unfoldURI(uris);
  } else {
    nargs = uris;
  }
  RequestGroups groups;
  if(op->get(PREF_FORCE_SEQUENTIAL) == V_TRUE) {
    for(Strings::const_iterator itr = nargs.begin();
	itr != nargs.end(); ++itr) {
      Strings xuris;
      ncopy(itr, itr+1, op->getAsInt(PREF_SPLIT),
	    back_inserter(xuris));
      RequestGroupHandle rg = createRequestGroup(op, xuris);
      groups.push_back(rg);
    }
  } else {
    Strings xargs;
    ncopy(nargs.begin(), nargs.end(), op->getAsInt(PREF_SPLIT),
	  back_inserter(xargs));
    RequestGroupHandle rg = createRequestGroup(op, xargs, op->get(PREF_OUT));
    groups.push_back(rg);
  }
  MultiUrlRequestInfo(groups, op).execute();
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
  Option* op = option_processing(argc, argv);
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
    logger->info("%s %s %s", PACKAGE, PACKAGE_VERSION, TARGET);
    logger->info(MSG_LOGGING_STARTED);

    AuthConfigFactoryHandle authConfigFactory = new AuthConfigFactory(op);
    File netrccf(op->get(PREF_NETRC_PATH));
    if(!op->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
      mode_t mode = netrccf.mode();
      if(mode&(S_IRWXG|S_IRWXO)) {
	logger->notice(MSG_INCORRECT_NETRC_PERMISSION,
		       op->get(PREF_NETRC_PATH).c_str());
      } else {
	NetrcHandle netrc = new Netrc();
	netrc->parse(op->get(PREF_NETRC_PATH));
	authConfigFactory->setNetrc(netrc);
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

    AuthConfigFactorySingleton::instance(authConfigFactory);
    CUIDCounterHandle cuidCounter = new CUIDCounter();
    CUIDCounterSingletonHolder::instance(cuidCounter);
#ifdef SIGPIPE
    Util::setGlobalSignalHandler(SIGPIPE, SIG_IGN, 0);
#endif

#ifdef ENABLE_BITTORRENT
    if(op->defined(PREF_TORRENT_FILE)) {
      if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	DefaultBtContextHandle btContext = new DefaultBtContext();
	btContext->load(op->get(PREF_TORRENT_FILE));
	cout << btContext << endl;
      } else {
	downloadBitTorrent(op, args);
      }
    }
    else
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
      if(op->defined(PREF_METALINK_FILE)) {
	if(op->get(PREF_SHOW_FILES) == V_TRUE) {
	  Util::toStream(cout, MetalinkEntry::toFileEntry(MetalinkHelper::parseAndQuery(op->get(PREF_METALINK_FILE), op)));
	} else {
	  downloadMetalink(op);
	}
      }
      else
#endif // ENABLE_METALINK
	if(op->defined(PREF_INPUT_FILE)) {
	  downloadUriList(op);
	} else {
	  downloadUri(op, args);
	}
  } catch(Exception* ex) {
    cerr << EX_EXCEPTION_CAUGHT << "\n" << *ex << endl;
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
