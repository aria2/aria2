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
#include "download_helper.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "RequestGroup.h"
#include "Option.h"
#include "prefs.h"
#include "Metalink2RequestGroup.h"
#include "ProtocolDetector.h"
#include "ParameterizedStringParser.h"
#include "PStringBuildVisitor.h"
#include "UriListParser.h"
#include "SingleFileDownloadContext.h"
#include "RecoverableException.h"
#include "FatalException.h"
#include "message.h"
#include "StringFormat.h"
#include "DefaultBtContext.h"
#include "FileEntry.h"
#include "LogFactory.h"
#include "File.h"

namespace aria2 {

static void unfoldURI
(std::deque<std::string>& result, const std::deque<std::string>& args)
{
  ParameterizedStringParser p;
  PStringBuildVisitor v;
  for(std::deque<std::string>::const_iterator itr = args.begin();
      itr != args.end(); ++itr) {
    v.reset();
    p.parse(*itr)->accept(&v);
    result.insert(result.end(), v.getURIs().begin(), v.getURIs().end()); 
  }
}

static void splitURI(std::deque<std::string>& result,
		     std::deque<std::string>::const_iterator begin,
		     std::deque<std::string>::const_iterator end,
		     size_t numSplit)
{
  size_t numURIs = std::distance(begin, end);
  if(numURIs >= numSplit) {
    result.insert(result.end(), begin, end);
  } else if(numURIs > 0) {
    for(size_t i = 0; i < numSplit/numURIs; ++i) {
      result.insert(result.end(), begin, end);
    }
    result.insert(result.end(), begin, begin+(numSplit%numURIs));
  }
}

static SharedHandle<RequestGroup> createRequestGroup
(const Option* op, const std::deque<std::string>& uris,
 const Option& requestOption,
 bool useOutOption = false)
{
  SharedHandle<RequestGroup> rg(new RequestGroup(op, uris));
  SharedHandle<SingleFileDownloadContext> dctx
    (new SingleFileDownloadContext(op->getAsInt(PREF_SEGMENT_SIZE),
				   0,
				   A2STR::NIL,
				   useOutOption?
				   requestOption.get(PREF_OUT):A2STR::NIL));
  dctx->setDir(requestOption.get(PREF_DIR));
  rg->setDownloadContext(dctx);
  return rg;
}

#ifdef ENABLE_BITTORRENT

static
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

void createRequestGroupForBitTorrent
(std::deque<SharedHandle<RequestGroup> >& result, Option* op,
 const std::deque<std::string>& uris)
{
  std::deque<std::string> nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  // we ignore -Z option here
  size_t numSplit = op->getAsInt(PREF_SPLIT);
  std::deque<std::string> auxUris;
  splitURI(auxUris, nargs.begin(), nargs.end(), numSplit);
  SharedHandle<RequestGroup> rg =
    createBtRequestGroup(op->get(PREF_TORRENT_FILE), op, auxUris, *op);
  rg->setNumConcurrentCommand(numSplit);
  result.push_back(rg);
}

#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK
void createRequestGroupForMetalink
(std::deque<SharedHandle<RequestGroup> >& result, Option* op)
{
  Metalink2RequestGroup(op).generate(result, op->get(PREF_METALINK_FILE), *op);
  if(result.empty()) {
    throw FatalException(MSG_NO_FILES_TO_DOWNLOAD);
  }
}
#endif // ENABLE_METALINK

class AccRequestGroup {
private:
  std::deque<SharedHandle<RequestGroup> >& _requestGroups;
  ProtocolDetector _detector;
  Option* _op;
  const Option& _requestOption;
public:
  AccRequestGroup(std::deque<SharedHandle<RequestGroup> >& requestGroups,
		  Option* op,
		  const Option& requestOption):
    _requestGroups(requestGroups), _op(op), _requestOption(requestOption) {}

  void
  operator()(const std::string& uri)
  {
    if(_detector.isStreamProtocol(uri)) {
      std::deque<std::string> streamURIs;
      size_t numSplit = _op->getAsInt(PREF_SPLIT);
      for(size_t i = 0; i < numSplit; ++i) {
	streamURIs.push_back(uri);
      }
      SharedHandle<RequestGroup> rg =
	createRequestGroup(_op, streamURIs, _requestOption);
      rg->setNumConcurrentCommand(numSplit);
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

class StreamProtocolFilter {
private:
  ProtocolDetector _detector;
public:
  bool operator()(const std::string& uri) {
    return _detector.isStreamProtocol(uri);
  }
};

static void copyIfndef(Option& dest, const Option& src, const std::string& name)
{
  if(!dest.defined(name)) {
    dest.put(name, src.get(name));
  }
}

static void createRequestGroupForUri
(std::deque<SharedHandle<RequestGroup> >& result, Option* op,
 const std::deque<std::string>& uris, const Option& requestOption)
{
  std::deque<std::string> nargs;
  if(op->get(PREF_PARAMETERIZED_URI) == V_TRUE) {
    unfoldURI(nargs, uris);
  } else {
    nargs = uris;
  }
  if(op->get(PREF_FORCE_SEQUENTIAL) == V_TRUE) {
    std::for_each(nargs.begin(), nargs.end(),
		  AccRequestGroup(result, op, requestOption));
  } else {
    std::deque<std::string>::iterator strmProtoEnd =
      std::stable_partition(nargs.begin(), nargs.end(), StreamProtocolFilter());
    // let's process http/ftp protocols first.
    if(nargs.begin() != strmProtoEnd) {
      size_t numSplit = op->getAsInt(PREF_SPLIT);
      std::deque<std::string> streamURIs;
      splitURI(streamURIs, nargs.begin(), strmProtoEnd,
	       numSplit);
      SharedHandle<RequestGroup> rg =
	createRequestGroup(op, streamURIs, requestOption, true);
      rg->setNumConcurrentCommand(numSplit);
      result.push_back(rg);
    }
    // process remaining URIs(local metalink, BitTorrent files)
    std::for_each(strmProtoEnd, nargs.end(),
		  AccRequestGroup(result, op, requestOption));
  }
}

void createRequestGroupForUri
(std::deque<SharedHandle<RequestGroup> >& result, Option* op,
 const std::deque<std::string>& uris)
{
  createRequestGroupForUri(result, op, uris, *op);
}

static void createRequestGroupForUriList
(std::deque<SharedHandle<RequestGroup> >& result, Option* op, std::istream& in)
{
  UriListParser p(in);
  while(p.hasNext()) {
    std::deque<std::string> uris;
    Option requestOption;
    p.parseNext(uris, requestOption);
    if(uris.empty()) {
      continue;
    }
    copyIfndef(requestOption, *op, PREF_DIR);
    copyIfndef(requestOption, *op, PREF_OUT);

    createRequestGroupForUri(result, op, uris, requestOption);
  }
}

void createRequestGroupForUriList
(std::deque<SharedHandle<RequestGroup> >& result, Option* op)
{
  if(op->get(PREF_INPUT_FILE) == "-") {
    createRequestGroupForUriList(result, op, std::cin);
  } else {
    if(!File(op->get(PREF_INPUT_FILE)).isFile()) {
      throw FatalException
	(StringFormat(EX_FILE_OPEN, op->get(PREF_INPUT_FILE).c_str(),
		      "No such file").str());
    }
    std::ifstream f(op->get(PREF_INPUT_FILE).c_str());
    createRequestGroupForUriList(result, op, f);
  }
}

} // namespace aria2
