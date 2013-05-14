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
#include <numeric>
#include <algorithm>
#include <utility>

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
#include "SegmentMan.h"
#include "FeedbackURISelector.h"
#include "InorderURISelector.h"
#include "AdaptiveURISelector.h"
#include "Option.h"
#include "prefs.h"
#include "File.h"
#include "util.h"
#include "Command.h"
#include "FileEntry.h"
#include "fmt.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "Segment.h"
#include "DlAbortEx.h"
#include "uri.h"
#include "Triplet.h"
#include "Signature.h"
#include "OutputFile.h"
#include "download_helper.h"
#include "UriListParser.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#include "PeerStat.h"
#include "WrDiskCache.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace {
template<typename InputIterator>
void appendReservedGroup(RequestGroupList& list,
                         InputIterator first, InputIterator last)
{
  for(; first != last; ++first) {
    list.push_back((*first)->getGID(), *first);
  }
}
} // namespace

RequestGroupMan::RequestGroupMan
(const std::vector<SharedHandle<RequestGroup> >& requestGroups,
 int maxSimultaneousDownloads,
 const Option* option)
  : maxSimultaneousDownloads_(maxSimultaneousDownloads),
    option_(option),
    serverStatMan_(new ServerStatMan()),
    maxOverallDownloadSpeedLimit_
    (option->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT)),
    maxOverallUploadSpeedLimit_(option->getAsInt
                                (PREF_MAX_OVERALL_UPLOAD_LIMIT)),
    keepRunning_(option->getAsBool(PREF_ENABLE_RPC)),
    queueCheck_(true),
    removedErrorResult_(0),
    removedLastErrorResult_(error_code::FINISHED),
    maxDownloadResult_(option->getAsInt(PREF_MAX_DOWNLOAD_RESULT)),
    wrDiskCache_(0)
{
  appendReservedGroup(reservedGroups_,
                      requestGroups.begin(), requestGroups.end());
}

RequestGroupMan::~RequestGroupMan()
{
  delete wrDiskCache_;
}

bool RequestGroupMan::downloadFinished()
{
  if(keepRunning_) {
    return false;
  }
  return requestGroups_.empty() && reservedGroups_.empty();
}

void RequestGroupMan::addRequestGroup
(const SharedHandle<RequestGroup>& group)
{
  requestGroups_.push_back(group->getGID(), group);
}

void RequestGroupMan::addReservedGroup
(const std::vector<SharedHandle<RequestGroup> >& groups)
{
  requestQueueCheck();
  appendReservedGroup(reservedGroups_, groups.begin(), groups.end());
}

void RequestGroupMan::addReservedGroup
(const SharedHandle<RequestGroup>& group)
{
  requestQueueCheck();
  reservedGroups_.push_back(group->getGID(), group);
}

namespace {
struct RequestGroupKeyFunc {
  a2_gid_t operator()(const SharedHandle<RequestGroup>& rg) const
  {
    return rg->getGID();
  }
};
} // namespace

void RequestGroupMan::insertReservedGroup
(size_t pos, const std::vector<SharedHandle<RequestGroup> >& groups)
{
  requestQueueCheck();
  pos = std::min(reservedGroups_.size(), pos);
  reservedGroups_.insert(pos, RequestGroupKeyFunc(),
                         groups.begin(), groups.end());
}

void RequestGroupMan::insertReservedGroup
(size_t pos, const SharedHandle<RequestGroup>& group)
{
  requestQueueCheck();
  pos = std::min(reservedGroups_.size(), pos);
  reservedGroups_.insert(pos, group->getGID(), group);
}

size_t RequestGroupMan::countRequestGroup() const
{
  return requestGroups_.size();
}

SharedHandle<RequestGroup> RequestGroupMan::findGroup(a2_gid_t gid) const
{
  SharedHandle<RequestGroup> rg = requestGroups_.get(gid);
  if(!rg) {
    rg = reservedGroups_.get(gid);
  }
  return rg;
}

size_t RequestGroupMan::changeReservedGroupPosition
(a2_gid_t gid, int pos, OffsetMode how)
{
  ssize_t dest = reservedGroups_.move(gid, pos, how);
  if(dest == -1) {
    throw DL_ABORT_EX(fmt("GID#%s not found in the waiting queue.",
                          GroupId::toHex(gid).c_str()));
  } else {
    return dest;
  }
}

bool RequestGroupMan::removeReservedGroup(a2_gid_t gid)
{
  return reservedGroups_.remove(gid);
}

namespace {

void notifyDownloadEvent
(DownloadEvent event, const SharedHandle<RequestGroup>& group)
{
  // Check NULL to make unit test easier.
  if(SingletonHolder<Notifier>::instance()) {
    SingletonHolder<Notifier>::instance()->notifyDownloadEvent(event, group);
  }
}

} // namespace

namespace {

void executeStopHook
(const SharedHandle<RequestGroup>& group,
 const Option* option,
 error_code::Value result)
{
  if(result == error_code::FINISHED &&
     !option->blank(PREF_ON_DOWNLOAD_COMPLETE)) {
    util::executeHookByOptName(group, option, PREF_ON_DOWNLOAD_COMPLETE);
  } else if(result != error_code::IN_PROGRESS &&
            result != error_code::REMOVED &&
            !option->blank(PREF_ON_DOWNLOAD_ERROR)) {
    util::executeHookByOptName(group, option, PREF_ON_DOWNLOAD_ERROR);
  } else if(!option->blank(PREF_ON_DOWNLOAD_STOP)) {
    util::executeHookByOptName(group, option, PREF_ON_DOWNLOAD_STOP);
  }
  if(result == error_code::FINISHED) {
    notifyDownloadEvent(EVENT_ON_DOWNLOAD_COMPLETE, group);
  } else if(result != error_code::IN_PROGRESS &&
            result != error_code::REMOVED) {
    notifyDownloadEvent(EVENT_ON_DOWNLOAD_ERROR, group);
  } else {
    notifyDownloadEvent(EVENT_ON_DOWNLOAD_STOP, group);
  }
}

} // namespace

namespace {
class ProcessStoppedRequestGroup {
private:
  DownloadEngine* e_;
  RequestGroupList& reservedGroups_;

  void saveSignature(const SharedHandle<RequestGroup>& group)
  {
    SharedHandle<Signature> sig =
      group->getDownloadContext()->getSignature();
    if(sig && !sig->getBody().empty()) {
      // filename of signature file is the path to download file followed by
      // ".sig".
      std::string signatureFile = group->getFirstFilePath()+".sig";
      if(sig->save(signatureFile)) {
        A2_LOG_NOTICE(fmt(MSG_SIGNATURE_SAVED, signatureFile.c_str()));
      } else {
        A2_LOG_NOTICE(fmt(MSG_SIGNATURE_NOT_SAVED, signatureFile.c_str()));
      }
    }
  }

  // Collect statistics during download in PeerStats and update/register
  // ServerStatMan
  void collectStat(const SharedHandle<RequestGroup>& group)
  {
    if(group->getSegmentMan()) {
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
          e_->getRequestGroupMan()->getOrCreateServerStat((*i)->getHostname(),
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
public:
  ProcessStoppedRequestGroup
  (DownloadEngine* e,
   RequestGroupList& reservedGroups)
    : e_(e),
      reservedGroups_(reservedGroups)
  {}

  bool operator()(const RequestGroupList::value_type& group)
  {
    if(group->getNumCommand() == 0) {
      collectStat(group);
      const SharedHandle<DownloadContext>& dctx = group->getDownloadContext();
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
        if(group->isPauseRequested()) {
          A2_LOG_NOTICE
            (fmt(_("Download GID#%s paused"),
                 GroupId::toHex(group->getGID()).c_str()));
          group->saveControlFile();
        } else if(group->downloadFinished() &&
           !group->getDownloadContext()->isChecksumVerificationNeeded()) {
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
            A2_LOG_DEBUG
              (fmt("Adding %lu RequestGroups as a result of"
                   " PostDownloadHandler.",
                   static_cast<unsigned long>(nextGroups.size())));
            e_->getRequestGroupMan()->insertReservedGroup(0, nextGroups);
          }
#ifdef ENABLE_BITTORRENT
          // For in-memory download (e.g., Magnet URI), the
          // FileEntry::getPath() does not return actual file path, so
          // we don't remove it.
          if(group->getOption()->getAsBool(PREF_BT_REMOVE_UNSELECTED_FILE) &&
             !group->inMemoryDownload() &&
             dctx->hasAttribute(CTX_ATTR_BT)) {
            A2_LOG_INFO(fmt(MSG_REMOVING_UNSELECTED_FILE,
                            GroupId::toHex(group->getGID()).c_str()));
            const std::vector<SharedHandle<FileEntry> >& files =
              dctx->getFileEntries();
            for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
                  files.begin(), eoi = files.end(); i != eoi; ++i) {
              if(!(*i)->isRequested()) {
                if(File((*i)->getPath()).remove()) {
                  A2_LOG_INFO(fmt(MSG_FILE_REMOVED, (*i)->getPath().c_str()));
                } else {
                  A2_LOG_INFO(fmt(MSG_FILE_COULD_NOT_REMOVED,
                                  (*i)->getPath().c_str()));
                }
              }
            }
          }
#endif // ENABLE_BITTORRENT
        } else {
          A2_LOG_NOTICE
            (fmt(_("Download GID#%s not complete: %s"),
                 GroupId::toHex(group->getGID()).c_str(),
                 group->getDownloadContext()->getBasePath().c_str()));
          group->saveControlFile();
        }
      } catch(RecoverableException& ex) {
        A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
      }
      if(group->isPauseRequested()) {
        group->setState(RequestGroup::STATE_WAITING);
        reservedGroups_.push_front(group->getGID(), group);
        group->releaseRuntimeResource(e_);
        group->setForceHaltRequested(false);
        util::executeHookByOptName(group, e_->getOption(),
                                   PREF_ON_DOWNLOAD_PAUSE);
        notifyDownloadEvent(EVENT_ON_DOWNLOAD_PAUSE, group);
        // TODO Should we have to prepend spend uris to remaining uris
        // in case PREF_REUSE_URI is disabed?
      } else {
        SharedHandle<DownloadResult> dr = group->createDownloadResult();
        e_->getRequestGroupMan()->addDownloadResult(dr);
        executeStopHook(group, e_->getOption(), dr->result);
        group->releaseRuntimeResource(e_);
      }
      return true;
    } else {
      return false;
    }
  }
};
} // namespace

void RequestGroupMan::removeStoppedGroup(DownloadEngine* e)
{
  size_t numPrev = requestGroups_.size();
  requestGroups_.remove_if(ProcessStoppedRequestGroup(e, reservedGroups_));
  size_t numRemoved = numPrev-requestGroups_.size();
  if(numRemoved > 0) {
    A2_LOG_DEBUG(fmt("%lu RequestGroup(s) deleted.",
                     static_cast<unsigned long>(numRemoved)));
  }
}

void RequestGroupMan::configureRequestGroup
(const SharedHandle<RequestGroup>& requestGroup) const
{
  const std::string& uriSelectorValue =
    requestGroup->getOption()->get(PREF_URI_SELECTOR);
  SharedHandle<URISelector> sel;
  if(uriSelectorValue == V_FEEDBACK) {
    sel.reset(new FeedbackURISelector(serverStatMan_));
  } else if(uriSelectorValue == V_INORDER) {
    sel.reset(new InorderURISelector());
  } else if(uriSelectorValue == V_ADAPTIVE) {
    sel.reset(new AdaptiveURISelector(serverStatMan_, requestGroup.get()));
  }
  if(sel) {
    requestGroup->setURISelector(sel);
  }
}

namespace {
void createInitialCommand(const SharedHandle<RequestGroup>& requestGroup,
                          std::vector<Command*>& commands,
                          DownloadEngine* e)
{
  requestGroup->createInitialCommand(commands, e);
}
} // namespace

void RequestGroupMan::fillRequestGroupFromReserver(DownloadEngine* e)
{
  removeStoppedGroup(e);
  if(static_cast<size_t>(maxSimultaneousDownloads_) <= requestGroups_.size()) {
    return;
  }
  int count = 0;
  int num = maxSimultaneousDownloads_-requestGroups_.size();
  std::vector<SharedHandle<RequestGroup> > pending;

  while(count < num && (uriListParser_ || !reservedGroups_.empty())) {
    if(uriListParser_ && reservedGroups_.empty()) {
      std::vector<SharedHandle<RequestGroup> > groups;
      // May throw exception
      bool ok = createRequestGroupFromUriListParser(groups, option_,
                                                    uriListParser_.get());
      if(ok) {
        appendReservedGroup(reservedGroups_, groups.begin(), groups.end());
      } else {
        uriListParser_.reset();
        if(reservedGroups_.empty()) {
          break;
        }
      }
    }
    SharedHandle<RequestGroup> groupToAdd = *reservedGroups_.begin();
    reservedGroups_.pop_front();
    if((keepRunning_ && groupToAdd->isPauseRequested()) ||
       !groupToAdd->isDependencyResolved()) {
      pending.push_back(groupToAdd);
      continue;
    }
    // Drop pieceStorage here because paused download holds its
    // reference.
    groupToAdd->dropPieceStorage();
    configureRequestGroup(groupToAdd);
    groupToAdd->setRequestGroupMan(this);
    std::vector<Command*> commands;
    try {
      createInitialCommand(groupToAdd, commands, e);
      ++count;
    } catch(RecoverableException& ex) {
      A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
      A2_LOG_DEBUG("Deleting temporal commands.");
      std::for_each(commands.begin(), commands.end(), Deleter());
      commands.clear();
      A2_LOG_DEBUG("Commands deleted");
      groupToAdd->setLastErrorCode(ex.getErrorCode());
      // We add groupToAdd to e later in order to it is processed in
      // removeStoppedGroup().
    }
    if(commands.empty()) {
      requestQueueCheck();
    } else {
      e->addCommand(commands);
    }
    groupToAdd->setState(RequestGroup::STATE_ACTIVE);
    requestGroups_.push_back(groupToAdd->getGID(), groupToAdd);

    util::executeHookByOptName(groupToAdd, e->getOption(),
                               PREF_ON_DOWNLOAD_START);
    notifyDownloadEvent(EVENT_ON_DOWNLOAD_START, groupToAdd);
  }
  if(!pending.empty()) {
    reservedGroups_.insert(reservedGroups_.begin(), RequestGroupKeyFunc(),
                           pending.begin(), pending.end());
  }
  if(count > 0) {
    e->setNoWait(true);
    e->setRefreshInterval(0);
    A2_LOG_DEBUG(fmt("%d RequestGroup(s) added.", count));
  }
}

void RequestGroupMan::save()
{
  for(RequestGroupList::iterator itr = requestGroups_.begin(),
        eoi = requestGroups_.end(); itr != eoi; ++itr) {
    const SharedHandle<RequestGroup>& rg = *itr;
    if(rg->allDownloadFinished() &&
       !rg->getDownloadContext()->isChecksumVerificationNeeded()) {
      rg->removeControlFile();
    } else {
      try {
        rg->saveControlFile();
      } catch(RecoverableException& e) {
        A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
      }
    }
  }
}

void RequestGroupMan::closeFile()
{
  for(RequestGroupList::iterator itr = requestGroups_.begin(),
        eoi = requestGroups_.end(); itr != eoi; ++itr) {
    (*itr)->closeFile();
  }
}

RequestGroupMan::DownloadStat RequestGroupMan::getDownloadStat() const
{
  int finished = 0;
  int error = removedErrorResult_;
  int inprogress = 0;
  int removed = 0;
  error_code::Value lastError = removedLastErrorResult_;
  for(DownloadResultList::const_iterator itr =
        downloadResults_.begin(), eoi = downloadResults_.end(); itr != eoi;
      ++itr) {
    const SharedHandle<DownloadResult>& dr = *itr;
    if(dr->belongsTo != 0) {
      continue;
    }
    if(dr->result == error_code::FINISHED) {
      ++finished;
    } else if(dr->result == error_code::IN_PROGRESS) {
      ++inprogress;
    } else if(dr->result == error_code::REMOVED) {
      ++removed;
    } else {
      ++error;
      lastError = dr->result;
    }
  }
  return DownloadStat(error, inprogress,
                      reservedGroups_.size(),
                      lastError);
}

enum DownloadResultStatus {
  A2_STATUS_OK,
  A2_STATUS_INPR,
  A2_STATUS_RM,
  A2_STATUS_ERR
};

namespace {
const char* getStatusStr(DownloadResultStatus status, bool useColor)
{
  // status string is formatted in 4 characters wide.
  switch(status) {
  case(A2_STATUS_OK):
    if(useColor) {
      return "\033[1;32mOK\033[0m  ";
    } else {
      return "OK  ";
    }
  case(A2_STATUS_INPR):
    if(useColor) {
      return "\033[1;34mINPR\033[0m";
    } else {
      return "INPR";
    }
  case(A2_STATUS_RM):
    if(useColor) {
      return "\033[1mRM\033[0m  ";
    } else {
      return "RM  ";
    }
  case(A2_STATUS_ERR):
    if(useColor) {
      return "\033[1;31mERR\033[0m ";
    } else {
      return "ERR ";
    }
  default:
    return "";
  }
}
} // namespace

void RequestGroupMan::showDownloadResults(OutputFile& o, bool full) const
{
  int pathRowSize = 55;
  // Download Results:
  // idx|stat|path/length
  // ===+====+=======================================================================
  o.printf("\n%s"
           "\ngid   |stat|avg speed  |",
           _("Download Results:"));
  if(full) {
    o.write("  %|path/URI"
            "\n======+====+===========+===+");
    pathRowSize -= 4;
  } else {
    o.write("path/URI"
            "\n======+====+===========+");
  }
  std::string line(pathRowSize, '=');
  o.printf("%s\n", line.c_str());
  bool useColor = o.supportsColor();
  int ok = 0;
  int err = 0;
  int inpr = 0;
  int rm = 0;
  for(DownloadResultList::const_iterator itr =
        downloadResults_.begin(), eoi = downloadResults_.end(); itr != eoi;
      ++itr) {
    const SharedHandle<DownloadResult>& dr = *itr;
    if(dr->belongsTo != 0) {
      continue;
    }
    const char* status;
    switch(dr->result) {
    case error_code::FINISHED:
      status = getStatusStr(A2_STATUS_OK, useColor);
      ++ok;
      break;
    case error_code::IN_PROGRESS:
      status = getStatusStr(A2_STATUS_INPR, useColor);
      ++inpr;
      break;
    case error_code::REMOVED:
      status = getStatusStr(A2_STATUS_RM, useColor);
      ++rm;
      break;
    default:
      status = getStatusStr(A2_STATUS_ERR, useColor);
      ++err;
    }
    if(full) {
      formatDownloadResultFull(o, status, dr);
    } else {
      o.write(formatDownloadResult(status, dr).c_str());
      o.write("\n");
    }
  }
  if(ok > 0 || err > 0 || inpr > 0 || rm > 0) {
    o.printf("\n%s\n", _("Status Legend:"));
    if(ok > 0) {
      o.write(_("(OK):download completed."));
    }
    if(err > 0) {
      o.write(_("(ERR):error occurred."));
    }
    if(inpr > 0) {
      o.write(_("(INPR):download in-progress."));
    }
    if(rm > 0) {
      o.write(_("(RM):download removed."));
    }
    o.write("\n");
  }
}

namespace {
void formatDownloadResultCommon
(std::ostream& o,
 const char* status,
 const SharedHandle<DownloadResult>& downloadResult)
{
  o << std::setw(3) << downloadResult->gid->toAbbrevHex() << "|"
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
}
} // namespace

void RequestGroupMan::formatDownloadResultFull
(OutputFile& out,
 const char* status,
 const SharedHandle<DownloadResult>& downloadResult) const
{
  BitfieldMan bt(downloadResult->pieceLength, downloadResult->totalLength);
  bt.setBitfield(reinterpret_cast<const unsigned char*>
                 (downloadResult->bitfield.data()),
                 downloadResult->bitfield.size());
  bool head = true;
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    downloadResult->fileEntries;
  for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
        fileEntries.begin(), eoi = fileEntries.end(); i != eoi; ++i) {
    if(!(*i)->isRequested()) {
      continue;
    }
    std::stringstream o;
    if(head) {
      formatDownloadResultCommon(o, status, downloadResult);
      head = false;
    } else {
      o << "   |    |           |";
    }
    if((*i)->getLength() == 0 || downloadResult->bitfield.empty()) {
      o << "  -|";
    } else {
      int64_t completedLength =
        bt.getOffsetCompletedLength((*i)->getOffset(), (*i)->getLength());
      o << std::setw(3) << 100*completedLength/(*i)->getLength() << "|";
    }
    writeFilePath(o, *i, downloadResult->inMemoryDownload);
    o << "\n";
    out.write(o.str().c_str());
  }
  if(head) {
    std::stringstream o;
    formatDownloadResultCommon(o, status, downloadResult);
    o << "  -|n/a\n";
    out.write(o.str().c_str());
  }
}

std::string RequestGroupMan::formatDownloadResult
(const char* status,
 const SharedHandle<DownloadResult>& downloadResult) const
{
  std::stringstream o;
  formatDownloadResultCommon(o, status, downloadResult);
  const std::vector<SharedHandle<FileEntry> >& fileEntries =
    downloadResult->fileEntries;
  writeFilePath(fileEntries.begin(), fileEntries.end(), o,
                downloadResult->inMemoryDownload);
  return o.str();
}

namespace {
template<typename StringInputIterator, typename FileEntryInputIterator>
bool sameFilePathExists(StringInputIterator sfirst,
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
} // namespace

bool RequestGroupMan::isSameFileBeingDownloaded(RequestGroup* requestGroup) const
{
  // TODO it may be good to use dedicated method rather than use
  // isPreLocalFileCheckEnabled
  if(!requestGroup->isPreLocalFileCheckEnabled()) {
    return false;
  }
  std::vector<std::string> files;
  for(RequestGroupList::const_iterator itr = requestGroups_.begin(),
        eoi = requestGroups_.end(); itr != eoi; ++itr) {
    const SharedHandle<RequestGroup>& rg = *itr;
    if(rg.get() != requestGroup) {
      const std::vector<SharedHandle<FileEntry> >& entries =
        rg->getDownloadContext()->getFileEntries();
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
  for(RequestGroupList::const_iterator i = requestGroups_.begin(),
        eoi = requestGroups_.end(); i != eoi; ++i) {
    (*i)->setHaltRequested(true);
  }
}

void RequestGroupMan::forceHalt()
{
  for(RequestGroupList::const_iterator i = requestGroups_.begin(),
        eoi = requestGroups_.end(); i != eoi; ++i) {
    (*i)->setForceHaltRequested(true);
  }
}

TransferStat RequestGroupMan::calculateStat()
{
  // TODO Currently, all time upload length is not set.
  return netStat_.toTransferStat();
}

SharedHandle<DownloadResult>
RequestGroupMan::findDownloadResult(a2_gid_t gid) const
{
  return downloadResults_.get(gid);
}

bool RequestGroupMan::removeDownloadResult(a2_gid_t gid)
{
  return downloadResults_.remove(gid);
}

void RequestGroupMan::addDownloadResult(const SharedHandle<DownloadResult>& dr)
{
  bool rv = downloadResults_.push_back(dr->gid->getNumericId(), dr);
  assert(rv);
  while(downloadResults_.size() > maxDownloadResult_){
    DownloadResultList::iterator i = downloadResults_.begin();
    // Save last encountered error code so that we can report it
    // later.
    const SharedHandle<DownloadResult>& dr = *i;
    if(dr->belongsTo == 0 && dr->result != error_code::FINISHED) {
      removedLastErrorResult_ = dr->result;
      ++removedErrorResult_;
    }
    downloadResults_.pop_front();
  }
}

void RequestGroupMan::purgeDownloadResult()
{
  downloadResults_.clear();
}

SharedHandle<ServerStat>
RequestGroupMan::findServerStat(const std::string& hostname,
                                const std::string& protocol) const
{
  return serverStatMan_->find(hostname, protocol);
}

SharedHandle<ServerStat>
RequestGroupMan::getOrCreateServerStat(const std::string& hostname,
                                       const std::string& protocol)
{
  SharedHandle<ServerStat> ss = findServerStat(hostname, protocol);
  if(!ss) {
    ss.reset(new ServerStat(hostname, protocol));
    addServerStat(ss);
  }
  return ss;
}

bool RequestGroupMan::addServerStat(const SharedHandle<ServerStat>& serverStat)
{
  return serverStatMan_->add(serverStat);
}

bool RequestGroupMan::loadServerStat(const std::string& filename)
{
  return serverStatMan_->load(filename);
}

bool RequestGroupMan::saveServerStat(const std::string& filename) const
{
  return serverStatMan_->save(filename);
}

void RequestGroupMan::removeStaleServerStat(time_t timeout)
{
  serverStatMan_->removeStaleServerStat(timeout);
}

bool RequestGroupMan::doesOverallDownloadSpeedExceed()
{
  return maxOverallDownloadSpeedLimit_ > 0 &&
    maxOverallDownloadSpeedLimit_ < netStat_.calculateDownloadSpeed();
}

bool RequestGroupMan::doesOverallUploadSpeedExceed()
{
  return maxOverallUploadSpeedLimit_ > 0 &&
    maxOverallUploadSpeedLimit_ < netStat_.calculateUploadSpeed();
}

void RequestGroupMan::getUsedHosts
(std::vector<std::pair<size_t, std::string> >& usedHosts)
{
  // vector of triplet which consists of use count, -download speed,
  // hostname. We want to sort by least used and faster download
  // speed. We use -download speed so that we can sort them using
  // operator<().
  std::vector<Triplet<size_t, int, std::string> > tempHosts;
  for(RequestGroupList::const_iterator i = requestGroups_.begin(),
        eoi = requestGroups_.end(); i != eoi; ++i) {
    const SharedHandle<RequestGroup>& rg = *i;
    const FileEntry::InFlightRequestSet& inFlightReqs =
      rg->getDownloadContext()->getFirstFileEntry()->getInFlightRequests();
    for(FileEntry::InFlightRequestSet::iterator j =
          inFlightReqs.begin(), eoj = inFlightReqs.end(); j != eoj; ++j) {
      uri_split_result us;
      if(uri_split(&us, (*j)->getUri().c_str()) == 0) {
        std::vector<Triplet<size_t, int, std::string> >::iterator k;
        std::vector<Triplet<size_t, int, std::string> >::iterator eok =
          tempHosts.end();
        std::string host = uri::getFieldString(us, USR_HOST,
                                               (*j)->getUri().c_str());
        for(k =  tempHosts.begin(); k != eok; ++k) {
          if((*k).third == host) {
            ++(*k).first;
            break;
          }
        }
        if(k == eok) {
          std::string protocol = uri::getFieldString(us, USR_SCHEME,
                                                     (*j)->getUri().c_str());
          SharedHandle<ServerStat> ss = findServerStat(host, protocol);
          int invDlSpeed = (ss && ss->isOK()) ?
            -(static_cast<int>(ss->getDownloadSpeed())) : 0;
          tempHosts.push_back(makeTriplet(1, invDlSpeed, host));
        }
      }
    }
  }
  std::sort(tempHosts.begin(), tempHosts.end());
  std::transform(tempHosts.begin(), tempHosts.end(),
                 std::back_inserter(usedHosts), Tuple2Pair<1, 3>());
}

void RequestGroupMan::setUriListParser
(const SharedHandle<UriListParser>& uriListParser)
{
  uriListParser_ = uriListParser;
}

void RequestGroupMan::initWrDiskCache()
{
  assert(wrDiskCache_ == 0);
  size_t limit = option_->getAsInt(PREF_DISK_CACHE);
  if(limit > 0) {
    wrDiskCache_ = new WrDiskCache(limit);
  }
}

} // namespace aria2
