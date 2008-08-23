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
#include "RequestGroupMan.h"
#include "BtProgressInfoFile.h"
#include "RecoverableException.h"
#include "RequestGroup.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DownloadEngine.h"
#include "message.h"
#include "a2functional.h"
#include "DownloadResult.h"
#include "DownloadContext.h"
#include "ServerStatMan.h"
#include "ServerStat.h"
#include "PeerStat.h"
#include "SegmentMan.h"
#include "ServerStatURISelector.h"
#include "InOrderURISelector.h"
#include "Option.h"
#include "prefs.h"
#include "File.h"
#include <iomanip>
#include <sstream>
#include <ostream>
#include <fstream>
#include <numeric>
#include <algorithm>

namespace aria2 {

RequestGroupMan::RequestGroupMan(const RequestGroups& requestGroups,
				 unsigned int maxSimultaneousDownloads,
				 const Option* option):
  _requestGroups(requestGroups),
  _logger(LogFactory::getInstance()),
  _maxSimultaneousDownloads(maxSimultaneousDownloads),
  _gidCounter(0),
  _option(option),
  _serverStatMan(new ServerStatMan()) {}

bool RequestGroupMan::downloadFinished()
{
  if(!_reservedGroups.empty()) {
    return false;
  }
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->getNumCommand() > 0 || !(*itr)->downloadFinished()) {
      return false;
    }
  }
  return true;
}

void RequestGroupMan::addRequestGroup(const RequestGroupHandle& group)
{
  _requestGroups.push_back(group);
}

void RequestGroupMan::addReservedGroup(const RequestGroups& groups)
{
  _reservedGroups.insert(_reservedGroups.end(), groups.begin(), groups.end());
}

void RequestGroupMan::addReservedGroup(const RequestGroupHandle& group)
{
  _reservedGroups.push_back(group);
}

size_t RequestGroupMan::countRequestGroup() const
{
  return _requestGroups.size();
}

RequestGroupHandle RequestGroupMan::getRequestGroup(size_t index) const
{
  if(index < _requestGroups.size()) {
    return _requestGroups[index];
  } else {
    return SharedHandle<RequestGroup>();
  }
}

const std::deque<SharedHandle<RequestGroup> >&
RequestGroupMan::getRequestGroups() const
{
  return _requestGroups;
}

class ProcessStoppedRequestGroup {
private:
  std::deque<SharedHandle<RequestGroup> >& _reservedGroups;
  std::deque<SharedHandle<DownloadResult> >& _downloadResults;
  Logger* _logger;

  void saveSignature(const SharedHandle<RequestGroup>& group)
  {
    SharedHandle<Signature> sig =
      group->getDownloadContext()->getSignature();
    if(!sig.isNull() && !sig->getBody().empty()) {
      // filename of signature file is the path to download file followed by
      // ".sig".
      std::string signatureFile = group->getFilePath()+".sig";
      if(sig->save(signatureFile)) {
	_logger->notice(MSG_SIGNATURE_SAVED, signatureFile.c_str());
      } else {
	_logger->notice(MSG_SIGNATURE_NOT_SAVED, signatureFile.c_str());
      }
    }
  }
public:
  ProcessStoppedRequestGroup
  (std::deque<SharedHandle<RequestGroup> >& reservedGroups,
   std::deque<SharedHandle<DownloadResult> >& downloadResults):
    _reservedGroups(reservedGroups),
    _downloadResults(downloadResults),
    _logger(LogFactory::getInstance()) {}

  void operator()(const SharedHandle<RequestGroup>& group)
  {
    if(group->getNumCommand() == 0) {
      try {
	group->closeFile();      
	if(group->downloadFinished()) {
	  group->reportDownloadFinished();
	  if(group->allDownloadFinished()) {
	    group->getProgressInfoFile()->removeFile();
	    saveSignature(group);
	  } else {
	    group->getProgressInfoFile()->save();
	  }
	  RequestGroups nextGroups;
	  group->postDownloadProcessing(nextGroups);
	  if(!nextGroups.empty()) {
	    _logger->debug("Adding %zu RequestGroups as a result of PostDownloadHandler.", nextGroups.size());
	    _reservedGroups.insert(_reservedGroups.begin(),
				   nextGroups.begin(), nextGroups.end());
	  }
	} else {
	  group->getProgressInfoFile()->save();
	}
      } catch(RecoverableException& ex) {
	_logger->error(EX_EXCEPTION_CAUGHT, ex);
      }
      group->releaseRuntimeResource();
      _downloadResults.push_back(group->createDownloadResult());
    }
  }
};

class CollectServerStat {
private:
  RequestGroupMan* _requestGroupMan;
public:
  CollectServerStat(RequestGroupMan* requestGroupMan):
    _requestGroupMan(requestGroupMan) {}

  void operator()(const SharedHandle<RequestGroup>& group)
  {
    if(group->getNumCommand() == 0) {
      // Collect statistics during download in PeerStats and update/register
      // ServerStatMan
      if(!group->getSegmentMan().isNull()) {
	const std::deque<SharedHandle<PeerStat> >& peerStats =
	  group->getSegmentMan()->getPeerStats();
	for(std::deque<SharedHandle<PeerStat> >::const_iterator i =
	      peerStats.begin(); i != peerStats.end(); ++i) {
	  if((*i)->getHostname().empty() || (*i)->getProtocol().empty()) {
	    continue;
	  }
	  SharedHandle<ServerStat> ss =
	    _requestGroupMan->findServerStat((*i)->getHostname(),
					     (*i)->getProtocol());
	  if(ss.isNull()) {
	    ss.reset(new ServerStat((*i)->getHostname(),
				    (*i)->getProtocol()));
	    _requestGroupMan->addServerStat(ss);
	  }
	  ss->updateDownloadSpeed((*i)->getAvgDownloadSpeed());
	}
      }
    }    
  }
};

class FindStoppedRequestGroup {
public:
  bool operator()(const SharedHandle<RequestGroup>& group) {
    return group->getNumCommand() == 0;
  }
};

void RequestGroupMan::updateServerStat()
{
  std::for_each(_requestGroups.begin(), _requestGroups.end(),
		CollectServerStat(this));
}

void RequestGroupMan::removeStoppedGroup()
{
  size_t numPrev = _requestGroups.size();

  updateServerStat();

  std::for_each(_requestGroups.begin(), _requestGroups.end(),
		ProcessStoppedRequestGroup(_reservedGroups, _downloadResults));

  _requestGroups.erase(std::remove_if(_requestGroups.begin(),
				      _requestGroups.end(),
				      FindStoppedRequestGroup()),
		       _requestGroups.end());

  size_t numRemoved = numPrev-_requestGroups.size();
  if(numRemoved > 0) {
    _logger->debug("%zu RequestGroup(s) deleted.", numRemoved);
  }
}

void RequestGroupMan::configureRequestGroup
(const SharedHandle<RequestGroup>& requestGroup) const
{
  const std::string& uriSelectorValue = _option->get(PREF_URI_SELECTOR);
  if(uriSelectorValue == V_FEEDBACK) {
    requestGroup->setURISelector
      (SharedHandle<URISelector>(new ServerStatURISelector(_serverStatMan)));
  } else if(uriSelectorValue == V_INORDER) {
    requestGroup->setURISelector
      (SharedHandle<URISelector>(new InOrderURISelector()));
  }
}

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  RequestGroups temp;
  removeStoppedGroup();
  unsigned int count = 0;
  for(int num = _maxSimultaneousDownloads-_requestGroups.size();
      num > 0 && _reservedGroups.size() > 0; --num) {
    RequestGroupHandle groupToAdd = _reservedGroups.front();
    _reservedGroups.pop_front();
    try {
      if(!groupToAdd->isDependencyResolved()) {
	temp.push_back(groupToAdd);
	continue;
      }
      configureRequestGroup(groupToAdd);
      Commands commands;
      groupToAdd->createInitialCommand(commands, e);
      _requestGroups.push_back(groupToAdd);
      ++count;
      e->addCommand(commands);
    } catch(RecoverableException& ex) {
      _logger->error(EX_EXCEPTION_CAUGHT, ex);
      _downloadResults.push_back(groupToAdd->createDownloadResult());
    }
  }
  _reservedGroups.insert(_reservedGroups.begin(), temp.begin(), temp.end());
  if(count > 0) {
    e->setNoWait(true);
    _logger->debug("%d RequestGroup(s) added.", count);
  }
}

void RequestGroupMan::getInitialCommands(std::deque<Command*>& commands,
					 DownloadEngine* e)
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
	itr != _requestGroups.end();) {
    try {
      if((*itr)->isDependencyResolved()) {
	configureRequestGroup(*itr);
	(*itr)->createInitialCommand(commands, e);
	++itr;
      } else {
	_reservedGroups.push_front((*itr));
	itr = _requestGroups.erase(itr);
      }
    } catch(RecoverableException& e) {
      _logger->error(EX_EXCEPTION_CAUGHT, e);
      _downloadResults.push_back((*itr)->createDownloadResult());
      itr = _requestGroups.erase(itr);
    }  
  }
}

void RequestGroupMan::save()
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr)->allDownloadFinished()) {
      (*itr)->getProgressInfoFile()->removeFile();
    } else {
      try {
	(*itr)->getProgressInfoFile()->save();
      } catch(RecoverableException& e) {
	_logger->error(EX_EXCEPTION_CAUGHT, e);
      }
    }
  }
}

void RequestGroupMan::closeFile()
{
  for(RequestGroups::iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->closeFile();
  }
}

RequestGroupMan::DownloadStat RequestGroupMan::getDownloadStat() const
{
  DownloadStat stat;
  size_t finished = 0;
  size_t error = 0;
  size_t inprogress = 0;
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr = _downloadResults.begin();
      itr != _downloadResults.end(); ++itr) {
    if((*itr)->result == DownloadResult::FINISHED) {
      ++finished;
    } else {
      ++error;
    }
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    if(result->result == DownloadResult::FINISHED) {
      ++finished;
    } else {
      ++inprogress;
    }
  }
  stat.setCompleted(finished);
  stat.setError(error);
  stat.setInProgress(inprogress);
  stat.setWaiting(_reservedGroups.size());
  return stat;
}

void RequestGroupMan::showDownloadResults(std::ostream& o) const
{
  static const std::string MARK_OK("OK");
  static const std::string MARK_ERR("ERR");
  static const std::string MARK_INPR("INPR");

  // Download Results:
  // idx|stat|path/length
  // ===+====+=======================================================================
  o << "\n"
    <<_("Download Results:") << "\n"
    << "gid|stat|path/URI" << "\n"
    << "===+====+======================================================================" << "\n";
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr = _downloadResults.begin();
      itr != _downloadResults.end(); ++itr) {
    o << formatDownloadResult((*itr)->result == DownloadResult::FINISHED ?
			      MARK_OK : MARK_ERR, *itr) << "\n";
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    o << formatDownloadResult(result->result == DownloadResult::FINISHED ?
			      MARK_OK : MARK_INPR, result) << "\n";
  }
  o << "\n"
    << _("Status Legend:") << "\n"
    << " (OK):download completed.(ERR):error occurred.(INPR):download in-progress." << "\n";
}

std::string RequestGroupMan::formatDownloadResult(const std::string& status, const DownloadResultHandle& downloadResult) const
{
  std::stringstream o;
  o << std::setw(3) << downloadResult->gid << "|"
    << std::setw(4) << status << "|";
  if(downloadResult->result == DownloadResult::FINISHED) {
    o << downloadResult->filePath;
  } else {
    if(downloadResult->numUri == 0) {
      if(downloadResult->filePath.empty()) {
	o << "n/a";
      } else {
	o << downloadResult->filePath;
      }
    } else {
      o << downloadResult->uri;
      if(downloadResult->numUri > 1) {
	o << " (" << downloadResult->numUri-1 << "more)";
      }
    }
  }
  return o.str();
}

bool RequestGroupMan::isSameFileBeingDownloaded(RequestGroup* requestGroup) const
{
  // TODO it may be good to use dedicated method rather than use isPreLocalFileCheckEnabled
  if(!requestGroup->isPreLocalFileCheckEnabled()) {
    return false;
  }
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    if((*itr).get() != requestGroup &&
       (*itr)->getFilePath() == requestGroup->getFilePath()) {
      return true;
    }
  }
  return false;
}

void RequestGroupMan::halt()
{
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->setHaltRequested(true);
  }
}

void RequestGroupMan::forceHalt()
{
  for(RequestGroups::const_iterator itr = _requestGroups.begin();
      itr != _requestGroups.end(); ++itr) {
    (*itr)->setForceHaltRequested(true);
  }
}

TransferStat RequestGroupMan::calculateStat()
{
  return std::accumulate(_requestGroups.begin(), _requestGroups.end(), TransferStat(),
			 adopt2nd(std::plus<TransferStat>(), mem_fun_sh(&RequestGroup::calculateStat)));
}

const std::deque<SharedHandle<DownloadResult> >&
RequestGroupMan::getDownloadResults() const
{
  return _downloadResults;
}

SharedHandle<ServerStat>
RequestGroupMan::findServerStat(const std::string& hostname,
				const std::string& protocol) const
{
  return _serverStatMan->find(hostname, protocol);
}

bool RequestGroupMan::addServerStat(const SharedHandle<ServerStat>& serverStat)
{
  return _serverStatMan->add(serverStat);
}

bool RequestGroupMan::loadServerStat(const std::string& filename)
{
  std::ifstream in(filename.c_str());
  if(!in) {
    _logger->error(MSG_OPENING_READABLE_SERVER_STAT_FILE_FAILED, filename.c_str());
    return false;
  }
  if(_serverStatMan->load(in)) {
    _logger->notice(MSG_SERVER_STAT_LOADED, filename.c_str());
    return true;
  } else {
    _logger->error(MSG_READING_SERVER_STAT_FILE_FAILED, filename.c_str());
    return false;
  }
}

bool RequestGroupMan::saveServerStat(const std::string& filename) const
{
  std::string tempfile = filename+"__temp";
  std::ofstream out(tempfile.c_str());
  if(!out) {
    _logger->error(MSG_OPENING_WRITABLE_SERVER_STAT_FILE_FAILED,
		   tempfile.c_str());
    return false;
  }
  if(_serverStatMan->save(out) && File(tempfile).renameTo(filename)) {
    _logger->notice(MSG_SERVER_STAT_SAVED, filename.c_str());
    return true;
  } else {
    _logger->error(MSG_WRITING_SERVER_STAT_FILE_FAILED, filename.c_str());
    return false;
  }
}

void RequestGroupMan::removeStaleServerStat(time_t timeout)
{
  _serverStatMan->removeStaleServerStat(timeout);
}

} // namespace aria2
