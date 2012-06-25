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
#include "fmt.h"
#include "FileEntry.h"

namespace aria2 {

BtDependency::BtDependency
(RequestGroup* dependant,
 const SharedHandle<RequestGroup>& dependee)
  : dependant_(dependant),
    dependee_(dependee)
{}

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

namespace {
struct EntryCmp {
  bool operator()
  (const SharedHandle<FileEntry>& lhs,
   const SharedHandle<FileEntry>& rhs) const
  {
    return lhs->getOriginalName() < rhs->getOriginalName();
  }
};
} // namespace

bool BtDependency::resolve()
{
  if(!dependee_) {
    return true;
  }
  if(dependee_->getNumCommand() == 0 && dependee_->downloadFinished()) {
    SharedHandle<RequestGroup> dependee = dependee_;
    // cut reference here
    dependee_.reset();
    SharedHandle<DownloadContext> context(new DownloadContext());
    try {
      SharedHandle<DiskAdaptor> diskAdaptor =
        dependee->getPieceStorage()->getDiskAdaptor();
      diskAdaptor->openExistingFile();
      std::string content = util::toString(diskAdaptor);
      if(dependee->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)) {
        SharedHandle<TorrentAttribute> attrs =
          bittorrent::getTorrentAttrs(dependee->getDownloadContext());
        bittorrent::loadFromMemory
          (bittorrent::metadata2Torrent(content, attrs), context,
           dependant_->getOption(), "default");
        // We don't call bittorrent::adjustAnnounceUri() because it
        // has already been called with attrs.
      } else {
        bittorrent::loadFromMemory
          (content, context, dependant_->getOption(),
           File(dependee->getFirstFilePath()).getBasename());
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
        std::vector<SharedHandle<FileEntry> > destFiles;
        destFiles.reserve(fileEntries.size());
        for(std::vector<SharedHandle<FileEntry> >::const_iterator i =
              fileEntries.begin(), eoi = fileEntries.end(); i != eoi; ++i) {
          (*i)->setRequested(false);
          destFiles.push_back(*i);
        }
        std::sort(destFiles.begin(), destFiles.end(), EntryCmp());
        // Copy file path in dependant_'s FileEntries to newly created
        // context's FileEntries to endorse the path structure of
        // dependant_.  URIs and singleHostMultiConnection are also copied.
        for(std::vector<SharedHandle<FileEntry> >::const_iterator s =
              dependantFileEntries.begin(), eoi = dependantFileEntries.end();
            s != eoi; ++s){
          std::vector<SharedHandle<FileEntry> >::const_iterator d =
            std::lower_bound(destFiles.begin(), destFiles.end(), *s,
                             EntryCmp());
          if(d == destFiles.end() ||
             (*d)->getOriginalName() != (*s)->getOriginalName()) {
            throw DL_ABORT_EX
              (fmt("No entry %s in torrent file",
                   (*s)->getOriginalName().c_str()));
          } else {
            copyValues(*d, *s);
          }
        }
      }
    } catch(RecoverableException& e) {
      A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
      A2_LOG_INFO(fmt("BtDependency for GID#%" PRId64 " failed. Go without Bt.",
                      dependant_->getGID()));
      return true;
    }
    A2_LOG_INFO(fmt("Dependency resolved for GID#%" PRId64 "",
                    dependant_->getGID()));
    dependant_->setDownloadContext(context);
    return true;
  } else if(dependee_->getNumCommand() == 0) {
    // dependee_'s download failed.
    // cut reference here
    dependee_.reset();
    A2_LOG_INFO(fmt("BtDependency for GID#%" PRId64 " failed. Go without Bt.",
                    dependant_->getGID()));
    return true;
  } else {
    return false;
  }
}

} // namespace aria2
