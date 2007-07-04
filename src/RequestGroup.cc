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
#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "prefs.h"
#include "DefaultDiskWriter.h"
#include "RequestFactory.h"
#include "InitiateConnectionCommandFactory.h"
#include "CUIDCounter.h"
#include "DlAbortEx.h"
#include "File.h"
#include "message.h"
#include "DlAbortEx.h"
#include "Util.h"
#include "FatalException.h"
#include "DownloadCommand.h"
#ifdef ENABLE_MESSAGE_DIGEST
#include "CheckIntegrityCommand.h"
#include "CheckIntegrityEntry.h"
#endif // ENABLE_MESSAGE_DIGEST
#include <cerrno>

SegmentManHandle RequestGroup::initSegmentMan()
{
  _segmentMan = _segmentManFactory->createNewInstance();
  _segmentMan->ufilename = _ufilename;
  if(!_topDir.empty() && _topDir != ".") {
    _segmentMan->dir += "/"+_topDir;
  }
  return _segmentMan;
}

Commands RequestGroup::createNextCommandWithAdj(DownloadEngine* e, int32_t numAdj)
{
  int32_t numCommand = _numConcurrentCommand == 0 ? _uris.size() : _numConcurrentCommand+numAdj;
  return createNextCommand(e, numCommand, "GET");
}

Commands RequestGroup::createNextCommand(DownloadEngine* e, int32_t numCommand, const string& method)
{
  Commands commands;
  for(;!_uris.empty() && commands.size() < (size_t)numCommand; _uris.pop_front()) {
    string uri = _uris.front();
    _spentUris.push_back(uri);
    RequestHandle req = RequestFactorySingletonHolder::instance()->createRequest();
    req->setReferer(_option->get(PREF_REFERER));
    req->setMethod(method);
    if(req->setUrl(uri)) {
      commands.push_back(InitiateConnectionCommandFactory::createInitiateConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(), req, this, e));
    } else {
      logger->info(_("Unrecognized URL or unsupported protocol: %s\n"),
		   req->getUrl().c_str());
    }
  }
  return commands;
}

void RequestGroup::initBitfield()
{
  _segmentMan->initBitfield(_option->getAsInt(PREF_SEGMENT_SIZE),
			    _segmentMan->totalSize);
}

void RequestGroup::openExistingFile()
{
  _segmentMan->diskWriter->openExistingFile(_segmentMan->getFilePath());
}

void RequestGroup::markAllPiecesDone()
{
  _segmentMan->markAllPiecesDone();
}

void RequestGroup::markExistingPiecesDone()
{
  _segmentMan->markPieceDone(File(_segmentMan->getFilePath()).size());
}

void RequestGroup::markPieceDone(int64_t length)
{
  _segmentMan->markPieceDone(length);
}

void RequestGroup::shouldCancelDownloadForSafety()
{
  if(_segmentMan->shouldCancelDownloadForSafety()) {
    logger->notice(MSG_FILE_ALREADY_EXISTS,
		   _segmentMan->getFilePath().c_str(),
		   _segmentMan->getSegmentFilePath().c_str());
    
    throw new FatalException(EX_DOWNLOAD_ABORTED);
  }
}

void RequestGroup::initAndOpenFile()
{
  File d(getDir());
  if(d.isDir()) {
  } else if(d.exists()) {
    throw new FatalException(EX_MAKE_DIR, getDir().c_str(), "not a directory");
  } else if(!d.mkdirs()) {
    throw new FatalException(EX_MAKE_DIR, getDir().c_str(), strerror(errno));
  }
  _segmentMan->diskWriter->initAndOpenFile(_segmentMan->getFilePath());
}
  
bool RequestGroup::needsFileAllocation() const
{
  return _option->get(PREF_FILE_ALLOCATION) == V_PREALLOC
    && File(_segmentMan->getFilePath()).size() < _segmentMan->totalSize;
}
  
bool RequestGroup::fileExists() const
{
  return _segmentMan->fileExists();
}

bool RequestGroup::segmentFileExists() const
{
  return _segmentMan->segmentFileExists();
}

string RequestGroup::getFilePath() const
{
  if(_segmentMan.isNull()) {
    return "";
  } else {
    return _segmentMan->getFilePath();
  }
}

string RequestGroup::getDir() const
{
  if(_segmentMan.isNull()) {
    return "";
  } else {
    return _segmentMan->dir;
  }
}

void RequestGroup::loadAndOpenFile()
{
  bool segFileExists = segmentFileExists();
  if(segFileExists) {
    load();
    openExistingFile();
  } else if(fileExists() && _option->get(PREF_CONTINUE) == V_TRUE) {
    File existingFile(getFilePath());
    if(getTotalLength() < existingFile.size()) {
      throw new DlAbortEx("Invalid file length. Cannot continue download %s: local %s, remote %s",
		    getFilePath().c_str(),
		    Util::llitos(existingFile.size()).c_str(),
		    Util::llitos(getTotalLength()).c_str());
    }
    initBitfield();
    openExistingFile();
    _segmentMan->markPieceDone(existingFile.size());
  } else {
    shouldCancelDownloadForSafety();
    initBitfield();
#ifdef ENABLE_MESSAGE_DIGEST
    if(fileExists() && _option->get(PREF_CHECK_INTEGRITY) == V_TRUE) {
      openExistingFile();
    } else {
      initAndOpenFile();
    }
#else // ENABLE_MESSAGE_DIGEST
    initAndOpenFile();
#endif // ENABLE_MESSAGE_DIGEST
  }
}

bool RequestGroup::downloadFinishedByFileLength()
{
  if(_segmentMan->segmentFileExists()) {
    return false;
  }
  File existingFile(getFilePath());
  if(existingFile.exists() &&
     getTotalLength() == existingFile.size()) {
    _segmentMan->downloadStarted = true;
    initBitfield();
    _segmentMan->markAllPiecesDone();
    return true;
  } else {
    return false;
  }
}

void RequestGroup::prepareForNextAction(int cuid, const RequestHandle& req, DownloadEngine* e, DownloadCommand* downloadCommand)
{
  File existingFile(getFilePath());
#ifdef ENABLE_MESSAGE_DIGEST
  if(existingFile.size() > 0 && _option->get(PREF_CHECK_INTEGRITY) == V_TRUE) {
    // purge SegmentEntries
    _segmentMan->purgeSegmentEntry();

    CheckIntegrityEntryHandle entry = new CheckIntegrityEntry(cuid, req, this);
    entry->setNextDownloadCommand(downloadCommand);
    entry->initValidator();
    CheckIntegrityCommand* command = new CheckIntegrityCommand(cuid, this, e, entry);
    e->commands.push_back(command);
  } else
#endif // ENABLE_MESSAGE_DIGEST
    if(needsFileAllocation()) {
      FileAllocationEntryHandle entry = new FileAllocationEntry(cuid, req, this, existingFile.size());
      entry->setNextDownloadCommand(downloadCommand);
      e->_fileAllocationMan->pushFileAllocationEntry(entry);
    } else {
      if(downloadCommand) {
	e->commands.push_back(downloadCommand);
      } else {
	Commands commands = createNextCommandWithAdj(e, -1);
	Command* command = InitiateConnectionCommandFactory::createInitiateConnectionCommand(cuid, req, this, e);
	commands.push_front(command);
	e->addCommand(commands);
      }
    }
}

void RequestGroup::validateFilename(const string& expectedFilename,
				    const string& actualFilename) const
{
  if(expectedFilename.empty()) {
    return;
  }
  if(expectedFilename != actualFilename) {
    throw new DlAbortEx(EX_FILENAME_MISMATCH,
			expectedFilename.c_str(),
			actualFilename.c_str());
  }
}

void RequestGroup::validateTotalLength(int64_t expectedTotalLength,
				       int64_t actualTotalLength) const
{
  if(expectedTotalLength <= 0) {
    return;
  }
  if(expectedTotalLength != actualTotalLength) {
    throw new DlAbortEx(EX_SIZE_MISMATCH,
			expectedTotalLength,
			actualTotalLength);
  }
}

void RequestGroup::validateFilename(const string& actualFilename) const
{
  validateFilename(_segmentMan->filename, actualFilename);
}

void RequestGroup::validateTotalLength(int64_t actualTotalLength) const
{
  validateTotalLength(_segmentMan->totalSize, actualTotalLength);
}

void RequestGroup::validateFilenameByHint(const string& actualFilename) const
{
  validateFilename(_hintFilename, actualFilename);
}

void RequestGroup::validateTotalLengthByHint(int64_t actualTotalLength) const
{
  validateTotalLength(_hintTotalLength, actualTotalLength);
}

void RequestGroup::setUserDefinedFilename(const string& filename)
{
  _ufilename = filename;
}

int64_t RequestGroup::getExistingFileLength() const
{
  return File(getFilePath()).size();
}
