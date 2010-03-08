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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <ostream>
#include <fstream>
#include <numeric>
#include <algorithm>

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
#include "FeedbackURISelector.h"
#include "InOrderURISelector.h"
#include "AdaptiveURISelector.h"
#include "Option.h"
#include "prefs.h"
#include "File.h"
#include "util.h"
#include "Command.h"
#include "FileEntry.h"
#include "StringFormat.h"

namespace aria2 {

RequestGroupMan::RequestGroupMan
(const std::vector<SharedHandle<RequestGroup> >& requestGroups,
 unsigned int maxSimultaneousDownloads,
 const Option* option):
  _reservedGroups(requestGroups.begin(), requestGroups.end()),
  _logger(LogFactory::getInstance()),
  _maxSimultaneousDownloads(maxSimultaneousDownloads),
  _gidCounter(0),
  _option(option),
  _serverStatMan(new ServerStatMan()),
  _maxOverallDownloadSpeedLimit
  (option->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)),
  _maxOverallUploadSpeedLimit(option->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT)),
  _xmlRpc(option->getAsBool(PREF_ENABLE_XML_RPC)),
  _queueCheck(true)
{}

bool RequestGroupMan::downloadFinished()
{
#ifdef ENABLE_XML_RPC
  if(_xmlRpc) {
    return false;
  }
#endif // ENABLE_XML_RPC
  return _requestGroups.empty() && _reservedGroups.empty();
}

void RequestGroupMan::addRequestGroup
(const SharedHandle<RequestGroup>& group)
{
  _requestGroups.push_back(group);
}

void RequestGroupMan::addReservedGroup
(const std::vector<SharedHandle<RequestGroup> >& groups)
{
  if(_reservedGroups.empty()) {
    requestQueueCheck();
  }
  _reservedGroups.insert(_reservedGroups.end(), groups.begin(), groups.end());
}

void RequestGroupMan::addReservedGroup
(const SharedHandle<RequestGroup>& group)
{
  if(_reservedGroups.empty()) {
    requestQueueCheck();
  }
  _reservedGroups.push_back(group);
}

void RequestGroupMan::insertReservedGroup
(size_t pos, const std::vector<SharedHandle<RequestGroup> >& groups)
{
  if(_reservedGroups.empty()) {
    requestQueueCheck();
  }
  _reservedGroups.insert
    (_reservedGroups.begin()+std::min(_reservedGroups.size(), pos),
     groups.begin(), groups.end());
}

void RequestGroupMan::insertReservedGroup
(size_t pos, const SharedHandle<RequestGroup>& group)
{
  if(_reservedGroups.empty()) {
    requestQueueCheck();
  }
  _reservedGroups.insert
    (_reservedGroups.begin()+std::min(_reservedGroups.size(), pos), group);
}

size_t RequestGroupMan::countRequestGroup() const
{
  return _requestGroups.size();
}

SharedHandle<RequestGroup> RequestGroupMan::getRequestGroup(size_t index) const
{
  if(index < _requestGroups.size()) {
    return _requestGroups[index];
  } else {
    return SharedHandle<RequestGroup>();
  }
}

template<typename Iterator>
static Iterator findByGID(Iterator first, Iterator last, int32_t gid)
{
  for(; first != last; ++first) {
    if((*first)->getGID() == gid) {
      return first;
    }
  }
  return first;
}

SharedHandle<RequestGroup>
RequestGroupMan::findRequestGroup(int32_t gid) const
{
  std::deque<SharedHandle<RequestGroup> >::const_iterator i =
    findByGID(_requestGroups.begin(), _requestGroups.end(), gid);
  if(i == _requestGroups.end()) {
    return SharedHandle<RequestGroup>();
  } else {
    return *i;
  }
}

SharedHandle<RequestGroup>
RequestGroupMan::findReservedGroup(int32_t gid) const
{
  std::deque<SharedHandle<RequestGroup> >::const_iterator i =
    findByGID(_reservedGroups.begin(), _reservedGroups.end(), gid);
  if(i == _reservedGroups.end()) {
    return SharedHandle<RequestGroup>();
  } else {
    return *i;
  }
}

size_t RequestGroupMan::changeReservedGroupPosition
(int32_t gid, int pos, HOW how)
{
  std::deque<SharedHandle<RequestGroup> >::iterator i =
    findByGID(_reservedGroups.begin(), _reservedGroups.end(), gid);
  if(i == _reservedGroups.end()) {
    throw DL_ABORT_EX
      (StringFormat("GID#%d not found in the waiting queue.", gid).str());
  }
  SharedHandle<RequestGroup> rg = *i;
  const size_t maxPos = _reservedGroups.size()-1;
  if(how == POS_SET) {
    if(pos < 0) {
      pos = 0;
    } else if(pos > 0) {
      pos = std::min(maxPos, (size_t)pos);
    }
  } else if(how == POS_CUR) {
    size_t abspos = std::distance(_reservedGroups.begin(), i);
    if(pos < 0) {
      int dist = -std::distance(_reservedGroups.begin(), i);
      pos = abspos+std::max(pos, dist);
    } else if(pos > 0) {
      int dist = std::distance(i, _reservedGroups.end())-1;
      pos = abspos+std::min(pos, dist);
    } else {
      pos = abspos;
    }
  } else if(how == POS_END) {
    if(pos >= 0) {
      pos = maxPos;
    } else {
      pos = maxPos-std::min(maxPos, (size_t)-pos);
    }
  }
  if(std::distance(_reservedGroups.begin(), i) < pos) {
    std::rotate(i, i+1, _reservedGroups.begin()+pos+1);
  } else {
    std::rotate(_reservedGroups.begin()+pos, i, i+1);
  }
  return pos;
}

bool RequestGroupMan::removeReservedGroup(int32_t gid)
{
  std::deque<SharedHandle<RequestGroup> >::iterator i =
    findByGID(_reservedGroups.begin(), _reservedGroups.end(), gid);
  if(i == _reservedGroups.end()) {
    return false;
  } else {
    _reservedGroups.erase(i);
    return true;
  }
}

static void executeHook(const std::string& command, int gid)
{
  LogFactory::getInstance()->info("Executing user command: %s %d",
                                  command.c_str(), gid);
#ifndef __MINGW32__
  pid_t cpid = fork();
  if(cpid == -1) {
    LogFactory::getInstance()->error("fork() failed."
                                     " Cannot execute user command.");
  } else if(cpid == 0) {
    execl(command.c_str(), command.c_str(), util::itos(gid).c_str(),
          reinterpret_cast<char*>(0));
    perror(("Could not execute user command: "+command).c_str());
    exit(EXIT_FAILURE);
  }
#else
  PROCESS_INFORMATION pi;
  STARTUPINFO si;

  memset(&si, 0, sizeof (si));
  si.cb = sizeof(STARTUPINFO);

  memset(&pi, 0, sizeof (pi));

  std::string cmdline = command;
  strappend(cmdline, " ", util::itos(gid));

  DWORD rc = CreateProcess(
                           NULL,
                           (LPSTR)cmdline.c_str(),
                           NULL,
                           NULL,
                           true,
                           NULL,
                           NULL,
                           0,
                           &si,
                           &pi);

  if(!rc)
    LogFactory::getInstance()->error("CreateProcess() failed."
                                     " Cannot execute user command.");
#endif 
}

static void executeStartHook
(const SharedHandle<RequestGroup>& group, const Option* option)
{
  if(!option->blank(PREF_ON_DOWNLOAD_START)) {
    executeHook(option->get(PREF_ON_DOWNLOAD_START), group->getGID());
  }
}

static void executeStopHook
(const SharedHandle<DownloadResult>& result, const Option* option)
{
  if(result->result == downloadresultcode::FINISHED &&
     !option->blank(PREF_ON_DOWNLOAD_COMPLETE)) {
    executeHook(option->get(PREF_ON_DOWNLOAD_COMPLETE), result->gid);
  } else if(result->result != downloadresultcode::IN_PROGRESS &&
            !option->blank(PREF_ON_DOWNLOAD_ERROR)) {
    executeHook(option->get(PREF_ON_DOWNLOAD_ERROR), result->gid);
  } else if(!option->blank(PREF_ON_DOWNLOAD_STOP)) {
    executeHook(option->get(PREF_ON_DOWNLOAD_STOP), result->gid);
  }
}

class ProcessStoppedRequestGroup {
private:
  DownloadEngine* _e;
  std::deque<SharedHandle<DownloadResult> >& _downloadResults;
  Logger* _logger;

  void saveSignature(const SharedHandle<RequestGroup>& group)
  {
    SharedHandle<Signature> sig =
      group->getDownloadContext()->getSignature();
    if(!sig.isNull() && !sig->getBody().empty()) {
      // filename of signature file is the path to download file followed by
      // ".sig".
      std::string signatureFile = group->getFirstFilePath()+".sig";
      if(sig->save(signatureFile)) {
        _logger->notice(MSG_SIGNATURE_SAVED, signatureFile.c_str());
      } else {
        _logger->notice(MSG_SIGNATURE_NOT_SAVED, signatureFile.c_str());
      }
    }
  }
public:
  ProcessStoppedRequestGroup
  (DownloadEngine* e,
   std::deque<SharedHandle<DownloadResult> >& downloadResults):
    _e(e),
    _downloadResults(downloadResults),
    _logger(LogFactory::getInstance()) {}

  void operator()(const SharedHandle<RequestGroup>& group)
  {
    if(group->getNumCommand() == 0) {
      SharedHandle<DownloadContext> dctx = group->getDownloadContext();
      // DownloadContext::resetDownloadStopTime() is only called when
      // download completed. If
      // DownloadContext::getDownloadStopTime().isZero() is true, then
      // there is a possibility that the download is error or
      // in-progress and resetDownloadStopTime() is not called. So
      // call it here.
      if(dctx->getDownloadStopTime().isZero()) {
        dctx->resetDownloadStopTime();
      }
      try {
        group->closeFile();
        if(group->downloadFinished()) {
          group->applyLastModifiedTimeToLocalFiles();
          group->reportDownloadFinished();
          if(group->allDownloadFinished()) {
            group->removeControlFile();
            saveSignature(group);
          } else {
            group->saveControlFile();
          }
          std::vector<SharedHandle<RequestGroup> > nextGroups;
          group->postDownloadProcessing(nextGroups);
          if(!nextGroups.empty()) {
            if(_logger->debug()) {
              _logger->debug
                ("Adding %lu RequestGroups as a result of PostDownloadHandler.",
                 static_cast<unsigned long>(nextGroups.size()));
            }
            _e->_requestGroupMan->insertReservedGroup(0, nextGroups);
          }
        } else {
          group->saveControlFile();
        }
      } catch(RecoverableException& ex) {
        _logger->error(EX_EXCEPTION_CAUGHT, ex);
      }
      group->releaseRuntimeResource(_e);
      _downloadResults.push_back(group->createDownloadResult());

      executeStopHook(_downloadResults.back(), _e->option);
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
        bool singleConnection =
          group->getSegmentMan()->getPeerStats().size() == 1;
        const std::vector<SharedHandle<PeerStat> >& peerStats =
          group->getSegmentMan()->getFastestPeerStats();
        for(std::vector<SharedHandle<PeerStat> >::const_iterator i =
              peerStats.begin(), eoi = peerStats.end(); i != eoi; ++i) {
          if((*i)->getHostname().empty() || (*i)->getProtocol().empty()) {
            continue;
          }
          int speed = (*i)->getAvgDownloadSpeed();
          if (speed == 0) continue;

          SharedHandle<ServerStat> ss =
            _requestGroupMan->getOrCreateServerStat((*i)->getHostname(),
                                                    (*i)->getProtocol());
          ss->increaseCounter();
          ss->updateDownloadSpeed(speed);
          if(singleConnection) {
            ss->updateSingleConnectionAvgSpeed(speed);
          }
          else {
            ss->updateMultiConnectionAvgSpeed(speed);
          }
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

void RequestGroupMan::removeStoppedGroup(DownloadEngine* e)
{
  size_t numPrev = _requestGroups.size();

  updateServerStat();

  std::for_each(_requestGroups.begin(), _requestGroups.end(),
                ProcessStoppedRequestGroup(e, _downloadResults));

  std::deque<SharedHandle<RequestGroup> >::iterator i =
    std::remove_if(_requestGroups.begin(),
                   _requestGroups.end(),
                   FindStoppedRequestGroup());
  if(i != _requestGroups.end()) {
    _requestGroups.erase(i, _requestGroups.end());
  }

  size_t numRemoved = numPrev-_requestGroups.size();
  if(numRemoved > 0) {
    if(_logger->debug()) {
      _logger->debug("%lu RequestGroup(s) deleted.",
                     static_cast<unsigned long>(numRemoved));
    }
  }
}

void RequestGroupMan::configureRequestGroup
(const SharedHandle<RequestGroup>& requestGroup) const
{
  const std::string& uriSelectorValue = _option->get(PREF_URI_SELECTOR);
  if(uriSelectorValue == V_FEEDBACK) {
    requestGroup->setURISelector
      (SharedHandle<URISelector>(new FeedbackURISelector(_serverStatMan)));
  } else if(uriSelectorValue == V_INORDER) {
    requestGroup->setURISelector
      (SharedHandle<URISelector>(new InOrderURISelector()));
  } else if(uriSelectorValue == V_ADAPTIVE) {
    requestGroup->setURISelector
      (SharedHandle<URISelector>(new AdaptiveURISelector(_serverStatMan,
                                                         requestGroup.get())));
  }
}

static void createInitialCommand(const SharedHandle<RequestGroup>& requestGroup,
                                 std::vector<Command*>& commands,
                                 DownloadEngine* e)
{
  requestGroup->createInitialCommand(commands, e);
}

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  removeStoppedGroup(e);
  if(_maxSimultaneousDownloads <= _requestGroups.size()) {
    return;
  }
  std::vector<SharedHandle<RequestGroup> > temp;
  unsigned int count = 0;
  size_t num = _maxSimultaneousDownloads-_requestGroups.size();
  while(count < num && !_reservedGroups.empty()) {
    SharedHandle<RequestGroup> groupToAdd = _reservedGroups.front();
    _reservedGroups.pop_front();
    std::vector<Command*> commands;
    try {
      if(!groupToAdd->isDependencyResolved()) {
        temp.push_back(groupToAdd);
        continue;
      }
      configureRequestGroup(groupToAdd);
      createInitialCommand(groupToAdd, commands, e);
      groupToAdd->setRequestGroupMan(this);
      if(commands.empty()) {
        requestQueueCheck();
      }
      _requestGroups.push_back(groupToAdd);
      ++count;
      e->addCommand(commands);
      commands.clear();
      executeStartHook(groupToAdd, e->option);
    } catch(RecoverableException& ex) {
      _logger->error(EX_EXCEPTION_CAUGHT, ex);
      if(_logger->debug()) {
        _logger->debug("Deleting temporal commands.");
      }
      std::for_each(commands.begin(), commands.end(), Deleter());
      commands.clear();
      if(_logger->debug()) {
        _logger->debug("Commands deleted");
      }
      groupToAdd->releaseRuntimeResource(e);
      _downloadResults.push_back(groupToAdd->createDownloadResult());
    }
  }
  if(!temp.empty()) {
    _reservedGroups.insert(_reservedGroups.begin(), temp.begin(), temp.end());
  }
  if(count > 0) {
    e->setNoWait(true);
    if(_logger->debug()) {
      _logger->debug("%d RequestGroup(s) added.", count);
    }
  }
}

void RequestGroupMan::save()
{
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
        _requestGroups.begin(), eoi = _requestGroups.end(); itr != eoi; ++itr) {
    if((*itr)->allDownloadFinished()) {
      (*itr)->removeControlFile();
    } else {
      try {
        (*itr)->saveControlFile();
      } catch(RecoverableException& e) {
        _logger->error(EX_EXCEPTION_CAUGHT, e);
      }
    }
  }
}

void RequestGroupMan::closeFile()
{
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
        _requestGroups.begin(), eoi = _requestGroups.end(); itr != eoi; ++itr) {
    (*itr)->closeFile();
  }
}

RequestGroupMan::DownloadStat RequestGroupMan::getDownloadStat() const
{
  size_t finished = 0;
  size_t error = 0;
  size_t inprogress = 0;
  downloadresultcode::RESULT lastError = downloadresultcode::FINISHED;
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr =
        _downloadResults.begin(), eoi = _downloadResults.end();
      itr != eoi; ++itr) {
    if((*itr)->result == downloadresultcode::FINISHED) {
      ++finished;
    } else {
      ++error;
      lastError = (*itr)->result;
    }
  }
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
        _requestGroups.begin(), eoi = _requestGroups.end(); itr != eoi; ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    if(result->result == downloadresultcode::FINISHED) {
      ++finished;
    } else {
      ++inprogress;
    }
  }
  return DownloadStat(finished, error, inprogress, _reservedGroups.size(),
                      lastError);
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
    << "gid|stat|avg speed  |path/URI" << "\n"
    << "===+====+===========+";
#ifdef __MINGW32__
  int pathRowSize = 58;
#else // !__MINGW32__
  int pathRowSize = 59;
#endif // !__MINGW32__
  o << std::setfill('=') << std::setw(pathRowSize) << '=' << "\n";
  int ok = 0;
  int err = 0;
  int inpr = 0;

  for(std::deque<SharedHandle<DownloadResult> >::const_iterator itr =
        _downloadResults.begin(), eoi = _downloadResults.end();
      itr != eoi; ++itr) {
    std::string status;
    if((*itr)->result == downloadresultcode::FINISHED) {
      status = MARK_OK;
      ++ok;
    } else if((*itr)->result == downloadresultcode::IN_PROGRESS) {
      status = MARK_INPR;
      ++inpr;
    } else {
      status = MARK_ERR;
      ++err;
    }
    o << formatDownloadResult(status, *itr) << "\n";
  }
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
        _requestGroups.begin(), eoi = _requestGroups.end(); itr != eoi; ++itr) {
    DownloadResultHandle result = (*itr)->createDownloadResult();
    std::string status;
    if(result->result == downloadresultcode::FINISHED) {
      status = MARK_OK;
      ++ok;
    } else {
      // Since this RequestGroup is not processed by ProcessStoppedRequestGroup,
      // its download stop time is not reseted.
      // Reset download stop time and assign sessionTime here.
      (*itr)->getDownloadContext()->resetDownloadStopTime();
      result->sessionTime =
        (*itr)->getDownloadContext()->calculateSessionTime();
      status = MARK_INPR;
      ++inpr;
    }
    o << formatDownloadResult(status, result) << "\n";
  }
  if(ok > 0 || err > 0 || inpr > 0) {
    o << "\n"
      << _("Status Legend:") << "\n";

    if(ok > 0) {
      o << " (OK):download completed.";
    }
    if(err > 0) {
      o << "(ERR):error occurred.";
    }
    if(inpr > 0) {
      o << "(INPR):download in-progress.";
    }
    o << "\n";
  }
}

std::string RequestGroupMan::formatDownloadResult(const std::string& status, const DownloadResultHandle& downloadResult) const
{
  std::stringstream o;
  o << std::setw(3) << downloadResult->gid << "|"
    << std::setw(4) << status << "|"
    << std::setw(11);
  if(downloadResult->sessionTime > 0) {
    o << util::abbrevSize
      (downloadResult->sessionDownloadLength*1000/downloadResult->sessionTime)+
      "B/s";
  } else {
    o << "n/a";
  }
  o << "|";
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    downloadResult->fileEntries;
  writeFilePath(fileEntries.begin(), fileEntries.end(), o,
                downloadResult->inMemoryDownload);
  return o.str();
}

template<typename StringInputIterator, typename FileEntryInputIterator>
static bool sameFilePathExists(StringInputIterator sfirst,
                               StringInputIterator slast,
                               FileEntryInputIterator ffirst,
                               FileEntryInputIterator flast)
{
  for(; ffirst != flast; ++ffirst) {
    if(std::binary_search(sfirst, slast, (*ffirst)->getPath())) {
      return true;
    }
  }
  return false;
}

bool RequestGroupMan::isSameFileBeingDownloaded(RequestGroup* requestGroup) const
{
  // TODO it may be good to use dedicated method rather than use
  // isPreLocalFileCheckEnabled
  if(!requestGroup->isPreLocalFileCheckEnabled()) {
    return false;
  }
  std::vector<std::string> files;
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator itr =
        _requestGroups.begin(), eoi = _requestGroups.end(); itr != eoi; ++itr) {
    if((*itr).get() != requestGroup) {
      const std::vector<SharedHandle<FileEntry> >& entries =
        (*itr)->getDownloadContext()->getFileEntries();
      std::transform(entries.begin(), entries.end(),
                     std::back_inserter(files),
                     mem_fun_sh(&FileEntry::getPath));
    }
  }
  std::sort(files.begin(), files.end());
  const std::vector<SharedHandle<FileEntry> >& entries =
    requestGroup->getDownloadContext()->getFileEntries();
  return sameFilePathExists(files.begin(), files.end(),
                            entries.begin(), entries.end());
}

void RequestGroupMan::halt()
{
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        _requestGroups.begin(), eoi = _requestGroups.end(); i != eoi; ++i) {
    (*i)->setHaltRequested(true);
  }
}

void RequestGroupMan::forceHalt()
{
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        _requestGroups.begin(), eoi = _requestGroups.end(); i != eoi; ++i) {
    (*i)->setForceHaltRequested(true);
  }
}

TransferStat RequestGroupMan::calculateStat()
{
  TransferStat s;
  for(std::deque<SharedHandle<RequestGroup> >::const_iterator i =
        _requestGroups.begin(), eoi = _requestGroups.end(); i != eoi; ++i) {
    s += (*i)->calculateStat();
  }
  return s;
}

SharedHandle<DownloadResult>
RequestGroupMan::findDownloadResult(int32_t gid) const
{
  for(std::deque<SharedHandle<DownloadResult> >::const_iterator i =
        _downloadResults.begin(), eoi = _downloadResults.end(); i != eoi; ++i) {
    if((*i)->gid == gid) {
      return *i;
    }
  }
  return SharedHandle<DownloadResult>();
}

void RequestGroupMan::purgeDownloadResult()
{
  _downloadResults.clear();
}

SharedHandle<ServerStat>
RequestGroupMan::findServerStat(const std::string& hostname,
                                const std::string& protocol) const
{
  return _serverStatMan->find(hostname, protocol);
}

SharedHandle<ServerStat>
RequestGroupMan::getOrCreateServerStat(const std::string& hostname,
                                       const std::string& protocol)
{
  SharedHandle<ServerStat> ss = findServerStat(hostname, protocol);
  if(ss.isNull()) {
    ss.reset(new ServerStat(hostname, protocol));
    addServerStat(ss);
  }
  return ss;
}

bool RequestGroupMan::addServerStat(const SharedHandle<ServerStat>& serverStat)
{
  return _serverStatMan->add(serverStat);
}

bool RequestGroupMan::loadServerStat(const std::string& filename)
{
  std::ifstream in(filename.c_str(), std::ios::binary);
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
  std::string tempfile = filename;
  tempfile += "__temp";
  {
    std::ofstream out(tempfile.c_str(), std::ios::binary);
    if(!out) {
      _logger->error(MSG_OPENING_WRITABLE_SERVER_STAT_FILE_FAILED,
                     filename.c_str());
      return false;
    }
    if(!_serverStatMan->save(out)) {
      _logger->error(MSG_WRITING_SERVER_STAT_FILE_FAILED, filename.c_str());
      return false;
    }
  }
  if(File(tempfile).renameTo(filename)) {
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

bool RequestGroupMan::doesOverallDownloadSpeedExceed()
{
  return _maxOverallDownloadSpeedLimit > 0 &&
    _maxOverallDownloadSpeedLimit < calculateStat().getDownloadSpeed();
}

bool RequestGroupMan::doesOverallUploadSpeedExceed()
{
  return _maxOverallUploadSpeedLimit > 0 &&
    _maxOverallUploadSpeedLimit < calculateStat().getUploadSpeed();
}

} // namespace aria2
