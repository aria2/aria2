/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2015 Tatsuhiro Tsujikawa
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
#include "SftpNegotiationCommand.h"

#include <cassert>
#include <utility>

#include "Request.h"
#include "DownloadEngine.h"
#include "RequestGroup.h"
#include "PieceStorage.h"
#include "FileEntry.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Segment.h"
#include "DownloadContext.h"
#include "DefaultBtProgressInfoFile.h"
#include "RequestGroupMan.h"
#include "SocketCore.h"
#include "fmt.h"
#include "DiskAdaptor.h"
#include "SegmentMan.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "a2functional.h"
#include "URISelector.h"
#include "CheckIntegrityEntry.h"
#include "NullProgressInfoFile.h"
#include "ChecksumCheckIntegrityEntry.h"
#include "SftpDownloadCommand.h"

namespace aria2 {

SftpNegotiationCommand::SftpNegotiationCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e, const std::shared_ptr<SocketCore>& socket, Seq seq)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, socket),
      sequence_(seq),
      authConfig_(e->getAuthConfigFactory()->createAuthConfig(
          req, requestGroup->getOption().get()))

{
  path_ = getPath();
  setWriteCheckSocket(getSocket());

  const std::string& checksum = getOption()->get(PREF_SSH_HOST_KEY_MD);
  if (!checksum.empty()) {
    auto p = util::divide(std::begin(checksum), std::end(checksum), '=');
    hashType_.assign(p.first.first, p.first.second);
    util::lowercase(hashType_);
    digest_ = util::fromHex(p.second.first, p.second.second);
  }
}

SftpNegotiationCommand::~SftpNegotiationCommand() = default;

bool SftpNegotiationCommand::executeInternal()
{
  disableWriteCheckSocket();
  for (;;) {
    switch (sequence_) {
    case SEQ_HANDSHAKE:
      setReadCheckSocket(getSocket());
      if (!getSocket()->sshHandshake(hashType_, digest_)) {
        goto again;
      }
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - SSH handshake success", getCuid()));
      sequence_ = SEQ_AUTH_PASSWORD;
      break;
    case SEQ_AUTH_PASSWORD:
      if (!getSocket()->sshAuthPassword(authConfig_->getUser(),
                                        authConfig_->getPassword())) {
        goto again;
      }
      A2_LOG_DEBUG(
          fmt("CUID#%" PRId64 " - SSH authentication success", getCuid()));
      sequence_ = SEQ_SFTP_OPEN;
      break;
    case SEQ_SFTP_OPEN:
      if (!getSocket()->sshSFTPOpen(path_)) {
        goto again;
      }
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - SFTP file %s opened", getCuid(),
                       path_.c_str()));
      sequence_ = SEQ_SFTP_STAT;
      break;
    case SEQ_SFTP_STAT: {
      int64_t totalLength;
      time_t mtime;
      if (!getSocket()->sshSFTPStat(totalLength, mtime, path_)) {
        goto again;
      }
      Time t(mtime);
      A2_LOG_INFO(
          fmt("CUID#%" PRId64 " - SFTP File %s, size=%" PRId64 ", mtime=%s",
              getCuid(), path_.c_str(), totalLength, t.toHTTPDate().c_str()));
      if (!getPieceStorage()) {
        getRequestGroup()->updateLastModifiedTime(Time(mtime));
        onFileSizeDetermined(totalLength);
      }
      else {
        getRequestGroup()->validateTotalLength(getFileEntry()->getLength(),
                                               totalLength);
        sequence_ = SEQ_SFTP_SEEK;
      }
      break;
    }
    case SEQ_SFTP_SEEK: {
      sequence_ = SEQ_NEGOTIATION_COMPLETED;
      if (getSegments().empty()) {
        break;
      }

      auto& segment = getSegments().front();

      A2_LOG_INFO(fmt("CUID#%" PRId64 " - SFTP seek to %" PRId64, getCuid(),
                      segment->getPositionToWrite()));
      getSocket()->sshSFTPSeek(segment->getPositionToWrite());

      break;
    }
    case SEQ_FILE_PREPARATION:
      sequence_ = SEQ_SFTP_SEEK;
      disableReadCheckSocket();
      disableWriteCheckSocket();
      return false;
    case SEQ_NEGOTIATION_COMPLETED: {
      auto command = make_unique<SftpDownloadCommand>(
          getCuid(), getRequest(), getFileEntry(), getRequestGroup(),
          getDownloadEngine(), getSocket(), std::move(authConfig_));
      command->setStartupIdleTime(
          std::chrono::seconds(getOption()->getAsInt(PREF_STARTUP_IDLE_TIME)));
      command->setLowestDownloadSpeedLimit(
          getOption()->getAsInt(PREF_LOWEST_SPEED_LIMIT));
      command->setStatus(Command::STATUS_ONESHOT_REALTIME);

      getDownloadEngine()->setNoWait(true);

      if (getFileEntry()->isUniqueProtocol()) {
        getFileEntry()->removeURIWhoseHostnameIs(getRequest()->getHost());
      }
      getRequestGroup()->getURISelector()->tuneDownloadCommand(
          getFileEntry()->getRemainingUris(), command.get());
      getDownloadEngine()->addCommand(std::move(command));
      return true;
    }
    case SEQ_DOWNLOAD_ALREADY_COMPLETED:
    case SEQ_HEAD_OK:
    case SEQ_EXIT:
      return true;
    };
  }
again:
  addCommandSelf();
  if (getSocket()->wantWrite()) {
    setWriteCheckSocket(getSocket());
  }
  return false;
}

void SftpNegotiationCommand::onFileSizeDetermined(int64_t totalLength)
{
  getFileEntry()->setLength(totalLength);
  if (getFileEntry()->getPath().empty()) {
    auto suffixPath = util::createSafePath(
        util::percentDecode(std::begin(getRequest()->getFile()),
                            std::end(getRequest()->getFile())));

    getFileEntry()->setPath(
        util::applyDir(getOption()->get(PREF_DIR), suffixPath));
    getFileEntry()->setSuffixPath(suffixPath);
  }
  getRequestGroup()->preDownloadProcessing();

  if (totalLength == 0) {
    sequence_ = SEQ_NEGOTIATION_COMPLETED;

    if (getOption()->getAsBool(PREF_DRY_RUN)) {
      getRequestGroup()->initPieceStorage();
      onDryRunFileFound();
      return;
    }

    if (getDownloadContext()->knowsTotalLength() &&
        getRequestGroup()->downloadFinishedByFileLength()) {
      // TODO Known issue: if .aria2 file exists, it will not be
      // deleted on successful verification, because .aria2 file is
      // not loaded.  See also
      // HttpResponseCommand::handleOtherEncoding()
      getRequestGroup()->initPieceStorage();
      if (getDownloadContext()->isChecksumVerificationNeeded()) {
        A2_LOG_DEBUG("Zero length file exists. Verify checksum.");
        auto entry =
            make_unique<ChecksumCheckIntegrityEntry>(getRequestGroup());
        entry->initValidator();
        getPieceStorage()->getDiskAdaptor()->openExistingFile();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(
            std::move(entry));
        sequence_ = SEQ_EXIT;
      }
      else {
        getPieceStorage()->markAllPiecesDone();
        getDownloadContext()->setChecksumVerified(true);
        sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
        A2_LOG_NOTICE(fmt(MSG_DOWNLOAD_ALREADY_COMPLETED,
                          GroupId::toHex(getRequestGroup()->getGID()).c_str(),
                          getRequestGroup()->getFirstFilePath().c_str()));
      }
      poolConnection();
      return;
    }

    getRequestGroup()->adjustFilename(std::make_shared<NullProgressInfoFile>());
    getRequestGroup()->initPieceStorage();
    getPieceStorage()->getDiskAdaptor()->initAndOpenFile();

    if (getDownloadContext()->knowsTotalLength()) {
      A2_LOG_DEBUG("File length becomes zero and it means download completed.");
      // TODO Known issue: if .aria2 file exists, it will not be
      // deleted on successful verification, because .aria2 file is
      // not loaded.  See also
      // HttpResponseCommand::handleOtherEncoding()
      if (getDownloadContext()->isChecksumVerificationNeeded()) {
        A2_LOG_DEBUG("Verify checksum for zero-length file");
        auto entry =
            make_unique<ChecksumCheckIntegrityEntry>(getRequestGroup());
        entry->initValidator();
        getDownloadEngine()->getCheckIntegrityMan()->pushEntry(
            std::move(entry));
        sequence_ = SEQ_EXIT;
      }
      else {
        sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
        getPieceStorage()->markAllPiecesDone();
      }
      poolConnection();
      return;
    }
    // We have to make sure that command that has Request object must
    // have segment after PieceStorage is initialized. See
    // AbstractCommand::execute()
    getSegmentMan()->getSegmentWithIndex(getCuid(), 0);
    return;
  }
  else {
    auto progressInfoFile = std::make_shared<DefaultBtProgressInfoFile>(
        getDownloadContext(), nullptr, getOption().get());
    getRequestGroup()->adjustFilename(progressInfoFile);
    getRequestGroup()->initPieceStorage();

    if (getOption()->getAsBool(PREF_DRY_RUN)) {
      onDryRunFileFound();
      return;
    }

    auto checkIntegrityEntry = getRequestGroup()->createCheckIntegrityEntry();
    if (!checkIntegrityEntry) {
      sequence_ = SEQ_DOWNLOAD_ALREADY_COMPLETED;
      poolConnection();
      return;
    }
    // We have to make sure that command that has Request object must
    // have segment after PieceStorage is initialized. See
    // AbstractCommand::execute()
    getSegmentMan()->getSegmentWithIndex(getCuid(), 0);

    checkIntegrityEntry->pushNextCommand(std::unique_ptr<Command>(this));
    prepareForNextAction(std::move(checkIntegrityEntry));

    disableReadCheckSocket();

    sequence_ = SEQ_FILE_PREPARATION;
  }
}

void SftpNegotiationCommand::poolConnection() const
{
  if (getOption()->getAsBool(PREF_FTP_REUSE_CONNECTION)) {
    // TODO we don't need options.  Probably, we need to pool socket
    // using scheme, port and auth info as key
    getDownloadEngine()->poolSocket(getRequest(), authConfig_->getUser(),
                                    createProxyRequest(), getSocket(), "");
  }
}

void SftpNegotiationCommand::onDryRunFileFound()
{
  getPieceStorage()->markAllPiecesDone();
  getDownloadContext()->setChecksumVerified(true);
  poolConnection();
  sequence_ = SEQ_HEAD_OK;
}

std::string SftpNegotiationCommand::getPath() const
{
  auto& req = getRequest();
  auto path = req->getDir() + req->getFile();
  return util::percentDecode(std::begin(path), std::end(path));
}

} // namespace aria2
