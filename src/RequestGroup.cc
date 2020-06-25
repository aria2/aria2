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
#include "RequestGroup.h"

#include <cassert>
#include <algorithm>

#include "PostDownloadHandler.h"
#include "DownloadEngine.h"
#include "SegmentMan.h"
#include "NullProgressInfoFile.h"
#include "Dependency.h"
#include "prefs.h"
#include "CreateRequestCommand.h"
#include "File.h"
#include "message.h"
#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DiskAdaptor.h"
#include "DiskWriterFactory.h"
#include "RecoverableException.h"
#include "StreamCheckIntegrityEntry.h"
#include "CheckIntegrityCommand.h"
#include "UnknownLengthPieceStorage.h"
#include "DownloadContext.h"
#include "DlAbortEx.h"
#include "DownloadFailureException.h"
#include "RequestGroupMan.h"
#include "DefaultBtProgressInfoFile.h"
#include "DefaultPieceStorage.h"
#include "download_handlers.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "DownloadHandlerConstants.h"
#include "Option.h"
#include "FileEntry.h"
#include "Request.h"
#include "FileAllocationIterator.h"
#include "fmt.h"
#include "A2STR.h"
#include "URISelector.h"
#include "InorderURISelector.h"
#include "PieceSelector.h"
#include "a2functional.h"
#include "SocketCore.h"
#include "SimpleRandomizer.h"
#include "Segment.h"
#include "SocketRecvBuffer.h"
#include "RequestGroupCriteria.h"
#include "CheckIntegrityCommand.h"
#include "ChecksumCheckIntegrityEntry.h"
#ifdef ENABLE_BITTORRENT
#  include "bittorrent_helper.h"
#  include "BtRegistry.h"
#  include "BtCheckIntegrityEntry.h"
#  include "DefaultPeerStorage.h"
#  include "DefaultBtAnnounce.h"
#  include "BtRuntime.h"
#  include "BtSetup.h"
#  include "BtPostDownloadHandler.h"
#  include "DHTSetup.h"
#  include "DHTRegistry.h"
#  include "DHTNode.h"
#  include "DHTRoutingTable.h"
#  include "DHTTaskQueue.h"
#  include "DHTTaskFactory.h"
#  include "DHTTokenTracker.h"
#  include "DHTMessageDispatcher.h"
#  include "DHTMessageReceiver.h"
#  include "DHTMessageFactory.h"
#  include "DHTMessageCallback.h"
#  include "BtMessageFactory.h"
#  include "BtRequestFactory.h"
#  include "BtMessageDispatcher.h"
#  include "BtMessageReceiver.h"
#  include "PeerConnection.h"
#  include "ExtensionMessageFactory.h"
#  include "DHTPeerAnnounceStorage.h"
#  include "DHTEntryPointNameResolveCommand.h"
#  include "LongestSequencePieceSelector.h"
#  include "PriorityPieceSelector.h"
#  include "bittorrent_helper.h"
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
#  include "MetalinkPostDownloadHandler.h"
#endif // ENABLE_METALINK

namespace aria2 {

RequestGroup::RequestGroup(const std::shared_ptr<GroupId>& gid,
                           const std::shared_ptr<Option>& option)
    : belongsToGID_(0),
      gid_(gid),
      option_(option),
      progressInfoFile_(std::make_shared<NullProgressInfoFile>()),
      uriSelector_(make_unique<InorderURISelector>()),
      requestGroupMan_(nullptr),
#ifdef ENABLE_BITTORRENT
      btRuntime_(nullptr),
      peerStorage_(nullptr),
#endif // ENABLE_BITTORRENT
      followingGID_(0),
      lastModifiedTime_(Time::null()),
      timeout_(option->getAsInt(PREF_TIMEOUT)),
      state_(STATE_WAITING),
      numConcurrentCommand_(option->getAsInt(PREF_SPLIT)),
      numStreamConnection_(0),
      numStreamCommand_(0),
      numCommand_(0),
      fileNotFoundCount_(0),
      maxDownloadSpeedLimit_(option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT)),
      maxUploadSpeedLimit_(option->getAsInt(PREF_MAX_UPLOAD_LIMIT)),
      resumeFailureCount_(0),
      haltReason_(RequestGroup::NONE),
      lastErrorCode_(error_code::UNDEFINED),
      saveControlFile_(true),
      preLocalFileCheckEnabled_(true),
      haltRequested_(false),
      forceHaltRequested_(false),
      pauseRequested_(false),
      restartRequested_(false),
      inMemoryDownload_(false),
      seedOnly_(false)
{
  fileAllocationEnabled_ = option_->get(PREF_FILE_ALLOCATION) != V_NONE;
  if (!option_->getAsBool(PREF_DRY_RUN)) {
    initializePreDownloadHandler();
    initializePostDownloadHandler();
  }
}

RequestGroup::~RequestGroup() = default;

bool RequestGroup::isCheckIntegrityReady()
{
  return option_->getAsBool(PREF_CHECK_INTEGRITY) &&
         ((downloadContext_->isChecksumVerificationAvailable() &&
           downloadFinishedByFileLength()) ||
          downloadContext_->isPieceHashVerificationAvailable());
}

bool RequestGroup::downloadFinished() const
{
  if (!pieceStorage_) {
    return false;
  }
  return pieceStorage_->downloadFinished();
}

bool RequestGroup::allDownloadFinished() const
{
  if (!pieceStorage_) {
    return false;
  }
  return pieceStorage_->allDownloadFinished();
}

std::pair<error_code::Value, std::string> RequestGroup::downloadResult() const
{
  if (downloadFinished() && !downloadContext_->isChecksumVerificationNeeded()) {
    return std::make_pair(error_code::FINISHED, "");
  }

  if (haltReason_ == RequestGroup::USER_REQUEST) {
    return std::make_pair(error_code::REMOVED, "");
  }

  if (lastErrorCode_ == error_code::UNDEFINED) {
    if (haltReason_ == RequestGroup::SHUTDOWN_SIGNAL) {
      return std::make_pair(error_code::IN_PROGRESS, "");
    }
    return std::make_pair(error_code::UNKNOWN_ERROR, "");
  }

  return std::make_pair(lastErrorCode_, lastErrorMessage_);
}

void RequestGroup::closeFile()
{
  if (pieceStorage_) {
    pieceStorage_->flushWrDiskCacheEntry(true);
    pieceStorage_->getDiskAdaptor()->flushOSBuffers();
    pieceStorage_->getDiskAdaptor()->closeFile();
  }
}

// TODO The function name is not intuitive at all.. it does not convey
// that this function open file.
std::unique_ptr<CheckIntegrityEntry> RequestGroup::createCheckIntegrityEntry()
{
  auto infoFile = std::make_shared<DefaultBtProgressInfoFile>(
      downloadContext_, pieceStorage_, option_.get());

  if (option_->getAsBool(PREF_CHECK_INTEGRITY) &&
      downloadContext_->isPieceHashVerificationAvailable()) {
    // When checking piece hash, we don't care file is downloaded and
    // infoFile exists.
    loadAndOpenFile(infoFile);
    return make_unique<StreamCheckIntegrityEntry>(this);
  }

  if (isPreLocalFileCheckEnabled() &&
      (infoFile->exists() || (File(getFirstFilePath()).exists() &&
                              option_->getAsBool(PREF_CONTINUE)))) {
    // If infoFile exists or -c option is given, we need to check
    // download has been completed (which is determined after
    // loadAndOpenFile()). If so, use ChecksumCheckIntegrityEntry when
    // verification is enabled, because CreateRequestCommand does not
    // issue checksum verification and download fails without it.
    loadAndOpenFile(infoFile);
    if (downloadFinished()) {
      if (downloadContext_->isChecksumVerificationNeeded()) {
        A2_LOG_INFO(MSG_HASH_CHECK_NOT_DONE);
        auto tempEntry = make_unique<ChecksumCheckIntegrityEntry>(this);
        tempEntry->setRedownload(true);
        return std::move(tempEntry);
      }
      downloadContext_->setChecksumVerified(true);
      A2_LOG_NOTICE(fmt(MSG_DOWNLOAD_ALREADY_COMPLETED, gid_->toHex().c_str(),
                        downloadContext_->getBasePath().c_str()));
      return nullptr;
    }
    return make_unique<StreamCheckIntegrityEntry>(this);
  }

  if (downloadFinishedByFileLength() &&
      downloadContext_->isChecksumVerificationAvailable()) {
    pieceStorage_->markAllPiecesDone();
    loadAndOpenFile(infoFile);
    auto tempEntry = make_unique<ChecksumCheckIntegrityEntry>(this);
    tempEntry->setRedownload(true);
    return std::move(tempEntry);
  }

  loadAndOpenFile(infoFile);
  return make_unique<StreamCheckIntegrityEntry>(this);
}

void RequestGroup::createInitialCommand(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e)
{
  // Start session timer here.  When file size becomes known, it will
  // be reset again in *FileAllocationEntry, because hash check and
  // file allocation takes a time.  For downloads in which file size
  // is unknown, session timer will not be reset.
  downloadContext_->resetDownloadStartTime();
#ifdef ENABLE_BITTORRENT
  if (downloadContext_->hasAttribute(CTX_ATTR_BT)) {
    auto torrentAttrs = bittorrent::getTorrentAttrs(downloadContext_);
    bool metadataGetMode = torrentAttrs->metadata.empty();
    if (option_->getAsBool(PREF_DRY_RUN)) {
      throw DOWNLOAD_FAILURE_EXCEPTION(
          "Cancel BitTorrent download in dry-run context.");
    }
    auto& btRegistry = e->getBtRegistry();
    if (btRegistry->getDownloadContext(torrentAttrs->infoHash)) {
      // TODO If metadataGetMode == false and each FileEntry has
      // URI, then go without BT.
      throw DOWNLOAD_FAILURE_EXCEPTION2(
          fmt("InfoHash %s is already registered.",
              bittorrent::getInfoHashString(downloadContext_).c_str()),
          error_code::DUPLICATE_INFO_HASH);
    }
    if (metadataGetMode) {
      // Use UnknownLengthPieceStorage.
      initPieceStorage();
    }
    else if (e->getRequestGroupMan()->isSameFileBeingDownloaded(this)) {
      throw DOWNLOAD_FAILURE_EXCEPTION2(
          fmt(EX_DUPLICATE_FILE_DOWNLOAD,
              downloadContext_->getBasePath().c_str()),
          error_code::DUPLICATE_DOWNLOAD);
    }
    else {
      initPieceStorage();
      if (downloadContext_->getFileEntries().size() > 1) {
        pieceStorage_->setupFileFilter();
      }
    }

    std::shared_ptr<DefaultBtProgressInfoFile> progressInfoFile;
    if (!metadataGetMode) {
      progressInfoFile = std::make_shared<DefaultBtProgressInfoFile>(
          downloadContext_, pieceStorage_, option_.get());
    }

    auto btRuntime = std::make_shared<BtRuntime>();
    btRuntime->setMaxPeers(option_->getAsInt(PREF_BT_MAX_PEERS));
    btRuntime_ = btRuntime.get();
    if (progressInfoFile) {
      progressInfoFile->setBtRuntime(btRuntime);
    }

    auto peerStorage = std::make_shared<DefaultPeerStorage>();
    peerStorage->setBtRuntime(btRuntime);
    peerStorage->setPieceStorage(pieceStorage_);
    peerStorage_ = peerStorage.get();
    if (progressInfoFile) {
      progressInfoFile->setPeerStorage(peerStorage);
    }

    auto btAnnounce = std::make_shared<DefaultBtAnnounce>(
        downloadContext_.get(), option_.get());
    btAnnounce->setBtRuntime(btRuntime);
    btAnnounce->setPieceStorage(pieceStorage_);
    btAnnounce->setPeerStorage(peerStorage);
    btAnnounce->setUserDefinedInterval(
        std::chrono::seconds(option_->getAsInt(PREF_BT_TRACKER_INTERVAL)));
    btAnnounce->shuffleAnnounce();

    assert(!btRegistry->get(gid_->getNumericId()));
    btRegistry->put(
        gid_->getNumericId(),
        make_unique<BtObject>(
            downloadContext_, pieceStorage_, peerStorage, btAnnounce, btRuntime,
            (progressInfoFile ? progressInfoFile : progressInfoFile_)));

    if (option_->getAsBool(PREF_ENABLE_DHT) ||
        (!e->getOption()->getAsBool(PREF_DISABLE_IPV6) &&
         option_->getAsBool(PREF_ENABLE_DHT6))) {

      if (option_->getAsBool(PREF_ENABLE_DHT)) {
        std::vector<std::unique_ptr<Command>> c, rc;
        std::tie(c, rc) = DHTSetup().setup(e, AF_INET);

        e->addCommand(std::move(c));
        for (auto& a : rc) {
          e->addRoutineCommand(std::move(a));
        }
      }

      if (!e->getOption()->getAsBool(PREF_DISABLE_IPV6) &&
          option_->getAsBool(PREF_ENABLE_DHT6)) {
        std::vector<std::unique_ptr<Command>> c, rc;
        std::tie(c, rc) = DHTSetup().setup(e, AF_INET6);

        e->addCommand(std::move(c));
        for (auto& a : rc) {
          e->addRoutineCommand(std::move(a));
        }
      }
      const auto& nodes = torrentAttrs->nodes;
      if (!torrentAttrs->privateTorrent && !nodes.empty()) {
        if (DHTRegistry::isInitialized()) {
          auto command = make_unique<DHTEntryPointNameResolveCommand>(
              e->newCUID(), e, AF_INET, nodes);
          const auto& data = DHTRegistry::getData();
          command->setTaskQueue(data.taskQueue.get());
          command->setTaskFactory(data.taskFactory.get());
          command->setRoutingTable(data.routingTable.get());
          command->setLocalNode(data.localNode);
          e->addCommand(std::move(command));
        }

        if (DHTRegistry::isInitialized6()) {
          auto command = make_unique<DHTEntryPointNameResolveCommand>(
              e->newCUID(), e, AF_INET6, nodes);
          const auto& data = DHTRegistry::getData6();
          command->setTaskQueue(data.taskQueue.get());
          command->setTaskFactory(data.taskFactory.get());
          command->setRoutingTable(data.routingTable.get());
          command->setLocalNode(data.localNode);
          e->addCommand(std::move(command));
        }
      }
    }
    else if (metadataGetMode) {
      A2_LOG_NOTICE(_("For BitTorrent Magnet URI, enabling DHT is strongly"
                      " recommended. See --enable-dht option."));
    }

    if (metadataGetMode) {
      BtCheckIntegrityEntry{this}.onDownloadIncomplete(commands, e);
      return;
    }

    removeDefunctControlFile(progressInfoFile);
    {
      int64_t actualFileSize = pieceStorage_->getDiskAdaptor()->size();
      if (actualFileSize == downloadContext_->getTotalLength()) {
        // First, make DiskAdaptor read-only mode to allow the
        // program to seed file in read-only media.
        pieceStorage_->getDiskAdaptor()->enableReadOnly();
      }
      else {
        // Open file in writable mode to allow the program
        // truncate the file to downloadContext_->getTotalLength()
        A2_LOG_DEBUG(fmt("File size not match. File is opened in writable"
                         " mode. Expected:%" PRId64 " Actual:%" PRId64 "",
                         downloadContext_->getTotalLength(), actualFileSize));
      }
    }
    // Call Load, Save and file allocation command here
    if (progressInfoFile->exists()) {
      // load .aria2 file if it exists.
      progressInfoFile->load();
      pieceStorage_->getDiskAdaptor()->openFile();
    }
    else if (pieceStorage_->getDiskAdaptor()->fileExists()) {
      if (!option_->getAsBool(PREF_CHECK_INTEGRITY) &&
          !option_->getAsBool(PREF_ALLOW_OVERWRITE) &&
          !option_->getAsBool(PREF_BT_SEED_UNVERIFIED)) {
        // TODO we need this->haltRequested = true?
        throw DOWNLOAD_FAILURE_EXCEPTION2(
            fmt(MSG_FILE_ALREADY_EXISTS,
                downloadContext_->getBasePath().c_str()),
            error_code::FILE_ALREADY_EXISTS);
      }
      pieceStorage_->getDiskAdaptor()->openFile();
      if (option_->getAsBool(PREF_BT_SEED_UNVERIFIED)) {
        pieceStorage_->markAllPiecesDone();
      }
    }
    else {
      pieceStorage_->getDiskAdaptor()->openFile();
    }
    progressInfoFile_ = progressInfoFile;

    auto entry = make_unique<BtCheckIntegrityEntry>(this);
    // --bt-seed-unverified=true is given and download has completed, skip
    // validation for piece hashes.
    if (option_->getAsBool(PREF_BT_SEED_UNVERIFIED) &&
        pieceStorage_->downloadFinished()) {
      entry->onDownloadFinished(commands, e);
    }
    else {
      processCheckIntegrityEntry(commands, std::move(entry), e);
    }
    return;
  }
#endif // ENABLE_BITTORRENT

  if (downloadContext_->getFileEntries().size() == 1) {
    // TODO I assume here when totallength is set to DownloadContext and it is
    // not 0, then filepath is also set DownloadContext correctly....
    if (option_->getAsBool(PREF_DRY_RUN) ||
        downloadContext_->getTotalLength() == 0) {
      createNextCommand(commands, e, 1);
      return;
    }
    auto progressInfoFile = std::make_shared<DefaultBtProgressInfoFile>(
        downloadContext_, nullptr, option_.get());
    adjustFilename(progressInfoFile);
    initPieceStorage();
    auto checkEntry = createCheckIntegrityEntry();
    if (checkEntry) {
      processCheckIntegrityEntry(commands, std::move(checkEntry), e);
    }
    return;
  }

  // TODO --dry-run is not supported for multifile download for now.
  if (option_->getAsBool(PREF_DRY_RUN)) {
    throw DOWNLOAD_FAILURE_EXCEPTION(
        "--dry-run in multi-file download is not supported yet.");
  }
  // TODO file size is known in this context?

  // In this context, multiple FileEntry objects are in
  // DownloadContext.
  if (e->getRequestGroupMan()->isSameFileBeingDownloaded(this)) {
    throw DOWNLOAD_FAILURE_EXCEPTION2(
        fmt(EX_DUPLICATE_FILE_DOWNLOAD,
            downloadContext_->getBasePath().c_str()),
        error_code::DUPLICATE_DOWNLOAD);
  }
  initPieceStorage();
  if (downloadContext_->getFileEntries().size() > 1) {
    pieceStorage_->setupFileFilter();
  }
  auto progressInfoFile = std::make_shared<DefaultBtProgressInfoFile>(
      downloadContext_, pieceStorage_, option_.get());
  removeDefunctControlFile(progressInfoFile);
  // Call Load, Save and file allocation command here
  if (progressInfoFile->exists()) {
    // load .aria2 file if it exists.
    progressInfoFile->load();
    pieceStorage_->getDiskAdaptor()->openFile();
  }
  else if (pieceStorage_->getDiskAdaptor()->fileExists()) {
    if (!isCheckIntegrityReady() && !option_->getAsBool(PREF_ALLOW_OVERWRITE)) {
      // TODO we need this->haltRequested = true?
      throw DOWNLOAD_FAILURE_EXCEPTION2(
          fmt(MSG_FILE_ALREADY_EXISTS, downloadContext_->getBasePath().c_str()),
          error_code::FILE_ALREADY_EXISTS);
    }
    pieceStorage_->getDiskAdaptor()->openFile();
  }
  else {
    pieceStorage_->getDiskAdaptor()->openFile();
  }
  progressInfoFile_ = progressInfoFile;
  processCheckIntegrityEntry(commands,
                             make_unique<StreamCheckIntegrityEntry>(this), e);
}

void RequestGroup::processCheckIntegrityEntry(
    std::vector<std::unique_ptr<Command>>& commands,
    std::unique_ptr<CheckIntegrityEntry> entry, DownloadEngine* e)
{
  int64_t actualFileSize = pieceStorage_->getDiskAdaptor()->size();
  if (actualFileSize > downloadContext_->getTotalLength()) {
    entry->cutTrailingGarbage();
  }
  if ((option_->getAsBool(PREF_CHECK_INTEGRITY) ||
       downloadContext_->isChecksumVerificationNeeded()) &&
      entry->isValidationReady()) {
    entry->initValidator();
    // Don't save control file(.aria2 file) when user presses
    // control-c key while aria2 is checking hashes. If control file
    // doesn't exist when aria2 launched, the completed length in
    // saved control file will be 0 byte and this confuses user.
    // enableSaveControlFile() will be called after hash checking is
    // done. See CheckIntegrityCommand.
    disableSaveControlFile();
    e->getCheckIntegrityMan()->pushEntry(std::move(entry));
    return;
  }

  entry->onDownloadIncomplete(commands, e);
}

void RequestGroup::initPieceStorage()
{
  std::shared_ptr<PieceStorage> tempPieceStorage;
  if (downloadContext_->knowsTotalLength() &&
      // Following conditions are needed for chunked encoding with
      // content-length = 0. Google's dl server used this before.
      (downloadContext_->getTotalLength() > 0
#ifdef ENABLE_BITTORRENT
       || downloadContext_->hasAttribute(CTX_ATTR_BT)
#endif // ENABLE_BITTORRENT
           )) {
#ifdef ENABLE_BITTORRENT
    auto ps =
        std::make_shared<DefaultPieceStorage>(downloadContext_, option_.get());
    if (downloadContext_->hasAttribute(CTX_ATTR_BT)) {
      if (isUriSuppliedForRequsetFileEntry(
              downloadContext_->getFileEntries().begin(),
              downloadContext_->getFileEntries().end())) {
        // Use LongestSequencePieceSelector when HTTP/FTP/BitTorrent
        // integrated downloads.
        A2_LOG_DEBUG("Using LongestSequencePieceSelector");
        ps->setPieceSelector(make_unique<LongestSequencePieceSelector>());
      }
      if (option_->defined(PREF_BT_PRIORITIZE_PIECE)) {
        std::vector<size_t> result;
        util::parsePrioritizePieceRange(result,
                                        option_->get(PREF_BT_PRIORITIZE_PIECE),
                                        downloadContext_->getFileEntries(),
                                        downloadContext_->getPieceLength());
        if (!result.empty()) {
          std::shuffle(std::begin(result), std::end(result),
                       *SimpleRandomizer::getInstance());
          auto priSelector =
              make_unique<PriorityPieceSelector>(ps->popPieceSelector());
          priSelector->setPriorityPiece(std::begin(result), std::end(result));
          ps->setPieceSelector(std::move(priSelector));
        }
      }
    }
#else  // !ENABLE_BITTORRENT
    auto ps =
        std::make_shared<DefaultPieceStorage>(downloadContext_, option_.get());
#endif // !ENABLE_BITTORRENT
    if (requestGroupMan_) {
      ps->setWrDiskCache(requestGroupMan_->getWrDiskCache());
    }
    if (diskWriterFactory_) {
      ps->setDiskWriterFactory(diskWriterFactory_);
    }
    tempPieceStorage = ps;
  }
  else {
    auto ps = std::make_shared<UnknownLengthPieceStorage>(downloadContext_);
    if (diskWriterFactory_) {
      ps->setDiskWriterFactory(diskWriterFactory_);
    }
    tempPieceStorage = ps;
  }
  tempPieceStorage->initStorage();
  if (requestGroupMan_) {
    tempPieceStorage->getDiskAdaptor()->setOpenedFileCounter(
        requestGroupMan_->getOpenedFileCounter());
  }
  segmentMan_ =
      std::make_shared<SegmentMan>(downloadContext_, tempPieceStorage);
  pieceStorage_ = tempPieceStorage;

#ifdef __MINGW32__
  // Windows build: --file-allocation=falloc uses SetFileValidData
  // which requires SE_MANAGE_VOLUME_NAME privilege.  SetFileValidData
  // has security implications (see
  // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365544%28v=vs.85%29.aspx).
  static auto gainPrivilegeAttempted = false;

  if (!gainPrivilegeAttempted &&
      pieceStorage_->getDiskAdaptor()->getFileAllocationMethod() ==
          DiskAdaptor::FILE_ALLOC_FALLOC &&
      isFileAllocationEnabled()) {
    if (!util::gainPrivilege(SE_MANAGE_VOLUME_NAME)) {
      A2_LOG_WARN("--file-allocation=falloc will not work properly.");
    }
    else {
      A2_LOG_DEBUG("SE_MANAGE_VOLUME_NAME privilege acquired");

      A2_LOG_WARN(
          "--file-allocation=falloc will use SetFileValidData() API, and "
          "aria2 uses uninitialized disk space which may contain "
          "confidential data as the download file space. If it is "
          "undesirable, --file-allocation=prealloc is slower, but safer "
          "option.");
    }

    gainPrivilegeAttempted = true;
  }
#endif // __MINGW32__
}

void RequestGroup::dropPieceStorage()
{
  segmentMan_.reset();
  pieceStorage_.reset();
}

bool RequestGroup::downloadFinishedByFileLength()
{
  // assuming that a control file doesn't exist.
  if (!isPreLocalFileCheckEnabled() ||
      option_->getAsBool(PREF_ALLOW_OVERWRITE)) {
    return false;
  }
  if (!downloadContext_->knowsTotalLength()) {
    return false;
  }
  File outfile(getFirstFilePath());
  if (outfile.exists() &&
      downloadContext_->getTotalLength() == outfile.size()) {
    return true;
  }
  return false;
}

void RequestGroup::adjustFilename(
    const std::shared_ptr<BtProgressInfoFile>& infoFile)
{
  if (!isPreLocalFileCheckEnabled()) {
    // OK, no need to care about filename.
    return;
  }
  // TODO need this?
  if (requestGroupMan_) {
    if (requestGroupMan_->isSameFileBeingDownloaded(this)) {
      // The file name must be renamed
      tryAutoFileRenaming();
      A2_LOG_NOTICE(fmt(MSG_FILE_RENAMED, getFirstFilePath().c_str()));
      return;
    }
  }
  if (!option_->getAsBool(PREF_DRY_RUN) &&
      option_->getAsBool(PREF_REMOVE_CONTROL_FILE) && infoFile->exists()) {
    infoFile->removeFile();
    A2_LOG_NOTICE(fmt(_("Removed control file for %s because it is requested by"
                        " user."),
                      infoFile->getFilename().c_str()));
  }

  if (infoFile->exists()) {
    // Use current filename
    return;
  }

  File outfile(getFirstFilePath());
  if (outfile.exists() && option_->getAsBool(PREF_CONTINUE) &&
      outfile.size() <= downloadContext_->getTotalLength()) {
    // File exists but user decided to resume it.
  }
  else if (outfile.exists() && isCheckIntegrityReady()) {
    // check-integrity existing file
  }
  else {
    shouldCancelDownloadForSafety();
  }
}

void RequestGroup::removeDefunctControlFile(
    const std::shared_ptr<BtProgressInfoFile>& progressInfoFile)
{
  // Remove the control file if download file doesn't exist
  if (progressInfoFile->exists() &&
      !pieceStorage_->getDiskAdaptor()->fileExists()) {
    progressInfoFile->removeFile();
    A2_LOG_NOTICE(fmt(MSG_REMOVED_DEFUNCT_CONTROL_FILE,
                      progressInfoFile->getFilename().c_str(),
                      downloadContext_->getBasePath().c_str()));
  }
}

void RequestGroup::loadAndOpenFile(
    const std::shared_ptr<BtProgressInfoFile>& progressInfoFile)
{
  try {
    if (!isPreLocalFileCheckEnabled()) {
      pieceStorage_->getDiskAdaptor()->initAndOpenFile();
      return;
    }
    removeDefunctControlFile(progressInfoFile);
    if (progressInfoFile->exists()) {
      progressInfoFile->load();
      pieceStorage_->getDiskAdaptor()->openExistingFile();
    }
    else {
      File outfile(getFirstFilePath());
      if (outfile.exists() && option_->getAsBool(PREF_CONTINUE) &&
          outfile.size() <= getTotalLength()) {
        pieceStorage_->getDiskAdaptor()->openExistingFile();
        pieceStorage_->markPiecesDone(outfile.size());
      }
      else if (outfile.exists() && isCheckIntegrityReady()) {
        pieceStorage_->getDiskAdaptor()->openExistingFile();
      }
      else {
        pieceStorage_->getDiskAdaptor()->initAndOpenFile();
      }
    }
    setProgressInfoFile(progressInfoFile);
  }
  catch (RecoverableException& e) {
    throw DOWNLOAD_FAILURE_EXCEPTION2(EX_DOWNLOAD_ABORTED, e);
  }
}

// assuming that a control file does not exist
void RequestGroup::shouldCancelDownloadForSafety()
{
  if (option_->getAsBool(PREF_ALLOW_OVERWRITE)) {
    return;
  }
  File outfile(getFirstFilePath());
  if (!outfile.exists()) {
    return;
  }

  tryAutoFileRenaming();
  A2_LOG_NOTICE(fmt(MSG_FILE_RENAMED, getFirstFilePath().c_str()));
}

void RequestGroup::tryAutoFileRenaming()
{
  if (!option_->getAsBool(PREF_AUTO_FILE_RENAMING)) {
    throw DOWNLOAD_FAILURE_EXCEPTION2(
        fmt(MSG_FILE_ALREADY_EXISTS, getFirstFilePath().c_str()),
        error_code::FILE_ALREADY_EXISTS);
  }

  std::string filepath = getFirstFilePath();
  if (filepath.empty()) {
    throw DOWNLOAD_FAILURE_EXCEPTION2(
        fmt("File renaming failed: %s", getFirstFilePath().c_str()),
        error_code::FILE_RENAMING_FAILED);
  }
  auto fn = filepath;
  std::string ext;
  const auto idx = fn.find_last_of(".");
  const auto slash = fn.find_last_of("\\/");
  // Do extract the extension, as in "file.ext" = "file" and ".ext",
  // but do not consider ".file" to be a file name without extension instead
  // of a blank file name and an extension of ".file"
  if (idx != std::string::npos &&
      // fn has no path component and starts with a dot, but has no extension
      // otherwise
      idx != 0 &&
      // has a file path component if we found a slash.
      // if slash == idx - 1 this means a form of "*/.*", so the file name
      // starts with a dot, has no extension otherwise, and therefore do not
      // extract an extension either
      (slash == std::string::npos || slash < idx - 1)) {
    ext = fn.substr(idx);
    fn = fn.substr(0, idx);
  }
  for (int i = 1; i < 10000; ++i) {
    auto newfilename = fmt("%s.%d%s", fn.c_str(), i, ext.c_str());
    File newfile(newfilename);
    File ctrlfile(newfile.getPath() + DefaultBtProgressInfoFile::getSuffix());
    if (!newfile.exists() || (newfile.exists() && ctrlfile.exists())) {
      downloadContext_->getFirstFileEntry()->setPath(newfile.getPath());
      return;
    }
  }
  throw DOWNLOAD_FAILURE_EXCEPTION2(
      fmt("File renaming failed: %s", getFirstFilePath().c_str()),
      error_code::FILE_RENAMING_FAILED);
}

void RequestGroup::createNextCommandWithAdj(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e,
    int numAdj)
{
  int numCommand;
  if (getTotalLength() == 0) {
    numCommand = 1 + numAdj;
  }
  else {
    numCommand = std::min(downloadContext_->getNumPieces(),
                          static_cast<size_t>(numConcurrentCommand_));
    numCommand += numAdj;
  }

  if (numCommand > 0) {
    createNextCommand(commands, e, numCommand);
  }
}

void RequestGroup::createNextCommand(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e)
{
  int numCommand;
  if (getTotalLength() == 0) {
    if (numStreamCommand_ > 0) {
      numCommand = 0;
    }
    else {
      numCommand = 1;
    }
  }
  else if (numStreamCommand_ >= numConcurrentCommand_) {
    numCommand = 0;
  }
  else {
    numCommand = std::min(
        downloadContext_->getNumPieces(),
        static_cast<size_t>(numConcurrentCommand_ - numStreamCommand_));
  }

  if (numCommand > 0) {
    createNextCommand(commands, e, numCommand);
  }
}

void RequestGroup::createNextCommand(
    std::vector<std::unique_ptr<Command>>& commands, DownloadEngine* e,
    int numCommand)
{
  for (; numCommand > 0; --numCommand) {
    commands.push_back(
        make_unique<CreateRequestCommand>(e->newCUID(), this, e));
  }
  if (!commands.empty()) {
    e->setNoWait(true);
  }
}

std::string RequestGroup::getFirstFilePath() const
{
  assert(downloadContext_);
  if (inMemoryDownload()) {
    return "[MEMORY]" +
           File(downloadContext_->getFirstFileEntry()->getPath()).getBasename();
  }
  return downloadContext_->getFirstFileEntry()->getPath();
}

int64_t RequestGroup::getTotalLength() const
{
  if (!pieceStorage_) {
    return 0;
  }

  if (pieceStorage_->isSelectiveDownloadingMode()) {
    return pieceStorage_->getFilteredTotalLength();
  }

  return pieceStorage_->getTotalLength();
}

int64_t RequestGroup::getCompletedLength() const
{
  if (!pieceStorage_) {
    return 0;
  }

  if (pieceStorage_->isSelectiveDownloadingMode()) {
    return pieceStorage_->getFilteredCompletedLength();
  }

  return pieceStorage_->getCompletedLength();
}

void RequestGroup::validateFilename(const std::string& expectedFilename,
                                    const std::string& actualFilename) const
{
  if (expectedFilename.empty()) {
    return;
  }

  if (expectedFilename != actualFilename) {
    throw DL_ABORT_EX(fmt(EX_FILENAME_MISMATCH, expectedFilename.c_str(),
                          actualFilename.c_str()));
  }
}

void RequestGroup::validateTotalLength(int64_t expectedTotalLength,
                                       int64_t actualTotalLength) const
{
  if (expectedTotalLength <= 0) {
    return;
  }

  if (expectedTotalLength != actualTotalLength) {
    throw DL_ABORT_EX(
        fmt(EX_SIZE_MISMATCH, expectedTotalLength, actualTotalLength));
  }
}

void RequestGroup::validateFilename(const std::string& actualFilename) const
{
  validateFilename(downloadContext_->getFileEntries().front()->getBasename(),
                   actualFilename);
}

void RequestGroup::validateTotalLength(int64_t actualTotalLength) const
{
  validateTotalLength(getTotalLength(), actualTotalLength);
}

void RequestGroup::increaseStreamCommand() { ++numStreamCommand_; }

void RequestGroup::decreaseStreamCommand() { --numStreamCommand_; }

void RequestGroup::increaseStreamConnection() { ++numStreamConnection_; }

void RequestGroup::decreaseStreamConnection() { --numStreamConnection_; }

int RequestGroup::getNumConnection() const
{
  int numConnection = numStreamConnection_;
#ifdef ENABLE_BITTORRENT
  if (btRuntime_) {
    numConnection += btRuntime_->getConnections();
  }
#endif // ENABLE_BITTORRENT
  return numConnection;
}

void RequestGroup::increaseNumCommand() { ++numCommand_; }

void RequestGroup::decreaseNumCommand()
{
  --numCommand_;
  if (!numCommand_ && requestGroupMan_) {
    A2_LOG_DEBUG(fmt("GID#%s - Request queue check", gid_->toHex().c_str()));
    requestGroupMan_->requestQueueCheck();
  }
}

TransferStat RequestGroup::calculateStat() const
{
  TransferStat stat = downloadContext_->getNetStat().toTransferStat();
#ifdef ENABLE_BITTORRENT
  if (btRuntime_) {
    stat.allTimeUploadLength =
        btRuntime_->getUploadLengthAtStartup() + stat.sessionUploadLength;
  }
#endif // ENABLE_BITTORRENT
  return stat;
}

void RequestGroup::setHaltRequested(bool f, HaltReason haltReason)
{
  haltRequested_ = f;
  if (haltRequested_) {
    pauseRequested_ = false;
    haltReason_ = haltReason;
  }
#ifdef ENABLE_BITTORRENT
  if (btRuntime_) {
    btRuntime_->setHalt(f);
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::setForceHaltRequested(bool f, HaltReason haltReason)
{
  setHaltRequested(f, haltReason);
  forceHaltRequested_ = f;
}

void RequestGroup::setPauseRequested(bool f) { pauseRequested_ = f; }

void RequestGroup::setRestartRequested(bool f) { restartRequested_ = f; }

void RequestGroup::releaseRuntimeResource(DownloadEngine* e)
{
#ifdef ENABLE_BITTORRENT
  e->getBtRegistry()->remove(gid_->getNumericId());
  btRuntime_ = nullptr;
  peerStorage_ = nullptr;
#endif // ENABLE_BITTORRENT
  if (pieceStorage_) {
    pieceStorage_->removeAdvertisedPiece(Timer::zero());
  }
  // Don't reset segmentMan_ and pieceStorage_ here to provide
  // progress information via RPC
  progressInfoFile_ = std::make_shared<NullProgressInfoFile>();
  downloadContext_->releaseRuntimeResource();
  // Reset seedOnly_, so that we can handle pause/unpause-ing seeding
  // torrent with --bt-detach-seed-only.
  seedOnly_ = false;
}

void RequestGroup::preDownloadProcessing()
{
  A2_LOG_DEBUG(fmt("Finding PreDownloadHandler for path %s.",
                   getFirstFilePath().c_str()));
  try {
    for (const auto& pdh : preDownloadHandlers_) {
      if (pdh->canHandle(this)) {
        pdh->execute(this);
        return;
      }
    }
  }
  catch (RecoverableException& ex) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
    return;
  }

  A2_LOG_DEBUG("No PreDownloadHandler found.");
  return;
}

void RequestGroup::postDownloadProcessing(
    std::vector<std::shared_ptr<RequestGroup>>& groups)
{
  A2_LOG_DEBUG(fmt("Finding PostDownloadHandler for path %s.",
                   getFirstFilePath().c_str()));
  try {
    for (const auto& pdh : postDownloadHandlers_) {
      if (pdh->canHandle(this)) {
        pdh->getNextRequestGroups(groups, this);
        return;
      }
    }
  }
  catch (RecoverableException& ex) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, ex);
  }

  A2_LOG_DEBUG("No PostDownloadHandler found.");
}

void RequestGroup::initializePreDownloadHandler()
{
#ifdef ENABLE_BITTORRENT
  if (option_->get(PREF_FOLLOW_TORRENT) == V_MEM) {
    preDownloadHandlers_.push_back(
        download_handlers::getBtPreDownloadHandler());
  }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  if (option_->get(PREF_FOLLOW_METALINK) == V_MEM) {
    preDownloadHandlers_.push_back(
        download_handlers::getMetalinkPreDownloadHandler());
  }
#endif // ENABLE_METALINK
}

void RequestGroup::initializePostDownloadHandler()
{
#ifdef ENABLE_BITTORRENT
  if (option_->getAsBool(PREF_FOLLOW_TORRENT) ||
      option_->get(PREF_FOLLOW_TORRENT) == V_MEM) {
    postDownloadHandlers_.push_back(
        download_handlers::getBtPostDownloadHandler());
  }
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  if (option_->getAsBool(PREF_FOLLOW_METALINK) ||
      option_->get(PREF_FOLLOW_METALINK) == V_MEM) {
    postDownloadHandlers_.push_back(
        download_handlers::getMetalinkPostDownloadHandler());
  }
#endif // ENABLE_METALINK
}

bool RequestGroup::isDependencyResolved()
{
  if (!dependency_) {
    return true;
  }
  return dependency_->resolve();
}

void RequestGroup::dependsOn(const std::shared_ptr<Dependency>& dep)
{
  dependency_ = dep;
}

void RequestGroup::setDiskWriterFactory(
    const std::shared_ptr<DiskWriterFactory>& diskWriterFactory)
{
  diskWriterFactory_ = diskWriterFactory;
}

void RequestGroup::addPostDownloadHandler(const PostDownloadHandler* handler)
{
  postDownloadHandlers_.push_back(handler);
}

void RequestGroup::addPreDownloadHandler(const PreDownloadHandler* handler)
{
  preDownloadHandlers_.push_back(handler);
}

void RequestGroup::clearPostDownloadHandler() { postDownloadHandlers_.clear(); }

void RequestGroup::clearPreDownloadHandler() { preDownloadHandlers_.clear(); }

void RequestGroup::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void RequestGroup::setProgressInfoFile(
    const std::shared_ptr<BtProgressInfoFile>& progressInfoFile)
{
  progressInfoFile_ = progressInfoFile;
}

bool RequestGroup::needsFileAllocation() const
{
  return isFileAllocationEnabled() &&
         option_->getAsLLInt(PREF_NO_FILE_ALLOCATION_LIMIT) <=
             getTotalLength() &&
         !pieceStorage_->getDiskAdaptor()->fileAllocationIterator()->finished();
}

std::shared_ptr<DownloadResult> RequestGroup::createDownloadResult() const
{
  A2_LOG_DEBUG(fmt("GID#%s - Creating DownloadResult.", gid_->toHex().c_str()));
  TransferStat st = calculateStat();
  auto res = std::make_shared<DownloadResult>();
  res->gid = gid_;
  res->attrs = downloadContext_->getAttributes();
  res->fileEntries = downloadContext_->getFileEntries();
  res->inMemoryDownload = inMemoryDownload_;
  res->sessionDownloadLength = st.sessionDownloadLength;
  res->sessionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      downloadContext_->calculateSessionTime());

  auto result = downloadResult();
  res->result = result.first;
  res->resultMessage = result.second;
  res->followedBy = followedByGIDs_;
  res->following = followingGID_;
  res->belongsTo = belongsToGID_;
  res->option = option_;
  res->metadataInfo = metadataInfo_;
  res->totalLength = getTotalLength();
  res->completedLength = getCompletedLength();
  res->uploadLength = st.allTimeUploadLength;
  if (pieceStorage_ && pieceStorage_->getBitfieldLength() > 0) {
    res->bitfield.assign(pieceStorage_->getBitfield(),
                         pieceStorage_->getBitfield() +
                             pieceStorage_->getBitfieldLength());
  }
#ifdef ENABLE_BITTORRENT
  if (downloadContext_->hasAttribute(CTX_ATTR_BT)) {
    const unsigned char* p = bittorrent::getInfoHash(downloadContext_);
    res->infoHash.assign(p, p + INFO_HASH_LENGTH);
  }
#endif // ENABLE_BITTORRENT
  res->pieceLength = downloadContext_->getPieceLength();
  res->numPieces = downloadContext_->getNumPieces();
  res->dir = option_->get(PREF_DIR);
  return res;
}

void RequestGroup::reportDownloadFinished()
{
  A2_LOG_NOTICE(fmt(MSG_FILE_DOWNLOAD_COMPLETED,
                    inMemoryDownload()
                        ? getFirstFilePath().c_str()
                        : downloadContext_->getBasePath().c_str()));
  uriSelector_->resetCounters();
#ifdef ENABLE_BITTORRENT
  if (downloadContext_->hasAttribute(CTX_ATTR_BT)) {
    TransferStat stat = calculateStat();
    int64_t completedLength = getCompletedLength();
    double shareRatio = completedLength == 0
                            ? 0.0
                            : 1.0 * stat.allTimeUploadLength / completedLength;
    auto attrs = bittorrent::getTorrentAttrs(downloadContext_);
    if (!attrs->metadata.empty()) {
      A2_LOG_NOTICE(fmt(MSG_SHARE_RATIO_REPORT, shareRatio,
                        util::abbrevSize(stat.allTimeUploadLength).c_str(),
                        util::abbrevSize(completedLength).c_str()));
    }
  }
#endif // ENABLE_BITTORRENT
}

void RequestGroup::setURISelector(std::unique_ptr<URISelector> uriSelector)
{
  uriSelector_ = std::move(uriSelector);
}

void RequestGroup::applyLastModifiedTimeToLocalFiles()
{
  if (!pieceStorage_ || !lastModifiedTime_.good()) {
    return;
  }
  A2_LOG_INFO(fmt("Applying Last-Modified time: %s",
                  lastModifiedTime_.toHTTPDate().c_str()));
  size_t n = pieceStorage_->getDiskAdaptor()->utime(Time(), lastModifiedTime_);
  A2_LOG_INFO(fmt("Last-Modified attrs of %lu files were updated.",
                  static_cast<unsigned long>(n)));
}

void RequestGroup::updateLastModifiedTime(const Time& time)
{
  if (time.good() && lastModifiedTime_ < time) {
    lastModifiedTime_ = time;
  }
}

void RequestGroup::increaseAndValidateFileNotFoundCount()
{
  ++fileNotFoundCount_;
  const int maxCount = option_->getAsInt(PREF_MAX_FILE_NOT_FOUND);
  if (maxCount > 0 && fileNotFoundCount_ >= maxCount &&
      downloadContext_->getNetStat().getSessionDownloadLength() == 0) {
    throw DOWNLOAD_FAILURE_EXCEPTION2(
        fmt("Reached max-file-not-found count=%d", maxCount),
        error_code::MAX_FILE_NOT_FOUND);
  }
}

void RequestGroup::markInMemoryDownload() { inMemoryDownload_ = true; }

void RequestGroup::setTimeout(std::chrono::seconds timeout)
{
  timeout_ = std::move(timeout);
}

bool RequestGroup::doesDownloadSpeedExceed()
{
  int spd = downloadContext_->getNetStat().calculateDownloadSpeed();
  return maxDownloadSpeedLimit_ > 0 && maxDownloadSpeedLimit_ < spd;
}

bool RequestGroup::doesUploadSpeedExceed()
{
  int spd = downloadContext_->getNetStat().calculateUploadSpeed();
  return maxUploadSpeedLimit_ > 0 && maxUploadSpeedLimit_ < spd;
}

void RequestGroup::saveControlFile() const
{
  if (saveControlFile_) {
    if (pieceStorage_) {
      pieceStorage_->flushWrDiskCacheEntry(false);
      pieceStorage_->getDiskAdaptor()->flushOSBuffers();
    }
    progressInfoFile_->save();
  }
}

void RequestGroup::removeControlFile() const
{
  progressInfoFile_->removeFile();
}

void RequestGroup::setDownloadContext(
    const std::shared_ptr<DownloadContext>& downloadContext)
{
  downloadContext_ = downloadContext;
  if (downloadContext_) {
    downloadContext_->setOwnerRequestGroup(this);
  }
}

bool RequestGroup::p2pInvolved() const
{
#ifdef ENABLE_BITTORRENT
  return downloadContext_->hasAttribute(CTX_ATTR_BT);
#else  // !ENABLE_BITTORRENT
  return false;
#endif // !ENABLE_BITTORRENT
}

void RequestGroup::enableSeedOnly()
{
  if (seedOnly_ || !option_->getAsBool(PREF_BT_DETACH_SEED_ONLY)) {
    return;
  }

  if (requestGroupMan_) {
    seedOnly_ = true;

    requestGroupMan_->decreaseNumActive();
    requestGroupMan_->requestQueueCheck();
  }
}

bool RequestGroup::isSeeder() const
{
#ifdef ENABLE_BITTORRENT
  return downloadContext_->hasAttribute(CTX_ATTR_BT) &&
         !bittorrent::getTorrentAttrs(downloadContext_)->metadata.empty() &&
         downloadFinished();
#else  // !ENABLE_BITTORRENT
  return false;
#endif // !ENABLE_BITTORRENT
}

void RequestGroup::setPendingOption(std::shared_ptr<Option> option)
{
  pendingOption_ = std::move(option);
}

} // namespace aria2
