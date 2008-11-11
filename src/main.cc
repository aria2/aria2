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

#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>

#include "SharedHandle.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include "BitfieldManFactory.h"
#include "FeatureConfig.h"
#include "MultiUrlRequestInfo.h"
#include "SimpleRandomizer.h"
#include "FatalException.h"
#include "File.h"
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
#include "A2STR.h"
#include "RecoverableException.h"
#ifdef ENABLE_METALINK
# include "MetalinkHelper.h"
# include "Metalink2RequestGroup.h"
# include "MetalinkEntry.h"
#endif // ENABLE_METALINK
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigestHelper.h"
#endif // ENABLE_MESSAGE_DIGEST

extern char* optarg;
extern int optind, opterr, optopt;

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

RequestGroupHandle createRequestGroup
(const Option* op, const std::deque<std::string>& uris,
 const Option& requestOption)
{
  RequestGroupHandle rg(new RequestGroup(op, uris));
  SingleFileDownloadContextHandle dctx
    (new SingleFileDownloadContext(op->getAsInt(PREF_SEGMENT_SIZE),
				   0,
				   A2STR::NIL,
				   requestOption.get(PREF_OUT)));
  dctx->setDir(requestOption.get(PREF_DIR));
  rg->setDownloadContext(dctx);
  return rg;
}

SharedHandle<StatCalc> getStatCalc(const Option* op)
{
  SharedHandle<StatCalc> statCalc;
  if(op->getAsBool(PREF_QUIET)) {
    statCalc.reset(new NullStatCalc());
  } else {
    statCalc.reset(new ConsoleStatCalc(op->getAsInt(PREF_SUMMARY_INTERVAL)));
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
		     const std::deque<std::string>& auxUris,
		     const Option& requestOption)
{
  SharedHandle<RequestGroup> rg(new RequestGroup(op, auxUris));
  SharedHandle<DefaultBtContext> btContext(new DefaultBtContext());
  btContext->load(torrentFilePath);// may throw exception
  if(op->defined(PREF_PEER_ID_PREFIX)) {
    btContext->setPeerIdPrefix(op->get(PREF_PEER_ID_PREFIX));
  }
  btContext->setDir(requestOption.get(PREF_DIR));
  rg->setDownloadContext(btContext);
  btContext->setOwnerRequestGroup(rg.get());
  return rg;
}

int32_t downloadBitTorrent(Option* op, const std::deque<std::string>& uris)
{
  std::deque<std::string> nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    nargs = unfoldURI(uris);
  } else {
    nargs = uris;
  }
  RequestGroups groups;
  size_t numSplit = op->getAsInt(PREF_SPLIT);
  if(nargs.size() >= numSplit) {
    RequestGroupHandle rg = createBtRequestGroup(op->get(PREF_TORRENT_FILE),
						 op, nargs, *op);
    rg->setNumConcurrentCommand(numSplit);
    groups.push_back(rg);
  } else {
    std::deque<std::string> xargs;
    if(!nargs.empty()) {
      ncopy(nargs.begin(), nargs.end(), numSplit, std::back_inserter(xargs));
      xargs.erase(xargs.begin()+numSplit, xargs.end());
    }
    RequestGroupHandle rg = createBtRequestGroup(op->get(PREF_TORRENT_FILE),
						 op, xargs, *op);
    rg->setNumConcurrentCommand(numSplit);
    groups.push_back(rg);
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
int32_t downloadMetalink(Option* op)
{
  RequestGroups groups;
  Metalink2RequestGroup(op).generate(groups, op->get(PREF_METALINK_FILE), *op);
  if(groups.empty()) {
    throw FatalException("No files to download.");
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}
#endif // ENABLE_METALINK

class AccRequestGroup {
private:
  std::deque<SharedHandle<RequestGroup> >& _requestGroups;
  ProtocolDetector _detector;
  Option* _op;
  Option& _requestOption;
public:
  AccRequestGroup(std::deque<SharedHandle<RequestGroup> >& requestGroups,
		  Option* op,
		  Option& requestOption):
    _requestGroups(requestGroups), _op(op), _requestOption(requestOption) {}

  void
  operator()(const std::string& uri)
  {
    if(_detector.isStreamProtocol(uri)) {
      std::deque<std::string> xuris;
      for(size_t count = _op->getAsInt(PREF_SPLIT); count; --count) {
	xuris.push_back(uri);
      }
      RequestGroupHandle rg = createRequestGroup(_op, xuris, _requestOption);
      _requestGroups.push_back(rg);
    }
#ifdef ENABLE_BITTORRENT
    else if(_detector.guessTorrentFile(uri)) {
      try {
	_requestGroups.push_back(createBtRequestGroup(uri, _op,
						      std::deque<std::string>(),
						      _requestOption));
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
	Metalink2RequestGroup(_op).generate(_requestGroups, uri,
					    _requestOption);
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
  }
};

void copyIfndef(Option& dest, const Option& src, const std::string& name)
{
  if(!dest.defined(name)) {
    dest.put(name, src.get(name));
  }
}

int32_t downloadUriList(Option* op, std::istream& in)
{
  UriListParser p(in);
  RequestGroups groups;
  while(p.hasNext()) {
    std::deque<std::string> uris;
    Option requestOption;
    p.parseNext(uris, requestOption);
    copyIfndef(requestOption, *op, PREF_DIR);
    copyIfndef(requestOption, *op, PREF_OUT);
    if(uris.size() == 1 && op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
      std::deque<std::string> unfoldedURIs = unfoldURI(uris);
      std::for_each(unfoldedURIs.begin(), unfoldedURIs.end(),
		    AccRequestGroup(groups, op, requestOption));
    } else if(uris.size() == 1) {
      std::for_each(uris.begin(), uris.end(),
		    AccRequestGroup(groups, op, requestOption));
    } else if(!uris.empty()) {
      size_t numSplit = op->getAsInt(PREF_SPLIT);
      if(uris.size() >= numSplit) {
	SharedHandle<RequestGroup> rg =
	  createRequestGroup(op, uris, requestOption);
	rg->setNumConcurrentCommand(numSplit);
	groups.push_back(rg);
      } else {
	std::deque<std::string> xuris;
	ncopy(uris.begin(), uris.end(), numSplit, std::back_inserter(xuris));
	xuris.erase(xuris.begin()+numSplit, xuris.end());
	SharedHandle<RequestGroup> rg =
	  createRequestGroup(op, xuris, requestOption);
	rg->setNumConcurrentCommand(numSplit);
	groups.push_back(rg);
      }
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
    std::for_each(nargs.begin(), nargs.end(), AccRequestGroup(groups, op, *op));
  } else {
    std::deque<std::string>::iterator strmProtoEnd =
      std::stable_partition(nargs.begin(), nargs.end(), StreamProtocolFilter());
    // let's process http/ftp protocols first.
    size_t numSplit = op->getAsInt(PREF_SPLIT);
    size_t numURIs = std::distance(nargs.begin(), strmProtoEnd);
    if(numURIs >= numSplit) {
      std::deque<std::string> xargs(nargs.begin(), strmProtoEnd);
      RequestGroupHandle rg = createRequestGroup(op, xargs, *op);
      rg->setNumConcurrentCommand(numSplit);
      groups.push_back(rg);
    } else if(numURIs > 0) {
      std::deque<std::string> xargs;
      ncopy(nargs.begin(), strmProtoEnd, numSplit, std::back_inserter(xargs));
      xargs.erase(xargs.begin()+numSplit, xargs.end());
      RequestGroupHandle rg = createRequestGroup(op, xargs, *op);
      rg->setNumConcurrentCommand(numSplit);
      groups.push_back(rg);
    }
    // process remaining URIs(local metalink, BitTorrent files)
    std::for_each(strmProtoEnd, nargs.end(), AccRequestGroup(groups, op, *op));
  }
  return MultiUrlRequestInfo(groups, op, getStatCalc(op), getSummaryOut(op)).execute();
}

int main(int argc, char* argv[])
{
  Option* op = option_processing(argc, argv);
  std::deque<std::string> args(argv+optind, argv+argc);

  SimpleRandomizer::init();
  BitfieldManFactory::setDefaultRandomizer(SimpleRandomizer::getInstance());
  if(op->get(PREF_LOG) == "-") {
    LogFactory::setLogFile(DEV_STDOUT);
  } else if(!op->get(PREF_LOG).empty()) {
    LogFactory::setLogFile(op->get(PREF_LOG));
  } else {
    LogFactory::setLogFile(DEV_NULL);
  }
  LogFactory::setLogLevel(op->get(PREF_LOG_LEVEL));
  if(op->getAsBool(PREF_QUIET)) {
    LogFactory::setConsoleOutput(false);
  }
  int32_t exitStatus = EXIT_SUCCESS;
  try {
    Logger* logger = LogFactory::getInstance();
    logger->info("<<--- --- --- ---");
    logger->info("  --- --- --- ---");
    logger->info("  --- --- --- --->>");
    logger->info("%s %s %s", PACKAGE, PACKAGE_VERSION, TARGET);
    logger->info(MSG_LOGGING_STARTED);

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
	  std::deque<SharedHandle<MetalinkEntry> > metalinkEntries;
	  MetalinkHelper::parseAndQuery(metalinkEntries,
					op->get(PREF_METALINK_FILE), op);
	  std::deque<SharedHandle<FileEntry> > fileEntries;
	  MetalinkEntry::toFileEntry(fileEntries, metalinkEntries);
	  Util::toStream(std::cout, fileEntries);
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
  aria2::Platform platform;

  int r = aria2::main(argc, argv);

  return r;
}
