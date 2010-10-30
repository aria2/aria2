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
#include "BtDependency.h"
#include "RequestGroup.h"
#include "Option.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DownloadContext.h"
#include "RecoverableException.h"
#include "message.h"
#include "prefs.h"
#include "util.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "File.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "StringFormat.h"

namespace aria2 {

BtDependency::BtDependency(const WeakHandle<RequestGroup>& dependant,
                           const SharedHandle<RequestGroup>& dependee):
  dependant_(dependant),
  dependee_(dependee),
  logger_(LogFactory::getInstance()) {}

BtDependency::~BtDependency() {}

namespace {
void copyValues(const SharedHandle<FileEntry>& d,
                const SharedHandle<FileEntry>& s)
{
  d->setRequested(true);
  d->setPath(s->getPath());
  d->addUris(s->getRemainingUris().begin(),
             s->getRemainingUris().end());
  d->setMaxConnectionPerServer(s->getMaxConnectionPerServer());
  d->setUniqueProtocol(s->isUniqueProtocol());
}
} // namespace

bool BtDependency::resolve()
{
  if(dependee_->getNumCommand() == 0 && dependee_->downloadFinished()) {
    SharedHandle<RequestGroup> dependee = dependee_;
    // cut reference here
    dependee_.reset();
    SharedHandle<DownloadContext> context(new DownloadContext());
    context->setDir(dependant_->getDownloadContext()->getDir());
    try {
      SharedHandle<DiskAdaptor> diskAdaptor =
        dependee->getPieceStorage()->getDiskAdaptor();
      diskAdaptor->openExistingFile();
      std::string content = util::toString(diskAdaptor);
      if(dependee->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) {
        SharedHandle<TorrentAttribute> attrs =
          bittorrent::getTorrentAttrs(dependee->getDownloadContext());
        bittorrent::loadFromMemory
          (bittorrent::metadata2Torrent(content, attrs), context, "default");
        // We don't call bittorrent::adjustAnnounceUri() because it
        // has already been called with attrs.
      } else {
        bittorrent::loadFromMemory
          (content, context, File(dependee->getFirstFilePath()).getBasename());
        bittorrent::adjustAnnounceUri(bittorrent::getTorrentAttrs(context),
                                      dependant_->getOption());
      }
      const std::vector<SharedHandle<FileEntry> >& fileEntries =
        context->getFileEntries();
      const std::vector<SharedHandle<FileEntry> >& dependantFileEntries =
        dependant_->getDownloadContext()->getFileEntries();
      // If dependant's FileEntry::getOriginalName() is empty, we
      // assume that torrent is single file. In Metalink3, this is
      // always assumed.
      if(fileEntries.size() == 1 && dependantFileEntries.size() == 1 &&
         dependantFileEntries[0]->getOriginalName().empty()) {
        copyValues(fileEntries[0], dependantFileEntries[0]);
      } else {
        std::for_each(fileEntries.begin(), fileEntries.end(),
                      std::bind2nd(mem_fun_sh(&FileEntry::setRequested),false));
        // Copy file path in dependant_'s FileEntries to newly created
        // context's FileEntries to endorse the path structure of
        // dependant_.  URIs and singleHostMultiConnection are also copied.
        std::vector<SharedHandle<FileEntry> >::const_iterator ctxFilesEnd =
          fileEntries.end();
        for(std::vector<SharedHandle<FileEntry> >::const_iterator s =
              dependantFileEntries.begin(), eoi = dependantFileEntries.end();
            s != eoi; ++s){
          std::vector<SharedHandle<FileEntry> >::const_iterator d =
            fileEntries.begin();
          for(; d != ctxFilesEnd; ++d) {
            if((*d)->getOriginalName() == (*s)->getOriginalName()) {
              break;
            }
          }
          if(d == ctxFilesEnd) {
            throw DL_ABORT_EX
              (StringFormat("No entry %s in torrent file",
                            (*s)->getOriginalName().c_str()).str());
          }
          copyValues(*d, *s);
        }
      }
    } catch(RecoverableException& e) {
      logger_->error(EX_EXCEPTION_CAUGHT, e);
      if(logger_->info()) {
        logger_->info("BtDependency for GID#%s failed. Go without Bt.",
                      util::itos(dependant_->getGID()).c_str());
      }
      return true;
    }
    if(logger_->info()) {
      logger_->info("Dependency resolved for GID#%s",
                    util::itos(dependant_->getGID()).c_str());
    }
    dependant_->setDownloadContext(context);
    return true;
  } else if(dependee_->getNumCommand() == 0) {
    // dependee_'s download failed.
    // cut reference here
    dependee_.reset();
    if(logger_->info()) {
      logger_->info("BtDependency for GID#%s failed. Go without Bt.",
                    util::itos(dependant_->getGID()).c_str());
    }
    return true;
  } else {
    return false;
  }
}

} // namespace aria2
